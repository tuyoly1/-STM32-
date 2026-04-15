#include "usart.h"
#include <stdio.h>
#include <string.h>

#if (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t");
__asm(".global __ARM_use_no_argv \n\t");
#else
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
#endif

int _ttywrch(int ch) { return ch; }
void _sys_exit(int x) { x = x; }
char *_sys_command_string(char *cmd, int len) { return NULL; }
FILE __stdout;

// ==========================================
// printf 重定向使用 huart5 (UART5) 调试输出
// ==========================================
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart5, (uint8_t *)&ch, 1, 0xffff);
    return ch;
}

int fgetc(FILE *f)
{
    uint8_t ch = 0;
    HAL_UART_Receive(&huart5, &ch, 1, 0xffff);
    return ch;
}

void UsartReceive_IDLE(UART_HandleTypeDef *huart);

// 跨文件标志位
extern volatile uint8_t face_event_ready;
extern volatile uint8_t face_result;
extern volatile uint16_t face_id;

uint8_t g_usart_rx2_buf[USART_REC_LEN];
uint16_t g_usart_rx_sta = 0;

// TX510 串口帧缓存
static uint8_t tx510_frame[16] = {0};
static uint8_t tx510_idx = 0;

// ===== 指纹模块接收缓冲：改为 UART4 使用 =====
// 这里仍沿用你头文件里的 RXBUFFERSIZE_UART1 宏，避免你 usart.h 也得大改
uint8_t aRxBuffer[RXBUFFERSIZE_UART1];
uint8_t RX_len;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
UART_HandleTypeDef huart5;

// ==========================================
// 指纹模块 UART4 初始化（原 USART1 改到 UART4）
// ==========================================
void MX_UART4_Init(void)
{
    huart4.Instance = UART4;
    huart4.Init.BaudRate = 57600;
    huart4.Init.WordLength = UART_WORDLENGTH_8B;
    huart4.Init.StopBits = UART_STOPBITS_1;
    huart4.Init.Parity = UART_PARITY_NONE;
    huart4.Init.Mode = UART_MODE_TX_RX;
    huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart4.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart4) != HAL_OK) Error_Handler();

    if (HAL_UART_Receive_IT(&huart4, (uint8_t *)aRxBuffer, RXBUFFERSIZE_UART1) != HAL_OK)
        Error_Handler();

    // 开启 UART4 空闲中断，用于 AS608 返回包接收完成判断
    __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
}

void MX_USART2_UART_Init(void)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = 115200;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = UART_PARITY_NONE;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart2) != HAL_OK) Error_Handler();

    // 人脸模块：直接开底层接收中断
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
    __HAL_UART_ENABLE_IT(&huart2, UART_IT_ERR);
}

void MX_USART3_UART_Init(void)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart3) != HAL_OK) Error_Handler();
}

// ==========================================
// UART5 初始化（调试口）
// ==========================================
void MX_UART5_Init(void)
{
    huart5.Instance = UART5;
    huart5.Init.BaudRate = 115200;
    huart5.Init.WordLength = UART_WORDLENGTH_8B;
    huart5.Init.StopBits = UART_STOPBITS_1;
    huart5.Init.Parity = UART_PARITY_NONE;
    huart5.Init.Mode = UART_MODE_TX_RX;
    huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart5.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart5) != HAL_OK) Error_Handler();
}

void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // ==========================================
    // UART4: 指纹模块
    // TX -> PC10
    // RX -> PC11
    // ==========================================
    if (uartHandle->Instance == UART4)
    {
        __HAL_RCC_UART4_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();

        // UART4 TX (PC10)
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // UART4 RX (PC11)
        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(UART4_IRQn, 3, 3);
        HAL_NVIC_EnableIRQ(UART4_IRQn);
    }
    else if (uartHandle->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    else if (uartHandle->Instance == USART3)
    {
        __HAL_RCC_USART3_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    else if (uartHandle->Instance == UART5)
    {
        __HAL_RCC_UART5_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        // UART5 TX (PC12)
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

        // UART5 RX (PD2)
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
    if (uartHandle->Instance == UART4)
    {
        __HAL_RCC_UART4_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10 | GPIO_PIN_11);
        HAL_NVIC_DisableIRQ(UART4_IRQn);
    }
    else if (uartHandle->Instance == USART2)
    {
        __HAL_RCC_USART2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2 | GPIO_PIN_3);
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
    else if (uartHandle->Instance == USART3)
    {
        __HAL_RCC_USART3_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
    }
    else if (uartHandle->Instance == UART5)
    {
        __HAL_RCC_UART5_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_12);
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
    }
}

// =======================================================================
// TX510 人脸模块底层裸机接收中断解析
// =======================================================================
static void TX510_ParseByte(uint8_t rx_byte)
{
    // 等待帧头 EF
    if (tx510_idx == 0)
    {
        if (rx_byte == 0xEF)
        {
            tx510_frame[tx510_idx++] = rx_byte;
        }
        return;
    }

    // 等待帧头 AA
    if (tx510_idx == 1)
    {
        if (rx_byte == 0xAA)
        {
            tx510_frame[tx510_idx++] = rx_byte;
        }
        else
        {
            tx510_idx = 0;
            if (rx_byte == 0xEF)
            {
                tx510_frame[tx510_idx++] = rx_byte;
            }
        }
        return;
    }

    // 继续收帧
    tx510_frame[tx510_idx++] = rx_byte;

    // 至少收够前7字节，才能知道 Size
    if (tx510_idx < 7)
        return;

    // 只处理 Reply_MsgID=0x00 的返回帧
    if (tx510_frame[2] != 0x00)
    {
        tx510_idx = 0;
        return;
    }

    // Size 是 4 字节，大端
    uint32_t size = ((uint32_t)tx510_frame[3] << 24) |
                    ((uint32_t)tx510_frame[4] << 16) |
                    ((uint32_t)tx510_frame[5] << 8)  |
                    ((uint32_t)tx510_frame[6]);

    // 成功帧总长 12，失败帧总长 10
    uint8_t expected_len = 0;
    if (size == 4) expected_len = 12;
    else if (size == 2) expected_len = 10;
    else
    {
        if (tx510_idx >= sizeof(tx510_frame)) tx510_idx = 0;
        return;
    }

    if (tx510_idx < expected_len)
        return;

    // 识别返回必须是 MsgID=0x12
    if (tx510_frame[7] != 0x12)
    {
        tx510_idx = 0;
        return;
    }

    // 校验和：除去 EF AA 后，其余字节做加法
    uint8_t sum = 0;
    for (uint8_t i = 2; i < expected_len - 1; i++)
    {
        sum += tx510_frame[i];
    }

    if (sum != tx510_frame[expected_len - 1])
    {
        tx510_idx = 0;
        return;
    }

    face_result = tx510_frame[8];
    face_id = 0xFFFF;

    if (size == 4 && face_result == 0x00)
    {
        face_id = ((uint16_t)tx510_frame[9] << 8) | tx510_frame[10];
    }

    face_event_ready = 1;
    tx510_idx = 0;
}

void USART2_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_ORE) != RESET ||
        __HAL_UART_GET_FLAG(&huart2, UART_FLAG_FE)  != RESET ||
        __HAL_UART_GET_FLAG(&huart2, UART_FLAG_NE)  != RESET)
    {
        volatile uint32_t tmp = USART2->SR;
        tmp = USART2->DR;
        (void)tmp;
    }

    if (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) != RESET)
    {
        uint8_t rx_byte = (uint8_t)(USART2->DR & 0x00FF);
        TX510_ParseByte(rx_byte);
    }
}

// ==========================================
// 指纹串口回调：改为 UART4
// ==========================================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        // 这里保留即可，AS608 主要依赖空闲中断收包
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
        HAL_UART_Receive_IT(&huart4, (uint8_t *)aRxBuffer, RXBUFFERSIZE_UART1);
    }
}

void UsartReceive_IDLE(UART_HandleTypeDef *huart)
{
    __HAL_UART_CLEAR_IDLEFLAG(&huart4);
    RX_len = RXBUFFERSIZE_UART1 - huart4.RxXferCount;
    HAL_UART_AbortReceive_IT(huart);
    HAL_UART_Receive_IT(&huart4, (uint8_t *)aRxBuffer, RXBUFFERSIZE_UART1);
}

