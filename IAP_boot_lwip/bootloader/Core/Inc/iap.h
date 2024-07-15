/*
 * iap.h
 *
 *  Created on: Apr 21, 2024
 *      Author: ECA1WX
 */

#ifndef INC_IAP_H_
#define INC_IAP_H_

#include "main.h"

typedef  void (*iapfun)(void);  

#define FLASH_APP_ADDR 0x08040000

typedef struct flashItem
{
    u8 *buf;
    u32 len;
} flashItem;

#define ITEM_SIZE sizeof(flashItem)

typedef struct frame
{
  uint8_t cmd;
  uint8_t subfunc;
  uint8_t *data;
  uint32_t len;
} frame;

void iap_load_app(u32 appxaddr);

void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	

uint32_t flash_write(u32 addr, u8 *buf, u32 sz);

#endif /* INC_IAP_H_ */
