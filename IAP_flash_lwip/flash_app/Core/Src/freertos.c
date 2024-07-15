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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
const u8 TEXT_Buffer[]={"New SP Embeded QSPI TEST"};
#define SIZE sizeof(TEXT_Buffer)
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
  u32 flash_size=32*1024*1024;	//FLASH 大�?为32M字节
  u8 datatemp[SIZE] = {0};
  printf("Start write data to W25Q256.... \r\n");
  W25Q256_Write((u8*)TEXT_Buffer,flash_size-100,SIZE);
  printf("W25Q256 Write Finished!\r\n");	//�??示传�?完�?
  printf("Start Read W25Q256.... \r\n");
  W25Q256_Read(datatemp, flash_size-100, SIZE);					//从倒数第100个地�?�处开始,读出SIZE个字节
  printf("Get data from w25Q256: %s\r\n", datatemp);
}

// void StartTask03(void const * argument);
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
//  osThreadDef(dmaReceiveTask, StartTask03, osPriorityIdle, 0, 2048);
//  dmaReceiveTaskHandle = osThreadCreate(osThread(dmaReceiveTask), NULL);
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

  printf("flashapp defult task started!!!\r\n");

  for(;;)
  {
	  HAL_GPIO_WritePin(GPIOB, LED1_Pin, GPIO_PIN_SET);
	  osDelay(500);
	  HAL_GPIO_WritePin(GPIOB, LED1_Pin, GPIO_PIN_RESET);
	  osDelay(500);

	  printf("hello flashApp\r\n");

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
  for(;;)
  {
    
  }
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
// void StartTask03(void const * argument)
// {
//   while(1){
//     //do nothing
//   }
  
// }

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    
}
/* USER CODE END Application */
