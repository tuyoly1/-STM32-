#include <string.h>
#include "main.h"
#include "AS608.h"
#include "usart.h"
#include "stdio.h"

uint32_t AS608Addr = 0xFFFFFFFF; // 指纹模块默认地址
extern uint8_t aRxBuffer[RXBUFFERSIZE_UART1]; // UART接收缓冲区
extern uint8_t RX_len; // 接收数据长度标记

// 发送单个字节数据
static void Com_SendData(uint8_t data)
{
    HAL_UART_Transmit(&huart4, &data, 1, 50);
}

// 发送包头
static void SendHead(void)
{
    Com_SendData(0xEF);
    Com_SendData(0x01);
}

// 发送模块地址
static void SendAddr(void)
{
    Com_SendData(AS608Addr >> 24);
    Com_SendData(AS608Addr >> 16);
    Com_SendData(AS608Addr >> 8);
    Com_SendData(AS608Addr);
}

// 发送标志位
static void SendFlag(uint8_t flag)
{
    Com_SendData(flag);
}

// 发送数据长度（高位在前）
static void SendLength(int length)
{
    Com_SendData(length >> 8);
    Com_SendData(length);
}

// 发送命令字
static void Sendcmd(uint8_t cmd)
{
    Com_SendData(cmd);
}

// 发送校验和（高位在前）
static void SendCheck(uint16_t check)
{
    Com_SendData(check >> 8);
    Com_SendData(check);
}

// 等待接收应答数据（超时返回NULL）
// waittime：超时时间（单位：ms）
static uint8_t *JudgeStr(uint16_t waittime)
{
    char *data;
    uint8_t str[8];
    str[0] = 0xEF;
    str[1] = 0x01;
    str[2] = AS608Addr >> 24;
    str[3] = AS608Addr >> 16;
    str[4] = AS608Addr >> 8;
    str[5] = AS608Addr;
    str[6] = 0x07;
    str[7] = '\0';

    while (--waittime)
    {
        HAL_Delay(1);
        if (RX_len) // 检测到接收完成
        {
            RX_len = 0;
            data = strstr((const char *)&aRxBuffer, (const char *)str);
            if (data)
                return (uint8_t *)data;
        }
    }
    return 0;
}

// 获取指纹图像
uint8_t GZ_GetImage(void)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x01);
    temp = 0x01 + 0x03 + 0x01;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 生成特征码
uint8_t GZ_GenChar(uint8_t BufferID)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x04);
    Sendcmd(0x02);
    Com_SendData(BufferID);
    temp = 0x01 + 0x04 + 0x02 + BufferID;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 精确比对
uint8_t GZ_Match(void)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x03);
    temp = 0x01 + 0x03 + 0x03;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 搜索指纹库
uint8_t GZ_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x08);
    Sendcmd(0x04);
    Com_SendData(BufferID);
    Com_SendData(StartPage >> 8);
    Com_SendData(StartPage);
    Com_SendData(PageNum >> 8);
    Com_SendData(PageNum);

    temp = 0x01 + 0x08 + 0x04 + BufferID + (StartPage >> 8) + (uint8_t)StartPage + (PageNum >> 8) + (uint8_t)PageNum;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
    {
        ensure = data[9];
        p->pageID = (data[10] << 8) + data[11];
        p->mathscore = (data[12] << 8) + data[13];
    }
    else
        ensure = 0xff;

    return ensure;
}

// 合并特征生成模板
uint8_t GZ_RegModel(void)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x05);
    temp = 0x01 + 0x03 + 0x05;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 存储模板
uint8_t GZ_StoreChar(uint8_t BufferID, uint16_t PageID)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x06);
    Sendcmd(0x06);
    Com_SendData(BufferID);
    Com_SendData(PageID >> 8);
    Com_SendData(PageID);

    temp = 0x01 + 0x06 + 0x06 + BufferID + (PageID >> 8) + (uint8_t)PageID;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 删除模板
uint8_t GZ_DeletChar(uint16_t PageID, uint16_t N)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x07);
    Sendcmd(0x0C);
    Com_SendData(PageID >> 8);
    Com_SendData(PageID);
    Com_SendData(N >> 8);
    Com_SendData(N);

    temp = 0x01 + 0x07 + 0x0C + (PageID >> 8) + (uint8_t)PageID + (N >> 8) + (uint8_t)N;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 清空指纹库
uint8_t GZ_Empty(void)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x0D);
    temp = 0x01 + 0x03 + 0x0D;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 写系统寄存器
uint8_t GZ_WriteReg(uint8_t RegNum, uint8_t DATA)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x05);
    Sendcmd(0x0E);
    Com_SendData(RegNum);
    Com_SendData(DATA);

    temp = RegNum + DATA + 0x01 + 0x05 + 0x0E;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 读取系统参数
uint8_t GZ_ReadSysPara(SysPara *p)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x0F);
    temp = 0x01 + 0x03 + 0x0F;
    SendCheck(temp);

    data = JudgeStr(1000);
    if (data)
    {
        ensure = data[9];
        p->GZ_max = (data[14] << 8) + data[15];
        p->GZ_level = data[17];
        p->GZ_addr = (data[18] << 24) + (data[19] << 16) + (data[20] << 8) + data[21];
        p->GZ_size = data[23];
        p->GZ_N = data[25];
    }
    else
        ensure = 0xff;
    return ensure;
}

// 设置模块地址
uint8_t GZ_SetAddr(uint32_t GZ_addr)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x07);
    Sendcmd(0x15);
    Com_SendData(GZ_addr >> 24);
    Com_SendData(GZ_addr >> 16);
    Com_SendData(GZ_addr >> 8);
    Com_SendData(GZ_addr);

    temp = 0x01 + 0x07 + 0x15 + (uint8_t)(GZ_addr >> 24) + (uint8_t)(GZ_addr >> 16) + (uint8_t)(GZ_addr >> 8) + (uint8_t)GZ_addr;
    SendCheck(temp);

    AS608Addr = GZ_addr;
    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;

    AS608Addr = GZ_addr;
    return ensure;
}

// 写入记事本
uint8_t GZ_WriteNotepad(uint8_t NotePageNum, uint8_t *Byte32)
{
    uint16_t temp = 0;   // 这里必须初始化
    uint8_t ensure, i;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(36);
    Sendcmd(0x18);
    Com_SendData(NotePageNum);

    for (i = 0; i < 32; i++)
    {
        Com_SendData(Byte32[i]);
        temp += Byte32[i];
    }

    temp = 0x01 + 36 + 0x18 + NotePageNum + temp;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
        ensure = data[9];
    else
        ensure = 0xff;
    return ensure;
}

// 读取记事本
uint8_t GZ_ReadNotepad(uint8_t NotePageNum, uint8_t *Byte32)
{
    uint16_t temp;
    uint8_t ensure, i;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x04);
    Sendcmd(0x19);
    Com_SendData(NotePageNum);

    temp = 0x01 + 0x04 + 0x19 + NotePageNum;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
    {
        ensure = data[9];
        for (i = 0; i < 32; i++)
        {
            Byte32[i] = data[10 + i];
        }
    }
    else
        ensure = 0xff;
    return ensure;
}

// 高速搜索
uint8_t GZ_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x08);
    Sendcmd(0x1B);
    Com_SendData(BufferID);
    Com_SendData(StartPage >> 8);
    Com_SendData(StartPage);
    Com_SendData(PageNum >> 8);
    Com_SendData(PageNum);

    temp = 0x01 + 0x08 + 0x1B + BufferID + (StartPage >> 8) + (uint8_t)StartPage + (PageNum >> 8) + (uint8_t)PageNum;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
    {
        ensure = data[9];
        p->pageID = (data[10] << 8) + data[11];
        printf("pageID:%d\r\n", p->pageID);
        p->mathscore = (data[12] << 8) + data[13];
        printf("mathscore:%d\r\n", p->mathscore);
    }
    else
        ensure = 0xff;

    return ensure;
}

// 获取有效模板数量
uint8_t GZ_ValidTempleteNum(uint16_t *ValidN)
{
    uint16_t temp;
    uint8_t ensure;
    uint8_t *data;

    SendHead();
    SendAddr();
    SendFlag(0x01);
    SendLength(0x03);
    Sendcmd(0x1D);
    temp = 0x01 + 0x03 + 0x1D;
    SendCheck(temp);

    data = JudgeStr(2000);
    if (data)
    {
        ensure = data[9];
        *ValidN = (data[10] << 8) + data[11];
    }
    else
        ensure = 0xff;

    return ensure;
}

// 模块握手
uint8_t GZ_HandShake(uint32_t *GZ_Addr)
{
    SendHead();
    SendAddr();
    Com_SendData(0X01);
    Com_SendData(0X00);
    Com_SendData(0X00);
    HAL_Delay(200);

    if (RX_len)
    {
        RX_len = 0;
        if (aRxBuffer[0] == 0XEF && aRxBuffer[1] == 0X01 && aRxBuffer[6] == 0X07)
        {
            *GZ_Addr = (aRxBuffer[2] << 24) + (aRxBuffer[3] << 16) + (aRxBuffer[4] << 8) + aRxBuffer[5];
            return 0;
        }
    }
    return 1;
}

// 获取操作结果描述
const char *EnsureMessage(uint8_t ensure)
{
    const char *p;
    switch (ensure)
    {
    case 0x00:
        p = "OK";
        break;
    case 0x01:
        p = "Data Error";
        break;
    case 0x02:
        p = "Checksum Failed";
        break;
    case 0x03:
        p = "Communication Timeout";
        break;
    case 0x04:
        p = "No Device Response";
        break;
    case 0x05:
        p = "Invalid Command";
        break;
    case 0x06:
        p = "Parameter Error";
        break;
    case 0x07:
        p = "Memory Insufficient";
        break;
    case 0x08:
        p = "File Not Found";
        break;
    case 0x09:
        p = "Access Denied";
        break;
    case 0x0a:
        p = "Format Error";
        break;
    case 0x0b:
        p = "Initialization Failed";
        break;
    case 0x10:
        p = "Connection Lost";
        break;
    case 0x11:
        p = "Send Failed";
        break;
    case 0x15:
        p = "Receive Data Error";
        break;
    case 0x18:
        p = "Flash Write Failed";
        break;
    case 0x19:
        p = "Erase Failed";
        break;
    case 0x1a:
        p = "Address Error";
        break;
    case 0x1b:
        p = "Length Overflow";
        break;
    case 0x1c:
        p = "Checksum Mismatch";
        break;
    case 0x1f:
        p = "Busy Status";
        break;
    case 0x20:
        p = "Idle Status";
        break;
    default:
        p = "Unknown Error";
        break;
    }
    return p;
}

