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
#include "services.h"
#include <string.h>

#include "lwip/sockets.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId servicesTaskHandle;

uint8_t flashflag = 0;
SemaphoreHandle_t flash_mutex;

uint8_t APPValid = 0;

uint8_t jumpflag = 0;

QueueHandle_t  flashQueue;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId feedDogTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
int tcp_server();

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
  flash_mutex = xSemaphoreCreateMutex();

  flashQueue = xQueueCreate(3,  ITEM_SIZE);
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
    osThreadDef(servicesTask, StartTask03, osPriorityIdle, 0, 4096);
    servicesTaskHandle = osThreadCreate(osThread(servicesTask), NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
int server_socket;
void StartDefaultTask(void const * argument)
{
  /* init code for LWIP */
  MX_LWIP_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */

  printf("bootloader defult task started!!!\r\n");

  printf("wait for connection req from client\r\n");
  
  int client_sock = tcp_server();

  if(client_sock >= 0){
    xTaskNotify(feedDogTaskHandle, client_sock,  eNoAction);
    xTaskNotify(servicesTaskHandle, client_sock,  eNoAction);
  }

  for(;;)
  {
	  HAL_GPIO_WritePin(GPIOB, LED0_Pin, GPIO_PIN_SET);
	  osDelay(500);
	  HAL_GPIO_WritePin(GPIOB, LED0_Pin, GPIO_PIN_RESET);
	  osDelay(500);

	  if(jumpflag){
      printf("jump to APP\r\n");
      osDelay(2000);

      close(client_sock);
      close(server_socket);

      osDelay(2000);

      iap_load_app(FLASH_APP_ADDR);
    }

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
  int sock = 0;
  sock = ( ulTaskNotifyTake(pdFALSE,  portMAX_DELAY) )+1;

  printf("remote notify sock: %d\r\n", sock);

  osDelay(2000);

  uint32_t flashAddr = FLASH_APP_ADDR;
  printf("first flashaddr: %d\r\n", flashAddr);
  flashItem item_received;
  /* write flash */
  while (1)
  {
    if(xQueueReceive(flashQueue,  &item_received,  portMAX_DELAY)  ==  pdPASS)  {
        //  接收成功，处理数据
        //printf("Received item from the queue\r\n");
    }
    else{
        printf("Received from the queue failed!!!\r\n");
        continue;
    }

    if(xSemaphoreTake(flash_mutex,  portMAX_DELAY)  ==  pdTRUE)  {
      //  访问共享资源
      //flash
      flashAddr = flash_write(flashAddr, item_received.buf, item_received.len);

      if(item_received.len != 1024){
        //printf("this is the last flash!!!\r\n");
      }
      //  释放互斥量
      xSemaphoreGive(flash_mutex);
    }
    
  }
  
  /* USER CODE END StartTask02 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void print_addr(struct sockaddr_in* addr){

    char ipAddress[INET_ADDRSTRLEN];
    
    inet_ntop(AF_INET, &(addr->sin_addr), ipAddress, INET_ADDRSTRLEN);

    printf("remote ip: %s\r\n", ipAddress);

    uint16_t port = ntohs(addr->sin_port);

    printf("remote port: %d\r\n", port);

    return;
}


int tcp_server(){
    server_socket;
    struct  sockaddr_in  server_addr,  client_addr;
    socklen_t  client_len  =  sizeof(client_addr);
    int  i,  new_sock;

    server_socket  =  socket(AF_INET,  SOCK_STREAM,  0);
    if  (server_socket  <  0)  {
        //  错误处理
        return -1;
    }

    // struct linger ling;
    // ling.l_onoff = 1;
    // ling.l_linger = 0;

    // if(setsockopt(server_socket,  SOL_SOCKET,  SO_LINGER,  &ling,  sizeof(ling))  <  0){
    //   printf("set server socket linger off failed!!\r\n");
    //   // const  char  *err_str  =  strerror(errno);
    //   // printf("Error  occurred:  %s\r\n",  err_str);
    //   close(server_socket);
    //   return -1;
    // }

    server_addr.sin_family  =  AF_INET;
    server_addr.sin_addr.s_addr  =  INADDR_ANY;
    server_addr.sin_port  =  htons(12345);

    if  (bind(server_socket,  (struct  sockaddr  *)&server_addr,  sizeof(server_addr))  <  0)  {
        //  错误处理
        printf("bind failed!!\r\n");
        close(server_socket);
        return -1;
    }

    if  (listen(server_socket,  3)  <  0)  {
        //  错误处理
        printf("listen error\r\n");
        closesocket(server_socket);
        return -1;
    }

    new_sock  =  accept(server_socket,  (struct  sockaddr  *)&client_addr,  &client_len);
    printf("new_sock: %d\r\n", new_sock);
    if  (new_sock  <  0)  {
        printf("accept error!!!\r\n");
        return -1;
    }
    else{
      print_addr(&client_addr);

      // struct linger ling;
      // ling.l_onoff = 1;
      // ling.l_linger = 0;

      // if(setsockopt(new_sock,  SOL_SOCKET,  SO_LINGER,  &ling,  sizeof(ling))  <  0){
      //   printf("set server socket linger off failed!!\r\n");
      //   // const  char  *err_str  =  strerror(errno);
      //   // printf("Error  occurred:  %s\r\n",  err_str);
      //   close(new_sock);
      //   return -1;
      // }
        return new_sock;
    }
}

int messageAnalysis(uint8_t *buf, int len, frame *message){
  if(len < 2) return -1;
  
  // message->cmd = (uint16_t)( (((uint16_t)(buf[0]<<8)) && 0xFF00) | (((uint16_t)(buf[1])) && 0x00FF) );
  // message->subfunc = (uint16_t)( (((uint16_t)(buf[2]<<8)) && 0xFF00) | (((uint16_t)(buf[3])) && 0x00FF) );

  message->cmd = buf[0];
  message->subfunc = buf[1];

  if(len > 2){
    message->data = buf+2;
    message->len = len-2;
  }

  return 0;
}

void StartTask03(void const * argument)
{
  /*
      单线程：接收、处理、发送
      多线程：接收线程、发送线程、服务处理，并不是异步，因为上位机是同步的，即使多线程下位机也相当于同步
  */
  int sock = 0;
  sock = ( ulTaskNotifyTake(pdFALSE,  portMAX_DELAY) )+1;

  osDelay(2000);

  uint8_t *rxbuffer = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * 1048);
  uint8_t *txbuffer = (uint8_t *)pvPortMalloc(sizeof(uint8_t) * 32);

  while(1){
    int len = recv(sock, rxbuffer, 1048, 0);

    if(len>0){
      //printf("receive from client %d bytes\r\n", len);
    }
    else{
      printf("receive nothing \r\n");
      close(sock);
      break;
    }

    //analysis message
    frame message;

    if(messageAnalysis(rxbuffer, len, &message) < 0){
      printf("message analysis failed!!!\r\n");
      //send error frame warning...
      continue;
    }
    else{

    }

    //service analysis
    int lensend = 0;
    errorCode err = servicesAnalysis(&message, txbuffer, &lensend);
    if(err != NOERR){
      char errstr[30];
      analysisErr(err, errstr, 30);
      printf("service err: %s!!!\r\n", errstr);
      continue;
    }

    //send response
    if(send(sock, txbuffer, lensend, 0) < 0 ){
      printf("send faild!!!\r\n");
    }

    if(APPValid) break;

  }

  while (1)
  {
    /* code */
  }
  
  
}

/* USER CODE END Application */

