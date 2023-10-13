/* 
 * chardev2.c – создание символьного устройства ввода/вывода 
 */ 
 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/irq.h> 
#include <linux/kernel.h>  
#include <linux/module.h>  
#include <linux/poll.h> 
 
#include "chardev.h" 
#define SUCCESS 0 
#define DEVICE_NAME "char_dev" 
#define BUF_LEN 80 
 
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
 
/* Открыто ли сейчас устройство? Служит для предотвращения 
 * конкурентного доступа к одному устройству.
 */ 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
 
/* Сообщение, которое устройство будет выдавать при обращении. */ 
static char message[BUF_LEN]; 
 
static struct class *cls; 
 
/* Вызывается, когда процесс пытается открыть файл устройства. */ 
static int device_open(struct inode *inode, struct file *file) 
{ 
    pr_info("device_open(%p)\n", file); 
 
    try_module_get(THIS_MODULE); 
    return SUCCESS; 
} 
 
static int device_release(struct inode *inode, struct file *file) 
{ 
    pr_info("device_release(%p,%p)\n", inode, file); 
 
    module_put(THIS_MODULE); 
    return SUCCESS; 
} 
 
/* Эта функция вызывается, когда процесс, уже открывший файл, 
 * пытается считать из него. 
 */ 
static ssize_t device_read(struct file *file, /* см. include/linux/fs.h   */ 
                           char __user *buffer, /* Буфер для заполнения.  */ 
                           size_t length, /* Длина буфера.     */ 
                           loff_t *offset) 
{ 
    /* Количество байтов, фактически записываемых в буфер. */ 
    int bytes_read = 0; 
    /* Как далеко зашел процесс, считывающий  
     * сообщение? Пригождается, когда сообщение больше размера буфера
     * в device_read.
     */ 
    const char *message_ptr = message; 
 
    if (!*(message_ptr + *offset)) { /* Мы в конце сообщения. */ 
        *offset = 0; /* Сброс смещения. */ 
        return 0; /* Обозначение конца файла. */ 
    } 
 
    message_ptr += *offset; 
 
    /* Фактически помещает данные в буфер. */ 
    while (length && *message_ptr) { 
        /* Поскольку буфер находится в пользовательском сегменте данных, 
         * а не в сегменте ядра, присваивание не сработает. Вместо этого 
         * нужно использовать put_user, которая скопирует данные из 
         * сегмента ядра в сегмент пользователя. 
         */ 
        put_user(*(message_ptr++), buffer++); 
        length--; 
        bytes_read++; 
    } 
 
    pr_info("Read %d bytes, %ld left\n", bytes_read, length); 
 
    *offset += bytes_read; 
 
    /* Функции чтения должны возвращать количество байтов, реально 
     * вставляемых в буфер. 
     */ 
    return bytes_read; 
} 
 
/* Вызывается, когда кто-то пытается произвести запись в файл устройства. */ 
static ssize_t device_write(struct file *file, const char __user *buffer, 
                            size_t length, loff_t *offset) 
{ 
    int i; 
 
    pr_info("device_write(%p,%p,%ld)", file, buffer, length); 
 
    for (i = 0; i < length && i < BUF_LEN; i++) 
        get_user(message[i], buffer + i); 
 
    /* Также возвращает количество использованных во вводе символов. */ 
    return i; 
} 
 
/* Эта функция вызывается, когда процесс пытается выполнить ioctl для 
 * файла устройства. Мы получаем два дополнительных параметра 
 * (дополнительных для структур inode и file, которые получают все  
 * функции устройств): номер ioctl и параметр, заданный для этой ioctl. 
 * 
 * Если ioctl реализует запись или запись/чтение (то есть ее вывод 
 * возвращается вызывающему процессу), вызов ioctl возвращает вывод 
 * этой функции.
 */ 
static long 
device_ioctl(struct file *file, /* То же самое. */ 
             unsigned int ioctl_num, /* Число и параметр для ioctl */ 
             unsigned long ioctl_param) 
{ 
    int i; 
    long ret = SUCCESS; 
 
    /* Мы не хотим взаимодействовать с двумя процессами одновременно */ 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    /* Переключение согласно вызванной ioctl. */ 
    switch (ioctl_num) { 
    case IOCTL_SET_MSG: { 
        /* Получение указателя на сообщение (в пользовательском  
         * пространстве) и установка его как сообщения устройства. 
         * Получение параметра, передаваемого ioctl процессом. 
         */ 
        char __user *tmp = (char __user *)ioctl_param; 
        char ch; 
 
        /* Определение длины сообщения. */ 
        get_user(ch, tmp); 
        for (i = 0; ch && i < BUF_LEN; i++, tmp++) 
            get_user(ch, tmp); 
 
        device_write(file, (char __user *)ioctl_param, i, NULL); 
        break; 
    } 
    case IOCTL_GET_MSG: { 
        loff_t offset = 0; 
 
        /* Передача текущего сообщения вызывающему процессу. Получаемый 
         * параметр является указателем, который мы заполняем. 
         */ 
        i = device_read(file, (char __user *)ioctl_param, 99, &offset); 
 
        /* Помещаем в конец буфера нуль, чтобы он правильно завершился. 
         */ 
        put_user('\0', (char __user *)ioctl_param + i); 
        break; 
    } 
    case IOCTL_GET_NTH_BYTE: 
        /* Эта ioctl является и вводом (ioctl_param), и выводом 
         * (возвращаемым значением этой функции). 
         */ 
        ret = (long)message[ioctl_param]; 
        break; 
    } 
 
    /* Теперь можно принимать следующий вызов. */ 
    atomic_set(&already_open, CDEV_NOT_USED); 
 
    return ret; 
} 
 
/* Объявления модулей. */ 
 
/* Эта структура будет хранить функции, вызываемые при выполнении  
 * процессом действий с созданным нами устройством. Поскольку указатель  
 * на эту структуру содержится в таблице устройств, он не может быть 
 * локальным для init_module. NULL для не реализованных функций. 
 */ 
static struct file_operations fops = { 
    .read = device_read, 
    .write = device_write, 
    .unlocked_ioctl = device_ioctl, 
    .open = device_open, 
    .release = device_release, /* Аналогично закрытию. */ 
}; 
 
/* Инициализация модуля – регистрация символьного устройства. */ 
static int __init chardev2_init(void) 
{ 
    /* Регистрация символьного устройства (по меньшей мере попытка). */ 
    int ret_val = register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops); 
 
    /* Отрицательные значения означают ошибку. */ 
    if (ret_val < 0) { 
        pr_alert("%s failed with %d\n", 
                 "Sorry, registering the character device ", ret_val); 
        return ret_val; 
    } 
 
    cls = class_create(THIS_MODULE, DEVICE_FILE_NAME); 
    device_create(cls, NULL, MKDEV(MAJOR_NUM, 0), NULL, DEVICE_FILE_NAME); 
 
    pr_info("Device created on /dev/%s\n", DEVICE_FILE_NAME); 
 
    return 0; 
} 
 
/* Очистка – снятие регистрации соответствующего файла из /proc. */ 
static void __exit chardev2_exit(void) 
{ 
    device_destroy(cls, MKDEV(MAJOR_NUM, 0)); 
    class_destroy(cls); 
 
    /* Снятие регистрации устройства. */ 
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME); 
} 
 
module_init(chardev2_init); 
module_exit(chardev2_exit); 
 
MODULE_LICENSE("GPL");

