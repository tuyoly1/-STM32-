#include "main.h"
#include "RC522.H"

uint8_t UID[4], Temp[4];

// 授权卡 ID 定义
uint8_t UI0[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // 卡0 ID号
uint8_t UI1[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // 卡1 ID号
uint8_t UI2[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // 卡2 ID号
uint8_t UI3[4] = {0xFF, 0xFF, 0xFF, 0xFF}; // 卡3 ID号

/**
 * @brief  RC522 接口 GPIO 初始化
 * @note   已修改为通用宏定义版，适配 GPIOF 和 GPIOC
 */
void PcdInit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 1. 开启时钟 */
    // 开启 GPIOG (RC522), GPIOC (按键)
    __HAL_RCC_GPIOG_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    
    // 保留 A/B 时钟开启，防止其他代码（如串口）需要
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* 2. 配置输出引脚的默认电平 */
    // RST(PG6), SCK(PG8) 默认拉低
    HAL_GPIO_WritePin(GPIOG, MF522_RST_Pin | MF522_SCK_Pin, GPIO_PIN_RESET);
    // SDA(PG7), MOSI(PG14) 默认拉高
    HAL_GPIO_WritePin(GPIOG, MF522_SDA_Pin | MF522_MOSI_Pin, GPIO_PIN_SET);

    /* 3. 配置 MISO (PG13) - 输入模式 */
    GPIO_InitStruct.Pin = MF522_MISO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(MF522_MISO_GPIO_Port, &GPIO_InitStruct);

    /* 4. 配置 RST (PG6) - 推挽输出 */
    GPIO_InitStruct.Pin = MF522_RST_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(MF522_RST_GPIO_Port, &GPIO_InitStruct);

    /* 5. 配置 SCK (PG8) - 推挽输出 */
    GPIO_InitStruct.Pin = MF522_SCK_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(MF522_SCK_GPIO_Port, &GPIO_InitStruct);

    /* 6. 配置 SDA (PG7) 和 MOSI (PG14) - 推挽输出 */
    GPIO_InitStruct.Pin = MF522_SDA_Pin | MF522_MOSI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct); // 统一用 GPIOG
}

/**
 * @brief  寻卡
 * @param  req_code   寻卡方式: 0x52 = 寻感应区内所有符合14443A标准的卡; 0x26 = 寻未进入休眠状态的卡
 * @param  pTagType   返回卡片类型代码
 * 0x4400 = Mifare_UltraLight
 * 0x0400 = Mifare_One(S50)
 * 0x0200 = Mifare_One(S70)
 * 0x0800 = Mifare_Pro(X)
 * 0x4403 = Mifare_DESFire
 * @retval MI_OK      成功
 */
char PcdRequest(unsigned char req_code, unsigned char *pTagType)
{
    char status;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x07);

    SetBitMask(TxControlReg, 0x03);

    ucComMF522Buf[0] = req_code;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 1, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x10))
    {
        *pTagType = ucComMF522Buf[0];
        *(pTagType + 1) = ucComMF522Buf[1];
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**
 * @brief  防冲突 (获取卡号)
 * @param  pSnr  [OUT] 返回卡片序列号，4字节
 * @retval MI_OK 成功
 */
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i, snr_check = 0;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ClearBitMask(Status2Reg, 0x08);
    WriteRawRC(BitFramingReg, 0x00);
    ClearBitMask(CollReg, 0x80);

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 2, ucComMF522Buf, &unLen);

    if (status == MI_OK)
    {
        for (i = 0; i < 4; i++)
        {
            *(pSnr + i) = ucComMF522Buf[i];
            snr_check ^= ucComMF522Buf[i];
        }
        if (snr_check != ucComMF522Buf[i])
        {
            status = MI_ERR;
        }
    }

    SetBitMask(CollReg, 0x80);
    return status;
}

/**
 * @brief  选定卡片
 * @param  pSnr  [IN] 卡片序列号，4字节
 * @retval MI_OK 成功
 */
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i = 0; i < 4; i++)
    {
        ucComMF522Buf[i + 2] = *(pSnr + i);
        ucComMF522Buf[6] ^= *(pSnr + i);
    }
    CalulateCRC(ucComMF522Buf, 7, &ucComMF522Buf[7]);

    ClearBitMask(Status2Reg, 0x08);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 9, ucComMF522Buf, &unLen);

    if ((status == MI_OK) && (unLen == 0x18))
    {
        status = MI_OK;
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**
 * @brief  验证卡片密码
 * @param  auth_mode [IN] 密码验证模式
 * 0x60 = 验证A密钥
 * 0x61 = 验证B密钥
 * @param  addr      [IN] 块地址
 * @param  pKey      [IN] 密码 (6字节)
 * @param  pSnr      [IN] 卡片序列号 (4字节)
 * @retval MI_OK     成功
 */
char PcdAuthState(unsigned char auth_mode, unsigned char addr, unsigned char *pKey, unsigned char *pSnr)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 2] = *(pKey + i);
    }
    for (i = 0; i < 6; i++)
    {
        ucComMF522Buf[i + 8] = *(pSnr + i);
    }

    status = PcdComMF522(PCD_AUTHENT, ucComMF522Buf, 12, ucComMF522Buf, &unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {
        status = MI_ERR;
    }

    return status;
}

/**
 * @brief  读取M1卡一块数据
 * @param  addr   [IN]  块地址
 * @param  pData  [OUT] 读出的数据，16字节
 * @retval MI_OK  成功
 */
char PcdRead(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);
    if ((status == MI_OK) && (unLen == 0x90))
    {
        for (i = 0; i < 16; i++)
        {
            *(pData + i) = ucComMF522Buf[i];
        }
    }
    else
    {
        status = MI_ERR;
    }

    return status;
}

/**
 * @brief  写数据到M1卡的一块
 * @param  addr   [IN] 块地址
 * @param  pData  [IN] 写入的数据，16字节
 * @retval MI_OK  成功
 */
char PcdWrite(unsigned char addr, unsigned char *pData)
{
    char status;
    unsigned int unLen;
    unsigned char i, ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {
        status = MI_ERR;
    }

    if (status == MI_OK)
    {
        for (i = 0; i < 16; i++)
        {
            ucComMF522Buf[i] = *(pData + i);
        }
        CalulateCRC(ucComMF522Buf, 16, &ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 18, ucComMF522Buf, &unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {
            status = MI_ERR;
        }
    }

    return status;
}

/**
 * @brief  命令卡片进入休眠状态
 * @retval MI_OK 成功
 */
char PcdHalt(void)
{
    unsigned int unLen;
    unsigned char ucComMF522Buf[MAXRLEN];

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf, 2, &ucComMF522Buf[2]);

    PcdComMF522(PCD_TRANSCEIVE, ucComMF522Buf, 4, ucComMF522Buf, &unLen);

    return MI_OK;
}

/**
 * @brief  用RC522计算CRC16
 * @param  pIndata  [IN]  输入数据
 * @param  len      [IN]  数据长度
 * @param  pOutData [OUT] 输出结果 (2字节)
 */
void CalulateCRC(unsigned char *pIndata, unsigned char len, unsigned char *pOutData)
{
    unsigned char i, n;
    ClearBitMask(DivIrqReg, 0x04);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);
    for (i = 0; i < len; i++)
    {
        WriteRawRC(FIFODataReg, *(pIndata + i));
    }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/**
 * @brief  复位RC522
 * @retval MI_OK 成功
 */
char PcdReset(void)
{
    RST_H;
    delay_10ms(1);
    RST_L;
    delay_10ms(1);
    RST_H;
    delay_10ms(10);

    if (ReadRawRC(0x02) == 0x80)
    {
    }

    WriteRawRC(CommandReg, PCD_RESETPHASE);

    WriteRawRC(ModeReg, 0x3D); // 和Mifare卡通讯，CRC初始值0x6363
    WriteRawRC(TReloadRegL, 30);
    WriteRawRC(TReloadRegH, 0);
    WriteRawRC(TModeReg, 0x8D);
    WriteRawRC(TPrescalerReg, 0x3E);
    WriteRawRC(TxAutoReg, 0x40);
    return MI_OK;
}

/**
 * @brief  设置RC522的工作方式
 * @param  type  'A' = ISO14443_A
 * @retval MI_OK 成功
 */
char M500PcdConfigISOType(unsigned char type)
{
    if (type == 'A') // ISO14443_A
    {
        ClearBitMask(Status2Reg, 0x08);

        WriteRawRC(ModeReg, 0x3D); // 3F
        WriteRawRC(RxSelReg, 0x86); // 84
        WriteRawRC(RFCfgReg, 0x7F); // 4F
        WriteRawRC(TReloadRegL, 30);
        WriteRawRC(TReloadRegH, 0);
        WriteRawRC(TModeReg, 0x8D);
        WriteRawRC(TPrescalerReg, 0x3E);

        delay_10ms(1);
        PcdAntennaOn();
    }
    else
    {
        return (char)-1;
    }

    return MI_OK;
}

/**
 * @brief  读取RC522寄存器
 * @param  Address [IN] 寄存器地址
 * @retval 读出的值
 */
unsigned char ReadRawRC(unsigned char Address)
{
    unsigned char i, ucAddr;
    unsigned char ucResult = 0;

    NSS_L;
    ucAddr = ((Address << 1) & 0x7E) | 0x80;

    for (i = 8; i > 0; i--)
    {
        SCK_L;
        if (ucAddr & 0x80)
        {
            MOSI_H;
        }
        else
        {
            MOSI_L;
        }
        SCK_H;
        ucAddr <<= 1;
    }

    for (i = 8; i > 0; i--)
    {
        SCK_L;
        ucResult <<= 1;
        SCK_H;
        if (HAL_GPIO_ReadPin(MF522_MISO_GPIO_Port, MF522_MISO_Pin) == 1)
            ucResult |= 1;
    }

    NSS_H;
    SCK_H;
    return ucResult;
}

/**
 * @brief  写RC522寄存器
 * @param  Address [IN] 寄存器地址
 * @param  value   [IN] 写入的值
 */
void WriteRawRC(unsigned char Address, unsigned char value)
{
    unsigned char i, ucAddr;

    SCK_L;
    NSS_L;
    ucAddr = ((Address << 1) & 0x7E);

    for (i = 8; i > 0; i--)
    {
        if (ucAddr & 0x80)
        {
            MOSI_H;
        }
        else
            MOSI_L;
        SCK_H;
        ucAddr <<= 1;
        SCK_L;
    }

    for (i = 8; i > 0; i--)
    {
        if (value & 0x80)
        {
            MOSI_H;
        }
        else
        {
            MOSI_L;
        }
        SCK_H;
        value <<= 1;
        SCK_L;
    }
    NSS_H;
    SCK_H;
}

/**
 * @brief  置位RC522寄存器位
 * @param  reg  [IN] 寄存器地址
 * @param  mask [IN] 置位值
 */
void SetBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp | mask); // set bit mask
}

/**
 * @brief  清零RC522寄存器位
 * @param  reg  [IN] 寄存器地址
 * @param  mask [IN] 清零值
 */
void ClearBitMask(unsigned char reg, unsigned char mask)
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask); // clear bit mask
}

/**
 * @brief  通过RC522和ISO14443卡通讯
 * @param  Command    [IN]  RC522命令字
 * @param  pInData    [IN]  通过RC522发送到卡片的数据
 * @param  InLenByte  [IN]  发送数据的字节长度
 * @param  pOutData   [OUT] 接收到的卡片返回数据
 * @param  pOutLenBit [OUT] 返回数据的位长度
 * @retval MI_OK      成功
 */
char PcdComMF522(unsigned char Command,
                 unsigned char *pInData,
                 unsigned char InLenByte,
                 unsigned char *pOutData,
                 unsigned int *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
    case PCD_AUTHENT:
        irqEn = 0x12;
        waitFor = 0x10;
        break;
    case PCD_TRANSCEIVE:
        irqEn = 0x77;
        waitFor = 0x30;
        break;
    default:
        break;
    }

    WriteRawRC(ComIEnReg, irqEn | 0x80);
    ClearBitMask(ComIrqReg, 0x80);
    WriteRawRC(CommandReg, PCD_IDLE);
    SetBitMask(FIFOLevelReg, 0x80);

    for (i = 0; i < InLenByte; i++)
    {
        WriteRawRC(FIFODataReg, pInData[i]);
    }
    WriteRawRC(CommandReg, Command);

    if (Command == PCD_TRANSCEIVE)
    {
        SetBitMask(BitFramingReg, 0x80);
    }

    i = 2000;
    do
    {
        n = ReadRawRC(ComIrqReg);
        i--;
    } while ((i != 0) && !(n & 0x01) && !(n & waitFor));
    ClearBitMask(BitFramingReg, 0x80);

    if (i != 0)
    {
        if (!(ReadRawRC(ErrorReg) & 0x1B))
        {
            status = MI_OK;
            if (n & irqEn & 0x01)
            {
                status = MI_NOTAGERR;
            }
            if (Command == PCD_TRANSCEIVE)
            {
                n = ReadRawRC(FIFOLevelReg);
                lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {
                    *pOutLenBit = (n - 1) * 8 + lastBits;
                }
                else
                {
                    *pOutLenBit = n * 8;
                }
                if (n == 0)
                {
                    n = 1;
                }
                if (n > MAXRLEN)
                {
                    n = MAXRLEN;
                }
                for (i = 0; i < n; i++)
                {
                    pOutData[i] = ReadRawRC(FIFODataReg);
                }
            }
        }
        else
        {
            status = MI_ERR;
        }
    }

    SetBitMask(ControlReg, 0x80); // stop timer now
    WriteRawRC(CommandReg, PCD_IDLE);
    return status;
}

/**
 * @brief  开启天线
 * @note   每次开启或关闭天线之间应至少有1ms的间隔
 */
void PcdAntennaOn()
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}

/**
 * @brief  关闭天线
 */
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}

/**
 * @brief  等待卡片离开
 */
void WaitCardOff(void)
{
    char status;
    unsigned char TagType[2];

    while (1)
    {
        status = PcdRequest(REQ_ALL, TagType);
        if (status)
        {
            status = PcdRequest(REQ_ALL, TagType);
            if (status)
            {
                status = PcdRequest(REQ_ALL, TagType);
                if (status)
                {
                    return;
                }
            }
        }
        delay_10ms(10);
    }
}

// 软件延时 10ms
void delay_10ms(unsigned int _10ms)
{
    unsigned int i, j;

    for (i = 0; i < _10ms; i++)
    {
        for (j = 0; j < 60000; j++)
            ;
    }
}

///////////////////////以下为RC522驱动函数/////////////////////////

/**
 * @brief  RFID模块初始化
 */
void RFID_Init(void)
{
    PcdInit();       // RC522端口配置
    PcdReset();      // 复位RC522
    PcdAntennaOff(); // 关闭天线
    PcdAntennaOn();  // 开启天线
    M500PcdConfigISOType('A');
}

/**
 * @brief  读取卡号并匹配授权卡
 * @retval 
 * 1~4 : 已授权卡 (卡0-卡3)
 * 0   : 未授权卡
 * 5   : 防冲突失败
 * 6   : 未检测到卡
 */
uint8_t Rc522Test(void)
{
    uint8_t cardno;
    // 1. 寻卡
    if (PcdRequest(REQ_ALL, Temp) == MI_OK)
    {
        // 2. 防冲突获取UID
        if (PcdAnticoll(UID) == MI_OK)
        {
            cardno = 0;
            // 3. 比对卡号
            if (UID[0] == UI0[0] && UID[1] == UI0[1] && UID[2] == UI0[2] && UID[3] == UI0[3])
            {
                cardno = 1;
            }
            else if (UID[0] == UI1[0] && UID[1] == UI1[1] && UID[2] == UI1[2] && UID[3] == UI1[3])
            {
                cardno = 2;
            }
            else if (UID[0] == UI2[0] && UID[1] == UI2[1] && UID[2] == UI2[2] && UID[3] == UI2[3])
            {
                cardno = 3;
            }
            else if (UID[0] == UI3[0] && UID[1] == UI3[1] && UID[2] == UI3[2] && UID[3] == UI3[3])
            {
                cardno = 4;
            }
            else
                cardno = 0;
        }
        else
            cardno = 5; // 防冲突失败
    }
    else
        cardno = 6; // 寻卡失败
    return cardno;
}
