/*  userspace_ioctl.c – процесс, позволяющий контролировать модуль ядра  
 *  с помощью ioctl.
 * 	
 *  До этого момента можно было использовать для ввода и вывода cat.
 *  Теперь необходимо использовать ioctl, для чего нужно написать свой  
 *  процесс.
 */ 
 
/* Детали устройства, такие как номера ioctl и старший файл устройства.  */ 
#include "../Modul/chardev.h" 
 
#include <stdio.h> /* Стандартный ввод-вывод. */ 
#include <fcntl.h> /* Открытие. */ 
#include <unistd.h> /* Закрытие. */ 
#include <stdlib.h> /* Выход. */ 
#include <sys/ioctl.h> /* ioctl */ 
 
/* Функции для вызовов ioctl. */ 
 
int ioctl_set_msg(int file_desc, char *message) 
{ 
    int ret_val; 
 
    ret_val = ioctl(file_desc, IOCTL_SET_MSG, message); 
 
    if (ret_val < 0) { 
        printf("ioctl_set_msg failed:%d\n", ret_val); 
    } 
 
    return ret_val; 
} 
 
int ioctl_get_msg(int file_desc) 
{ 
    int ret_val; 
    char message[100] = { 0 }; 
 
  /* Внимание! Это опасно, так как мы не сообщаем ядру, до куда  
   * можно производить запись, то есть рискуем вызвать переполнение   
   * буфера. В реальной программе мы бы использовали две ioctl - одну   
   * для информирования ядра о длине буфера и вторую для предоставления
   * ему самого буфера под заполнение.  
   */ 
    ret_val = ioctl(file_desc, IOCTL_GET_MSG, message); 
 
    if (ret_val < 0) { 
        printf("ioctl_get_msg failed:%d\n", ret_val); 
    } 
    printf("get_msg message:%s", message); 
 
    return ret_val; 
} 
 
int ioctl_get_nth_byte(int file_desc) 
{ 
    int i, c; 
 
    printf("get_nth_byte message:"); 
 
    i = 0; 
    do { 
        c = ioctl(file_desc, IOCTL_GET_NTH_BYTE, i++); 
 
        if (c < 0) { 
            printf("\nioctl_get_nth_byte failed at the %d'th byte:\n", i); 
            return c; 
        } 
 
        putchar(c); 
    } while (c != 0); 
 
    return 0; 
} 
 
/* Main – вызов функций ioctl. */ 
int main(void) 
{ 
    int file_desc, ret_val; 
    char *msg = "Message passed by ioctl\n"; 
 
    file_desc = open(DEVICE_PATH, O_RDWR); 
    if (file_desc < 0) { 
        printf("Can't open device file: %s, error:%d\n", DEVICE_PATH, 
               file_desc); 
        exit(EXIT_FAILURE); 
    } 
 
    ret_val = ioctl_set_msg(file_desc, msg); 
    if (ret_val) 
        goto error; 
    ret_val = ioctl_get_nth_byte(file_desc); 
    if (ret_val) 
        goto error; 
    ret_val = ioctl_get_msg(file_desc); 
    if (ret_val) 
        goto error; 
 
    close(file_desc); 
    return 0; 
error: 
    close(file_desc); 
    exit(EXIT_FAILURE); 
}

