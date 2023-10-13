/*  userspace_ioctl.c – процесс, позволяющий контролировать модуль ядра  
 *  с помощью ioctl.
 * 	
 *  До этого момента можно было использовать для ввода и вывода cat.
 *  Теперь необходимо использовать ioctl, для чего нужно написать свой  
 *  процесс.
 */ 
 
/* Детали устройства, такие как номера ioctl и старший файл устройства.  */ 
#include "../Modul/SysFile.h" 
 
#include <stdio.h> /* Стандартный ввод-вывод. */ 
#include <fcntl.h> /* Открытие. */ 
#include <unistd.h> /* Закрытие. */ 
#include <stdlib.h> /* Выход. */ 
//#include <sys/ioctl.h> /* ioctl */ 
 
#include <string.h>
 
 
/* Main – вызов функций proc. */ 
int main(void) 
{ 
    int file_desc, ret_val; 
    char *msg = "64";
    char buf[PROCFS_MAX_SIZE];
//fp=fopen("1.txt", "r+")
    file_desc = open(FILE_PATH, O_RDWR); 
    if (file_desc < 0) { 
        printf("Can't open device file: %s, error:%d\n", FILE_PATH, 
               file_desc); 
        exit(EXIT_FAILURE); 
    } 
 
 
    ret_val = write(file_desc, msg, strlen(msg) + 1);
    if(ret_val < 0){goto error;} 
    close(file_desc); 
    
    sleep (2);
    
    file_desc = open(FILE_PATH, O_RDWR); 
    ret_val = read(file_desc, buf, PROCFS_MAX_SIZE);
    if(ret_val < 0){goto error;} 
    printf("read value: %s\n", buf);
//    for(unsigned int i = 0; i < 32; ++i)
//    {
//       printf("%d - %c = %d\n", i, buf[i], buf[i]);
//    }
 
 
    close(file_desc); 
    return 0; 
error: 
    close(file_desc); 
    exit(EXIT_FAILURE); 
}

