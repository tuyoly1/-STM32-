/**
 ******************************************************************************
 * @file    usart.h
 * @brief   This file contains all the function prototypes for
 *          the usart.c file
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

#define USART_REC_LEN 200      /* 定义最大接收字节数 200 */
#define RXBUFFERSIZE_UART2 1   /* 串口2接收缓存大小 */
#define RXBUFFERSIZE_UART1 500 /* 指纹模块接收缓存大小（现在实际给 UART4 用） */

/* 串口句柄声明 */
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart5;

/* 接收缓冲区声明 */
extern uint8_t aRxBuffer[RXBUFFERSIZE_UART1];
extern uint8_t RX_len;

extern uint8_t g_usart_rx2_buf[USART_REC_LEN];
extern uint16_t g_usart_rx_sta;

/* 初始化函数声明 */
void MX_UART4_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);
void MX_UART5_Init(void);

/* 其他函数声明 */
void UsartReceive_IDLE(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
