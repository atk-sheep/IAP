/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "pcf8574.h"
#include <stdio.h>
#include "w25q256.h"
#include <stdbool.h>
#include "usart.h"
#include "iap.h"
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum{
  CMDERR = 0,
  NOCMD = 1,
  CMD_34 = 2,
  CMD_36 = 3,
} cmd_t;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const u8 TEXT_Buffer[]={"New SP Embeded QSPI TEST"};
#define SIZE sizeof(TEXT_Buffer)

#define USART_REC_LEN 32*1024    //预留128k接收APP
u8 USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20040000)));
uint8_t lenbuf[9] = {0};
uint8_t cmdbuf[14] = {0};
cmd_t cmd = NOCMD;
bool cmdflag = false;
static bool flag_36 = false, done = false;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId dmaReceiveTaskHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId feedDogTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void test_w25Q256(void){
  u32 flash_size=32*1024*1024;	//FLASH 大小为32M字节
  u8 datatemp[SIZE] = {0};
  printf("Start write data to W25Q256.... \r\n");
  W25Q256_Write((u8*)TEXT_Buffer,flash_size-100,SIZE);
  printf("W25Q256 Write Finished!\r\n");
  printf("Start Read W25Q256.... \r\n");
  W25Q256_Read(datatemp, flash_size-100, SIZE);					//从倒数第100个地�?�处开始,读出SIZE个字节
  printf("Get data from w25Q256: %s\r\n", datatemp);
}

void StartTask03(void const * argument);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void StartTask02(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 4096);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of feedDogTask */
  osThreadDef(feedDogTask, StartTask02, osPriorityIdle, 0, 2048);
  feedDogTaskHandle = osThreadCreate(osThread(feedDogTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
    osThreadDef(dmaReceiveTask, StartTask03, osPriorityIdle, 0, 2048);
    dmaReceiveTaskHandle = osThreadCreate(osThread(dmaReceiveTask), NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

  printf("bootloader defult task started!!!\r\n");

  for(;;)
  {
	  HAL_GPIO_WritePin(GPIOB, LED0_Pin, GPIO_PIN_SET);
	  osDelay(500);
	  HAL_GPIO_WritePin(GPIOB, LED0_Pin, GPIO_PIN_RESET);
	  osDelay(500);

	  printf("hello bootloader\r\n");

  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
* @brief Function implementing the feedDogTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartTask02 */
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  osDelay(2000);

  for(;;)
  {
    if(done)
      iap_load_app(FLASH_APP_ADDR);
  }
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void print_func(uint8_t* pstr, int buflen){
  pstr[buflen] = '\0';
  printf("reveive string: %s\r\n", pstr);
  return;
}

bool hextodec(uint8_t hex_string[], int *decimal){
    char *ptr;

    // printf("hex string: %s", hex_string);

    // 将16进制字符串转�?�为长整型
    (*decimal) = strtol(hex_string, &ptr, 16);

    // 检查转换是否成功
    if (ptr == hex_string) {
        printf("Transform failed!!!\r\n");
        return false;
    } else {
        printf("Transform successfully: %d\r\n", *decimal);
        return true;
    }
}

void alycmd(uint8_t *ptr){
  if(ptr[0]==0 && ptr[1]==0){
    cmd = NOCMD;
  }
  else if(ptr[0]=='3' && ptr[1]=='4'){
    cmd = CMD_34;
  }
  else if(ptr[0]=='3' && ptr[1]=='6'){
    cmd = CMD_36;
  }
  else{
    cmd = CMDERR;
  }
}

void StartTask03(void const * argument)
{
  cmdbuf[11] = '\0';
  int applength = 0;
  /**
   * @brief DMA接收
   * 
   */
  HAL_UART_Receive_DMA(&huart1, cmdbuf, sizeof(cmdbuf)-1);

  osDelay(50);

  printf("reveive dma state: %d\r\n", hdma_usart1_rx.State);

  while(1){
    if(hdma_usart1_rx.State == HAL_DMA_STATE_READY)
		{
      // cmd
      if(cmdflag){
        print_func(cmdbuf, sizeof(cmdbuf));
        cmdflag = false;
        alycmd(cmdbuf);

        switch (cmd)
        {
        case CMD_34:
          memcpy(lenbuf, cmdbuf+3, sizeof(lenbuf)-1);
          lenbuf[8] = '\0';
          if(hextodec(lenbuf, &applength)){
            memset(cmdbuf, 0, sizeof(cmdbuf));
            //开启36 cmd接收
            HAL_UART_Receive_DMA(&huart1, cmdbuf, 4);
          }
          else{
            memset(cmdbuf, 0, sizeof(cmdbuf));
          }
          break;
        case CMD_36:
          printf("Prepare receiving APP!!!\r\n");
          memset(cmdbuf, 0, sizeof(cmdbuf));
          flag_36 = true;
          printf("applength: %d", applength);
          //开启APP文件接收
          HAL_UART_Receive_DMA(&huart1, USART_RX_BUF, applength);
          break;
        
        default:
          break;
        }
      }
      //not cmd
      else{
        if(flag_36 & (!done)){
          printf("Writing into flash!!!\r\n");
          iap_write_appbin(FLASH_APP_ADDR,USART_RX_BUF,applength);//更新FLASH代码
          printf("Writing done!!!\r\n");
          done = true;
          osDelay(50);
        }
        
      }
    }
  }

  /**
   * @brief 中断接收
   * 
   */
  // HAL_UART_Receive_IT(&huart1, rxbuf, sizeof(rxbuf));

  // osDelay(50);

  // printf("reveive dma state: %x\r\n", huart1.RxState);

  // while(1){
  //   if(huart1.RxState == HAL_UART_STATE_READY)
	// 	{
  //     printf("reveive string: %s\r\n", rxbuf);
  //     while(huart1.RxState != HAL_UART_STATE_BUSY_RX){
  //       HAL_UART_Receive_IT(&huart1, rxbuf, sizeof(rxbuf));
  //     }
  //     // not work
  //     //while(HAL_UART_Receive_IT(&huart1, rxbuf, sizeof(rxbuf)) != HAL_OK) ;
  //   }
    
  // }
  
}

/**
 * @brief DMA receive cpl interrupt callback
 * 
 * @param huart 
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    //cmd可处理
    if(cmdbuf[0]!=0 && cmdbuf[1]!=0){
      cmdflag = true;
    }
    else{
      cmdflag = false;
    }
    return;
}
/* USER CODE END Application */
