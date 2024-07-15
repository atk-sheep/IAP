/**
 * @file tcp_flash.c
 * @author cyy (xxx@dxxx.com)
 * @brief this file is used for flash stm32F7 APP
 * @version 0.1
 * @date 2024-07-09
 * 
 * @copyright Copyright (c) 2024
 * 
 */

/*
rule:
    services：10 34 36 37
    10 02 : enter into flash mode
    34 <flash address> <bin length>
    36 <package label> <package length:1024>
    37 01 : quit flash
    //10 01 : jump to APP
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#include <pthread.h>

#include "flash.h"
#include "services.h"

#define PORT 6789

typedef struct sockaddr sockaddr;

typedef struct sockaddr_in sockaddr_in;

#define LWIP_SEND_DATA 0x80
uint8_t udp_flag = 0x80;

void print_addr(sockaddr_in* addr){

    char ipAddress[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &(addr->sin_addr), ipAddress, INET_ADDRSTRLEN);

    printf("remote ip: %s\n", ipAddress);

    uint16_t port = ntohs(addr->sin_port);

    printf("remote port: %d\n", port);

    return;
}

char input[30] = {};
pthread_mutex_t mutex;

bool flag_start = false;
bool flag_exit = false;
bool flag_close = false;

#define CAPACITY 1024;

uint8_t *txbuffer;

void *getInput(void *arg){
    
    while(1){
        
        pthread_mutex_lock(&mutex);

        printf("enter a string: \n");

        fgets(input, sizeof(input), stdin);

        input[1] = 0;

        printf("input: %s\n", input);

        if(strcmp(input, "q") == 0){
            printf("will exit\n");
            flag_exit = true;
        }
        else if(strcmp(input, "s") == 0){
            printf("will start\n");
            flag_start = true;
        }
        else{
            printf("not valid\n");
        }

        pthread_mutex_unlock(&mutex);

        sleep(1);

        if(flag_exit) break;
    }
}

int buildCnn(){
    int localsock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.1.10", &server_addr.sin_addr.s_addr);
    server_addr.sin_port = htons(12345);

    int ret = -1;
    if((ret = connect(localsock, (sockaddr *)&server_addr, sizeof(server_addr))) < 0){
        perror("connect failed!");
        close(localsock);
        return -1;
    }
    else{
        return localsock;
    }
}

//send and recv results analyse
int sendData(int sockid, uint8_t *buf, int len){
    //printf("sockid: %d\n", sockid);
    if(send(sockid, buf, len, 0) <= 0){   //最后flags 置0，同write
        printf("send failed!!!\n");
        return -1;
    }

    printf("send successful\n");

    int  recv_len  =  recv(sockid,  buf,  50,  0);

    if(recv_len > 0){
        buf[recv_len] = '\0';

        printf("recieve reply from server: %s\n", buf);
    }
    else{
        return -1;  //链接关闭
    }
}

int main(){
    pthread_mutex_init(&mutex, NULL);

    pthread_t tid = -1;

    int ret = pthread_create(&tid, NULL, getInput, NULL);

    if(ret != 0){
        fprintf(stderr, "pthread_create error: %s\n", strerror(ret));
    }
    else{
        fprintf(stdout, "pthread_create success\n");
    }

    //build tcp connection with remote, connect to server
    int sockid = buildCnn();
    if(sockid < 0){
        printf("build connction failed\n");
        return 0;
    }

    flash_state f_state = FLASH_START;

    txbuffer = (uint8_t*)malloc(sizeof(uint8_t)*1048);

    int txbufLen = 0;

    int packnum = 0, packsum = 0;

    while(1){
        if(flag_start){
            switch (f_state)
            {
            case FLASH_START:
                /* 10 02 */
                printf("FLASH_START\n");
                if((txbufLen = diagnosticSession(txbuffer)) < 0){
                    printf("10 02 failed\n");
                    f_state = FLASH_ERROR;
                }
                else{
                    f_state = FLASH_REQ;
                }
                break;

            case FLASH_REQ:
                printf("FLASH_REQ\n");
                if((txbufLen = requestDownload(txbuffer, &packsum)) <0){
                    printf("request error\n");
                    f_state = FLASH_ERROR;
                }
                else{
                    printf("packsum: %d\n", packsum);
                    printf("input n to start flash\n");
                    char t[5];

                    while(1){
                        pthread_mutex_lock(&mutex);
                        printf("enter n to start transfer APP\n");
                        fgets(t, sizeof(t), stdin);
                        pthread_mutex_unlock(&mutex);

                        if(t[0] == 'n') break;
                    }

                    f_state = FLASH_TRANS;
                    /*initial flash paraments*/
                    packnum = 0;
                }
                break;

            case FLASH_TRANS:
                printf("FLASH_TRANS\n");
                if((txbufLen = tranfer(txbuffer, packnum)) <0){
                    printf("transfer error\n");
                    f_state = FLASH_ERROR;
                }
                else if(packnum >= (packsum-1) ){
                    printf("FLASH_TRANS over\n");
                    f_state = FLASH_QUIT;
                }
                else{
                    packnum++;
                }
                break;

            case FLASH_QUIT:
                printf("FLASH_QUIT\n");
                if((txbufLen = transferExit(txbuffer)) <0){
                    printf("request error\n");
                    f_state = FLASH_ERROR;
                }
                else{
                    f_state = FLASH_START;
                    flag_start = false;
                    flag_close = true;
                }
                break;
            
            default:
                break;
            }
            if(f_state == FLASH_ERROR) break;
            
            //send and recv
            if(sendData(sockid, txbuffer, txbufLen) < 0) break; 

        }

        if(flag_close){
            // sleep(2);
            // printf("close....\n");
            // close(sockid);
            // flag_close = false;
            int  recv_len  =  recv(sockid,  txbuffer,  50,  0);
            if(recv_len == 0){
                printf("remote close\n");
                close(sockid);
                flag_close = false;
            }
        }

        if(flag_exit) break;
    }
    
    return 0;
}