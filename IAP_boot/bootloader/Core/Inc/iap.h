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

#define FLASH_APP_ADDR		0x08040000  				

void iap_load_app(u32 appxaddr);

void iap_write_appbin(u32 appxaddr,u8 *appbuf,u32 applen);	

#endif /* INC_IAP_H_ */
