#ifndef __AS608_H
#define __AS608_H
#include "main.h"

#define GZ_Sta   PAin(6)  // 指纹模块状态引脚 PA6（输入检测）
#define CharBuffer1 0x01   // 字符缓冲区1标识
#define CharBuffer2 0x02   // 字符缓冲区2标识

extern uint32_t AS608Addr;  // 指纹模块地址

// 搜索结果结构体
typedef struct  
{
    uint16_t pageID;     // 匹配的页ID
    uint16_t mathscore;  // 匹配得分（置信度）
} SearchResult;

// 系统参数结构体
typedef struct
{
    uint16_t GZ_max;      // 指纹最大容量
    uint8_t  GZ_level;    // 安全等级（0-5级）
    uint32_t GZ_addr;     // 模块地址
    uint8_t  GZ_size;     // 通讯数据包大小
    uint8_t  GZ_N;        // 波特率设置（N值）
} SysPara;

// 函数声明
void GZ_StaGPIO_Init(void);          // 初始化状态检测GPIO（PA6）

uint8_t GZ_GetImage(void);           // 获取指纹图像
uint8_t GZ_GenChar(uint8_t BufferID); // 生成特征码到指定缓冲区
uint8_t GZ_Match(void);              // 精确比对两个缓冲区特征
uint8_t GZ_Search(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p); // 搜索指纹库
uint8_t GZ_RegModel(void);           // 合并特征生成模板
uint8_t GZ_StoreChar(uint8_t BufferID, uint16_t PageID); // 存储模板到指定页
uint8_t GZ_DeletChar(uint16_t PageID, uint16_t N);       // 删除指定范围的模板
uint8_t GZ_Empty(void);              // 清空指纹库
uint8_t GZ_WriteReg(uint8_t RegNum, uint8_t DATA);      // 写系统寄存器
uint8_t GZ_ReadSysPara(SysPara *p);  // 读取系统参数
uint8_t GZ_SetAddr(uint32_t addr);   // 设置模块地址
uint8_t GZ_WriteNotepad(uint8_t NotePageNum, uint8_t *content); // 写入记事本
uint8_t GZ_ReadNotepad(uint8_t NotePageNum, uint8_t *note);    // 读取记事本
uint8_t GZ_HighSpeedSearch(uint8_t BufferID, uint16_t StartPage, uint16_t PageNum, SearchResult *p); // 高速搜索
uint8_t GZ_ValidTempleteNum(uint16_t *ValidN);         // 获取有效模板数量
uint8_t GZ_HandShake(uint32_t *GZ_Addr);              // 与AS608模块握手

const char *EnsureMessage(uint8_t ensure);            // 获取操作结果描述

#endif
