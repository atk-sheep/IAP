#ifndef _SERVICES_H_
#define _SERVICES_H_

#include<stdint.h>
#include"iap.h"
#include"main.h"

#define CMD10 0x10
#define CMD34 0x34
#define CMD36 0x36
#define CMD37 0x37

typedef enum errorCode{
    NOERR,
    ERR10,
    ERR34,
    ERR36,
    ERR37,
    WRONGSERV
} errorCode;

errorCode servicesAnalysis(frame *message, uint8_t *txbuf, int *plen);

errorCode diagControl(frame *message, uint8_t *txbuf, int *plen);

errorCode requestDownload(frame *message, uint8_t *txbuf, int *plen);

errorCode transfer(frame *message, uint8_t *txbuf, int *plen);

errorCode transferExit(frame *message, uint8_t *txbuf, int *plen);

void analysisErr(errorCode code, uint8_t *buf, int len);

#endif