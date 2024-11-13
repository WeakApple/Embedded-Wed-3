#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>


int main(void){
    int dev,len;
    char send_buf[10]="hello drv";
    char recv_buf[10];
    dev = open("/dev/simple_buffer",O_RDWR);
    if(dev<0){
        printf("driver open failed!\n");
        return -1;
    }
    write(dev,send_buf,strlen(send_buf));
    len = read(dev,recv_buf,sizeof(recv_buf));
    if(len>0)
        printf("recv data -> %s\n",recv_buf);
    close(dev);
    return 0;
}