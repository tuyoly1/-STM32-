#include "main.h" // Device header

void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 1. 开启 GPIOC 时钟 (原代码是 GPIOB)
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Configure GPIO pins : PC0 PC1 PC2 PC3 */
    // 2. 配置引脚为 PC0, PC1, PC2, PC3
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    
    // 3. 模式：上拉输入 (默认高电平，按下接地变低电平)
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    
    // 4. 初始化 GPIOC 端口
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

// 按键扫描函数
uint8_t Key_Scan(void)
{
    uint8_t keynum = 0;

    // --- 检测按键 1 (PC0) ---
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == 0) // 读取 PC0
    {
        HAL_Delay(20); // 消抖
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == 0)
        {
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == 0); // 等待松手
            keynum = 1;
        }
    }

    // --- 检测按键 2 (PC1) ---
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == 0) // 读取 PC1
    {
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == 0)
        {
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_1) == 0);
            keynum = 2;
        }
    }

    // --- 检测按键 3 (PC2) ---
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == 0) // 读取 PC2
    {
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == 0)
        {
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == 0);
            keynum = 3;
        }
    }

    // --- 检测按键 4 (PC3) ---
    if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == 0) // 读取 PC3
    {
        HAL_Delay(20);
        if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == 0)
        {
            while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_3) == 0);
            keynum = 4;
        }
    }

    return keynum;
}

