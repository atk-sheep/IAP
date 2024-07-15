#include "services.h"

#include"FreeRTOS.h"
#include "cmsis_os.h"
#include "iap.h"
#include <string.h>

extern QueueHandle_t  flashQueue;

extern SemaphoreHandle_t flash_mutex;

extern uint8_t APPValid;

extern uint8_t jumpflag;

uint32_t filesize;

uint32_t bytessum;

errorCode servicesAnalysis(frame *message, uint8_t *txbuf, int *plen){
    uint8_t service = message->cmd;
    
    errorCode ret = NOERR;  //must must be initailized, carefully!!!

    switch (service)
    {
    case CMD10:
        ret = diagControl(message, txbuf, plen);
        break;
    
    case CMD34:
        ret = requestDownload(message, txbuf, plen);
        break;

    case CMD36:
        ret = transfer(message, txbuf, plen);
        break;

    case CMD37:
        ret = transferExit(message, txbuf, plen);
        break;

    default:
        ret = WRONGSERV;
        break;
    }

    return ret;
}

errorCode diagControl(frame *message, uint8_t *txbuf, int *len){
    uint8_t subfunc = message->subfunc;

    errorCode ret = NOERR;

    switch (subfunc)
    {
    case 0x02:
        /* code */
        printf("enter into program session\r\n");
        txbuf[0] = 0x50;
        txbuf[1] = 0x02;
        *len = 2;
        break;
    
    default:
        ret = ERR10;
        break;
    }

    return ret;
}

errorCode requestDownload(frame *message, uint8_t *txbuf, int *len){
    uint8_t subfunc = message->subfunc;

    errorCode ret = NOERR;

    switch (subfunc)
    {
    case 0x01:
        /* code */
        uint8_t *data = message->data;
        //printf("file size byte: %02X %02X %02X %02X \r\n", data[0], data[1], data[2], data[3]);

        filesize = (uint32_t)( ( (uint32_t)(data[0]) & 0x000000FF) | ( (uint32_t)(data[1]<<8) & 0x0000FF00)
                                        | ( (uint32_t)(data[2]<<16) & 0x00FF0000) | ( (uint32_t)(data[3]<<24) & 0xFF000000) );
        printf("34 request download file size: %d\r\n", filesize);

        txbuf[0] = 0x74;
        txbuf[1] = 0x01;
        *len = 2;
        break;
    
    default:
        ret = ERR34;
        break;
    }

    return ret;
}

errorCode transfer(frame *message, uint8_t *txbuf, int *len){
    uint8_t subfunc = message->subfunc;

    errorCode ret = NOERR;

    flashItem item_send;

    switch (subfunc)
    {
    case 0x01:
        //build flashitem
        uint8_t *data = message->data;
        uint32_t packnum = (uint32_t)( ((uint32_t)(data[0]) & 0x000000FF) | ((uint32_t)(data[1]<<8) & 0x0000FF00)
                                        | ((uint32_t)(data[2]<<16) & 0x00FF0000) | ((uint32_t)(data[3]<<24) & 0xFF000000) );

        printf("receive NO.%d pack\r\n", packnum);
        
        int datalen = message->len - 4;

        item_send.buf = data+4;
        item_send.len = datalen;

        bytessum += datalen;

        if(xSemaphoreTake(flash_mutex,  portMAX_DELAY)  ==  pdTRUE)  {
            //  访问共享资源
            
            if  (xQueueSend(flashQueue,  &item_send,  portMAX_DELAY)  !=  pdPASS)  {
              //  发送失败处理
              printf("Failed  to  send  item  to  the  queue\r\n");
                ret = ERR36;
            }  else  {
                //  发送成功

            }

            //  延时一段时间
            //vTaskDelay(pdMS_TO_TICKS(1000));

            //  释放互斥量
            xSemaphoreGive(flash_mutex);
        }

        txbuf[0] = 0x76;
        txbuf[1] = 0x01;
        *len = 2;
        break;
    
    default:
        ret = ERR36;
        break;
    }

    return ret;
}

errorCode transferExit(frame *message, uint8_t *txbuf, int *len){
    uint8_t subfunc = message->subfunc;

    errorCode ret = NOERR;

    switch (subfunc)
    {
    case 0x01:
        printf("transfer over\r\n");
        //简化的校验操作，可单独写服务校验
        if(filesize == bytessum){
            printf("APP is valid\r\n");
            APPValid = 1;
            txbuf[0] = 0x77;
            txbuf[1] = 0x01;
            *len = 2;
        }else{
            printf("APP is not valid\r\n");
            APPValid = 0;
            txbuf[0] = 0x7F;
            txbuf[1] = 0x37;
            *len = 2;
        }
        
        break;
    
    default:
        ret = ERR37;
        break;
    }

    return ret;
}

void analysisErr(errorCode code, uint8_t *buf, int len){
    if(code == ERR10){
        char str[] = "10 failed";
        memcpy(buf, str, strlen(str)+1);
        return;
    }
    else if(code == ERR34){
        char str[] = "34 failed";
        memcpy(buf, str, strlen(str)+1);
        return;
    }
    else if(code == ERR36){
        char str[] = "36 failed";
        memcpy(buf, str, strlen(str)+1);
        return;
    }
    else if(code == ERR37){
        char str[] = "37 failed";
        memcpy(buf, str, strlen(str)+1);
        return;
    }
    else{
        char str[] = "errcode not exited!!!";
        memcpy(buf, str, strlen(str)+1);
        return;
    }
}