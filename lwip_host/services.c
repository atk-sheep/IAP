#include "services.h"

#include<sys/stat.h>
#include<stdio.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<assert.h>

char filename[] = "flash_app.bin";

FILE *file = NULL;

int openFile(){
    file = fopen(filename, "rb");
    if(file == NULL){
        perror("open error file");
        return -1;
    }
    return 0;
}

uint32_t fileSize(){
    struct stat st;
    if  (stat(filename,  &st)  ==  -1)  {
        perror("Error  getting  file  size");
        return  -1;
    }

    printf("File  size:  %ld  bytes\n",  st.st_size);
    if(st.st_size > UINT32_MAX){
        printf("file is too large\n");
        return -1;
    }
    return (uint32_t)st.st_size;
}

int diagnosticSession(uint8_t *buf){
    // char cm[] = "1002";

    uint8_t cm[] = {0x10, 0x02};

    // for(int i=0; i<l; ++i){
    //     *(buf++) = cm[i];
    // }

    memcpy(buf, cm, 2);

    return 2;
}

int requestDownload(uint8_t *buf, int *p){
    uint32_t bytes = fileSize();

    int packs = bytes/1024;

    if(bytes%1024) packs++;

    *p = packs;

    uint8_t b[4];

    b[0] = (uint8_t)(bytes & 0xFF);

    b[1] = (uint8_t)((bytes>>8) & 0xFF);

    b[2] = (uint8_t)((bytes>>16) & 0xFF);

    b[3] = (uint8_t)((bytes>>24) & 0xFF);

    uint8_t cm[] = {0x34, 0x01};

    memcpy(buf, cm, 2);

    memcpy(buf+2, b, 4);

    //open file

    if(openFile() < 0) return -1;

    return 6;
}

int tranfer(uint8_t *buf, int packnum){

    uint8_t cm[] = {0x36, 0x01};

    memcpy(buf, cm, 2);

    uint32_t t = (uint32_t)packnum;

    printf("NO.%d pack\n", packnum);

    buf[2] = (t) & 0xFF;
    buf[3] = (t>>8) & 0xFF;
    buf[4] = (t>>16) & 0xFF;
    buf[5] = (t>>24) & 0xFF;

    int byte_read = fread(buf+6, sizeof(uint8_t), 1024, file);

    assert(byte_read > 0);

    return byte_read+6;
}

int transferExit(uint8_t *buf){
    uint8_t cm[] = {0x37, 0x01};

    memcpy(buf, cm, 2);

    return 2;
}
