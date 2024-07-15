#ifndef _SEVICES_H_
#define _SEVICES_H_

#include<stdint.h>

int diagnosticSession(uint8_t *);

int requestDownload(uint8_t *, int *);

int tranfer(uint8_t *, int);

int transferExit(uint8_t *);

#endif