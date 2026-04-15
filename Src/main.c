#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "tim.h"
#include "AS608.h"
#include "RC522.h"
#include "OLED.h"
#include "key.h"
#include "myflash.h"
#include "Servo.h"
#include <stdio.h>
#include <string.h>

// ==========================================
// 定义蜂鸣器引脚 (PB13) - 低电平触发
// ==========================================
#define BUZZER_PORT GPIOB
#define BUZZER_PIN  GPIO_PIN_13

SysPara AS608Para;
uint16_t ValidN;

extern uint8_t UID[4], Temp[4];
extern uint8_t UI0[4];
extern uint8_t UI1[4];
extern uint8_t UI2[4];
extern uint8_t UI3[4];

volatile uint8_t face_event_ready = 0;
volatile uint8_t face_result = 0xFF;   // 0x00成功，其他失败码
volatile uint16_t face_id = 0xFFFF;    // 成功时有效
uint8_t as608_ready = 0;

// 连续失败计数器
uint8_t fail_count = 0;

static uint8_t menu_page = 0;
static uint8_t keynum = 0;
static uint8_t menu_select = 1;
static uint8_t ID_select = 0;

void SystemClock_Config(void);
void Read_Card_Flash(void);
int press_FR(uint16_t *finger_id);
void Add_Card_Flash(void);
void Del_FR(void);
void Buzzer_Init(void);

void UID_ToString(uint8_t *uid, char *out);
void Host_SendText(const char *text);
void Host_SendCardEvent(const char *prefix, uint8_t *uid);
void Host_SendFingerEvent(const char *prefix, uint16_t finger_id);

void Host_SendFaceOk(uint16_t face_id);
void Host_SendFaceFail(const char *reason);

extern UART_HandleTypeDef huart3;

void UID_ToString(uint8_t *uid, char *out)
{
    sprintf(out, "%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
}

void Host_SendText(const char *text)
{
    HAL_StatusTypeDef ret;

    ret = HAL_UART_Transmit(&huart3, (uint8_t *)text, strlen(text), 200);

    // 走 UART5 调试口输出，方便你看发送是否真的成功
    if (ret == HAL_OK)
    {
        printf("UART3 SEND OK: %s", text);
    }
    else if (ret == HAL_TIMEOUT)
    {
        printf("UART3 SEND TIMEOUT: %s\r\n", text);
    }
    else if (ret == HAL_BUSY)
    {
        printf("UART3 SEND BUSY: %s\r\n", text);
    }
    else
    {
        printf("UART3 SEND ERROR: %s\r\n", text);
    }
}

void Host_SendCardEvent(const char *prefix, uint8_t *uid)
{
    char uid_str[16];
    char msg[32];

    UID_ToString(uid, uid_str);
    sprintf(msg, "%s,%s\r\n", prefix, uid_str);
    Host_SendText(msg);
}

void Host_SendFingerEvent(const char *prefix, uint16_t finger_id)
{
    char msg[32];
    sprintf(msg, "%s,%d\r\n", prefix, finger_id);
    Host_SendText(msg);
}

void Host_SendFaceOk(uint16_t face_id)
{
    char msg[32];
    sprintf(msg, "FACE_OK,%d\r\n", face_id);
    Host_SendText(msg);
}

void Host_SendFaceFail(const char *reason)
{
    char msg[48];
    sprintf(msg, "FACE_FAIL,%s\r\n", reason);
    Host_SendText(msg);
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_TIM4_Init();

    // 初始化蜂鸣器引脚
    Buzzer_Init();

    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);

    // ===== 指纹模块改到 UART4 =====
    MX_UART4_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    MX_UART5_Init();

    OLED_Init();
    OLED_ShowString(1, 1, "  Welcome Home! ");
    OLED_ShowString(2, 1, "   loading...   ");

    RFID_Init();
    Key_Init();
    Servo_Init();

    uint8_t try_cnt = 0;
    while (GZ_HandShake(&AS608Addr))
    {
        HAL_Delay(100);
        try_cnt++;
        if (try_cnt >= 10)
        {
            OLED_ShowString(2, 1, " AS608 ERROR!   ");
            HAL_Delay(1500);
            as608_ready = 0;
            break;
        }
    }

    if (try_cnt < 10)
    {
        as608_ready = 1;
        OLED_ShowString(2, 1, " AS608 OK!      ");
        HAL_Delay(1000);
    }

    Read_Card_Flash();
    OLED_ShowString(2, 1, "  INIT SUCCESS! ");
    HAL_Delay(1000);

    uint32_t rfid_crash_cnt = 0;

    while (1)
    {
        OLED_Clear();

        switch (menu_page)
        {
        case 0:
            OLED_ShowString(1, 1, "  Please unlock");
            while (1)
            {
                // ==========================================
                // 报警统一处理
                // ==========================================
                if (fail_count >= 3)
                {
                    OLED_Clear();
                    OLED_ShowString(1, 1, "!!! WARNING !!!");
                    OLED_ShowString(2, 1, " SYSTEM LOCKED ");
                    OLED_ShowString(3, 1, " Unauthorized! ");

                    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
                    HAL_Delay(1500);
                    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);

                    fail_count = 0;

                    OLED_Clear();
                    OLED_ShowString(1, 1, "  Please unlock");
                }

                // ==== 检测人脸识别 ====
                if (face_event_ready)
                {
                    face_event_ready = 0;

                    if (face_result == 0x00)
                    {
                        fail_count = 0;

                        OLED_Clear();
                        OLED_ShowString(1, 1, "Unlock success");
                        OLED_ShowString(2, 1, "Face unlock");

                        Host_SendText("FACE_DEBUG,ENTER_OK\r\n");
                        Host_SendFaceOk(face_id);
                        HAL_Delay(50);

                        ControlServo();
                        HAL_Delay(1500);
                        break;
                    }
                    else
                    {
                        fail_count++;

                        OLED_Clear();
                        OLED_ShowString(1, 1, "Unlock failed");

                        if (face_result == 0x01)
                        {
                            OLED_ShowString(2, 1, "No face");
                            Host_SendText("FACE_DEBUG,ENTER_FAIL\r\n");
                            Host_SendFaceFail("NO_FACE");
                        }
                        else if (face_result == 0x06)
                        {
                            OLED_ShowString(2, 1, "2D live fail");
                            Host_SendFaceFail("LIVENESS_2D");
                        }
                        else if (face_result == 0x07)
                        {
                            OLED_ShowString(2, 1, "3D live fail");
                            Host_SendFaceFail("LIVENESS_3D");
                        }
                        else if (face_result == 0x08)
                        {
                            OLED_ShowString(2, 1, "Face mismatch");
                            Host_SendFaceFail("MATCH_FAIL");
                        }
                        else
                        {
                            OLED_ShowString(2, 1, "Face failed");
                            Host_SendFaceFail("UNKNOWN");
                        }

                        HAL_Delay(1500);
                        break;
                    }
                }

                // ==== 检测按键 ====
                keynum = Key_Scan();
                if (keynum == 1)
                {
                    menu_page = 1;
                    break;
                }

                // ==== RC522 防死机心跳 ====
                rfid_crash_cnt++;
                if (rfid_crash_cnt > 30000)
                {
                    RFID_Init();
                    rfid_crash_cnt = 0;
                }

                // ==== 检测IC卡 ====
                if (PcdRequest(REQ_ALL, Temp) == MI_OK)
                {
                    rfid_crash_cnt = 0;

                    if (PcdAnticoll(UID) == MI_OK)
                    {
                        if (UID[0] == UI0[0] && UID[1] == UI0[1] && UID[2] == UI0[2] && UID[3] == UI0[3])
                        {
                            fail_count = 0;
                            OLED_Clear();
                            OLED_ShowString(1, 1, "Unlock success");
                            OLED_ShowString(2, 1, "card ID: 0");

                            Host_SendCardEvent("CARD_OK", UID);

                            ControlServo();
                            menu_page = 0;
                            break;
                        }
                        else if (UID[0] == UI1[0] && UID[1] == UI1[1] && UID[2] == UI1[2] && UID[3] == UI1[3])
                        {
                            fail_count = 0;
                            OLED_Clear();
                            OLED_ShowString(1, 1, "Unlock success");
                            OLED_ShowString(2, 1, "card ID: 1");

                            Host_SendCardEvent("CARD_OK", UID);

                            ControlServo();
                            menu_page = 0;
                            break;
                        }
                        else if (UID[0] == UI2[0] && UID[1] == UI2[1] && UID[2] == UI2[2] && UID[3] == UI2[3])
                        {
                            fail_count = 0;
                            OLED_Clear();
                            OLED_ShowString(1, 1, "Unlock success");
                            OLED_ShowString(2, 1, "card ID: 2");

                            Host_SendCardEvent("CARD_OK", UID);

                            ControlServo();
                            menu_page = 0;
                            break;
                        }
                        else if (UID[0] == UI3[0] && UID[1] == UI3[1] && UID[2] == UI3[2] && UID[3] == UI3[3])
                        {
                            fail_count = 0;
                            OLED_Clear();
                            OLED_ShowString(1, 1, "Unlock success");
                            OLED_ShowString(2, 1, "card ID: 3");

                            Host_SendCardEvent("CARD_OK", UID);

                            ControlServo();
                            menu_page = 0;
                            break;
                        }
                        else
                        {
                            fail_count++;
                            OLED_Clear();
                            OLED_ShowString(1, 1, "Unlock failed");
                            OLED_ShowString(2, 1, "Invalid card");
                            OLED_ShowString(3, 1, "Please try again");

                            Host_SendCardEvent("DENY_CARD", UID);

                            HAL_Delay(1500);
                            menu_page = 0;
                            break;
                        }
                    }
                }
                // ==== 检测指纹 ====
                else if (as608_ready == 1)
                {
                    uint16_t finger_id = 0xFFFF;
                    int fr_res = press_FR(&finger_id);

                    if (fr_res == 0) // 成功
                    {
                        fail_count = 0;
                        Host_SendFingerEvent("FINGER_OK", finger_id);
                        menu_page = 0;
                        break;
                    }
                    else if (fr_res == 1 || fr_res == 2) // 失败
                    {
                        fail_count++;
                        Host_SendText("FINGER_FAIL,UNKNOWN\r\n");
                        menu_page = 0;
                        break;
                    }
                }
            }
            break;

        case 1:
            OLED_Clear();
            switch (menu_select)
            {
            case 1:
                OLED_ShowString(1, 1, "-->Add card");
                OLED_ShowString(2, 1, "   Add finger");
                OLED_ShowString(3, 1, "   Delete card");
                OLED_ShowString(4, 1, "   Delete finger");
                break;
            case 2:
                OLED_ShowString(1, 1, "   Add card");
                OLED_ShowString(2, 1, "-->Add finger");
                OLED_ShowString(3, 1, "   Delete card");
                OLED_ShowString(4, 1, "   Delete finger");
                break;
            case 3:
                OLED_ShowString(1, 1, "   Add card");
                OLED_ShowString(2, 1, "   Add finger");
                OLED_ShowString(3, 1, "-->Delete card");
                OLED_ShowString(4, 1, "   Delete finger");
                break;
            case 4:
                OLED_ShowString(1, 1, "   Add card");
                OLED_ShowString(2, 1, "   Add finger");
                OLED_ShowString(3, 1, "   Delete card");
                OLED_ShowString(4, 1, "-->Delete finger");
                break;
            }

            while (1)
            {
                keynum = Key_Scan();
                if (keynum == 1)
                {
                    menu_select++;
                    if (menu_select > 4) menu_select = 1;
                    break;
                }
                else if (keynum == 2)
                {
                    if (menu_select == 1) { menu_page = 2; break; }
                    else if (menu_select == 2) { menu_page = 3; break; }
                    else if (menu_select == 3) { menu_page = 4; break; }
                    else if (menu_select == 4) { menu_page = 5; break; }
                }
                else if (keynum == 3 || keynum == 4)
                {
                    menu_page = 0;
                    break;
                }
            }
            break;

        case 2:
            OLED_Clear();
            OLED_ShowString(2, 1, "Please Add Card");
            while (1)
            {
                keynum = Key_Scan();
                if (keynum == 1) { ID_select++; if (ID_select > 3) ID_select = 0; }
                else if (keynum == 4) { menu_page = 0; break; }
                else if (keynum == 3) { menu_page = 1; break; }

                if (ID_select == 0) OLED_ShowString(3, 1, "   Card 0");
                else if (ID_select == 1) OLED_ShowString(3, 1, "   Card 1");
                else if (ID_select == 2) OLED_ShowString(3, 1, "   Card 2");
                else if (ID_select == 3) OLED_ShowString(3, 1, "   Card 3");

                if (PcdRequest(REQ_ALL, Temp) == MI_OK)
                {
                    if (PcdAnticoll(UID) == MI_OK)
                    {
                        if (ID_select == 0) { UI0[0] = UID[0]; UI0[1] = UID[1]; UI0[2] = UID[2]; UI0[3] = UID[3]; }
                        else if (ID_select == 1) { UI1[0] = UID[0]; UI1[1] = UID[1]; UI1[2] = UID[2]; UI1[3] = UID[3]; }
                        else if (ID_select == 2) { UI2[0] = UID[0]; UI2[1] = UID[1]; UI2[2] = UID[2]; UI2[3] = UID[3]; }
                        else if (ID_select == 3) { UI3[0] = UID[0]; UI3[1] = UID[1]; UI3[2] = UID[2]; UI3[3] = UID[3]; }

                        Add_Card_Flash();
                        OLED_Clear();
                        OLED_ShowString(2, 1, "   Add card");
                        OLED_ShowString(3, 1, "   success");
                        HAL_Delay(3000);
                        menu_page = 1;
                        ID_select = 0;
                        break;
                    }
                }
            }
            break;

        case 3:
            OLED_Clear();
            OLED_ShowString(2, 1, "  please finger1");
            while (1)
            {
                uint8_t ensure = 0;

                keynum = Key_Scan();
                if (keynum == 1) { ID_select++; if (ID_select > 3) ID_select = 0; }
                else if (keynum == 2) { menu_page = 1; }
                else if (keynum == 3) { menu_page = 1; break; }
                else if (keynum == 4) { menu_page = 0; break; }

                if (ID_select == 0) OLED_ShowString(3, 1, "   Finger 0");
                else if (ID_select == 1) OLED_ShowString(3, 1, "   Finger 1");
                else if (ID_select == 2) OLED_ShowString(3, 1, "   Finger 2");
                else if (ID_select == 3) OLED_ShowString(3, 1, "   Finger 3");

                ensure = GZ_GetImage();
                if (ensure == 0x00)
                {
                    ensure = GZ_GenChar(CharBuffer1);
                    if (ensure == 0x00)
                    {
                        ensure = GZ_GetImage();
                        if (ensure == 0x00)
                        {
                            ensure = GZ_GenChar(CharBuffer2);
                            if (ensure == 0x00)
                            {
                                ensure = GZ_Match();
                                if (ensure == 0x00)
                                {
                                    ensure = GZ_RegModel();
                                    if (ensure == 0x00)
                                    {
                                        ensure = GZ_StoreChar(CharBuffer2, ID_select);
                                        if (ensure == 0x00)
                                        {
                                            GZ_ValidTempleteNum(&ValidN);
                                            OLED_Clear();
                                            OLED_ShowString(2, 1, "  Add finger");
                                            OLED_ShowString(3, 1, "   success");
                                            HAL_Delay(3000);
                                            menu_page = 1;
                                            ID_select = 0;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;

        case 4:
            OLED_Clear();
            OLED_ShowString(2, 1, "  Delete card1");
            while (1)
            {
                keynum = Key_Scan();
                if (keynum == 1) { ID_select++; if (ID_select > 3) ID_select = 0; }
                else if (keynum == 2)
                {
                    UID[0] = 0; UID[1] = 0; UID[2] = 0; UID[3] = 0;

                    if (ID_select == 0) { UI0[0] = UID[0]; UI0[1] = UID[1]; UI0[2] = UID[2]; UI0[3] = UID[3]; }
                    else if (ID_select == 1) { UI1[0] = UID[0]; UI1[1] = UID[1]; UI1[2] = UID[2]; UI1[3] = UID[3]; }
                    else if (ID_select == 2) { UI2[0] = UID[0]; UI2[1] = UID[1]; UI2[2] = UID[2]; UI2[3] = UID[3]; }
                    else if (ID_select == 3) { UI3[0] = UID[0]; UI3[1] = UID[1]; UI3[2] = UID[2]; UI3[3] = UID[3]; }

                    Add_Card_Flash();
                    OLED_Clear();
                    OLED_ShowString(2, 1, "  Delete card");
                    OLED_ShowString(3, 1, "   success");
                    HAL_Delay(3000);
                    menu_page = 1;
                    ID_select = 0;
                    break;
                }
                else if (keynum == 3) { menu_page = 1; break; }
                else if (keynum == 4) { menu_page = 0; break; }

                if (ID_select == 0) OLED_ShowString(3, 1, "   Card 0");
                else if (ID_select == 1) OLED_ShowString(3, 1, "   Card 1");
                else if (ID_select == 2) OLED_ShowString(3, 1, "   Card 2");
                else if (ID_select == 3) OLED_ShowString(3, 1, "   Card 3");
            }
            break;

        case 5:
            OLED_Clear();
            OLED_ShowString(2, 1, "  Delete finger1");
            while (1)
            {
                keynum = Key_Scan();
                if (keynum == 1) { ID_select++; if (ID_select > 3) ID_select = 0; }
                else if (keynum == 2)
                {
                    Del_FR();
                    OLED_Clear();
                    OLED_ShowString(2, 1, "  Delete Finger");
                    OLED_ShowString(3, 1, "   success");
                    HAL_Delay(3000);
                    menu_page = 1;
                    ID_select = 0;
                    break;
                }
                else if (keynum == 3) { menu_page = 1; break; }
                else if (keynum == 4) { menu_page = 0; break; }

                if (ID_select == 0) OLED_ShowString(3, 1, "   Finger 0");
                else if (ID_select == 1) OLED_ShowString(3, 1, "   Finger 1");
                else if (ID_select == 2) OLED_ShowString(3, 1, "   Finger 2");
                else if (ID_select == 3) OLED_ShowString(3, 1, "   Finger 3");
            }
            break;
        }
    }
}

// ==========================================
// 蜂鸣器初始化（低电平触发版）
// ==========================================
void Buzzer_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);

    GPIO_InitStruct.Pin = BUZZER_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUZZER_PORT, &GPIO_InitStruct);
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) Error_Handler();

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

void Error_Handler(void) {}

void Read_Card_Flash(void)
{
    uint16_t cardID[2];

    stmflash_read(0X08009000, cardID, 2);
    UI0[0] = cardID[0] >> 8; UI0[1] = cardID[0]; UI0[2] = cardID[1] >> 8; UI0[3] = cardID[1];

    stmflash_read(0X08009004, cardID, 2);
    UI1[0] = cardID[0] >> 8; UI1[1] = cardID[0]; UI1[2] = cardID[1] >> 8; UI1[3] = cardID[1];

    stmflash_read(0X08009008, cardID, 2);
    UI2[0] = cardID[0] >> 8; UI2[1] = cardID[0]; UI2[2] = cardID[1] >> 8; UI2[3] = cardID[1];

    stmflash_read(0X0800900C, cardID, 2);
    UI3[0] = cardID[0] >> 8; UI3[1] = cardID[0]; UI3[2] = cardID[1] >> 8; UI3[3] = cardID[1];
}

int press_FR(uint16_t *finger_id)
{
    SearchResult seach;
    uint8_t ensure;

    *finger_id = 0xFFFF;

    ensure = GZ_GetImage();
    if (ensure == 0x00)
    {
        ensure = GZ_GenChar(CharBuffer1);
        if (ensure == 0x00)
        {
            ensure = GZ_HighSpeedSearch(CharBuffer1, 0, 300, &seach);
            if (ensure == 0x00)
            {
                if (seach.mathscore > 100)
                {
                    *finger_id = seach.pageID;

                    OLED_Clear();
                    OLED_ShowString(1, 1, "Unlock success");
                    OLED_ShowString(2, 1, "Finger ID:");
                    OLED_ShowNum(2, 12, seach.pageID, 1);

                    ControlServo();
                    HAL_Delay(1500);
                    return 0;
                }
                else
                {
                    OLED_Clear();
                    OLED_ShowString(1, 1, "Unlock failed");
                    HAL_Delay(1500);
                    return 1;
                }
            }
            else
            {
                OLED_Clear();
                OLED_ShowString(1, 1, "Unlock failed");
                HAL_Delay(1500);
                return 2;
            }
        }
    }

    return -1;
}

void Del_FR(void)
{
    GZ_DeletChar(ID_select, 1);
}

void Add_Card_Flash(void)
{
    uint16_t cardID[2];
    cardID[0] = UID[0] << 8 | UID[1];
    cardID[1] = UID[2] << 8 | UID[3];

    if (ID_select == 0) stmflash_write(0X08009000, cardID, 2);
    else if (ID_select == 1) stmflash_write(0X08009004, cardID, 2);
    else if (ID_select == 2) stmflash_write(0X08009008, cardID, 2);
    else if (ID_select == 3) stmflash_write(0X0800900C, cardID, 2);
}

