/**
 ****************************************************************************************************
 * @file        usart.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2021-10-14
 * @brief       串口初始化代码(一般是串口1)，支持printf
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F407开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20211014
 * 第一次发布
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/ATK_MW8266D/atk_mw8266d.h"
#include "./BSP/ATK_MO1218/atk_mo1218.h"
#include "./BSP/ATK_MW1278D/atk_mw1278d.h"

/******************************************************************************************/
/* 加入以下代码, 支持printf函数, 而不需要选择use MicroLIB */

#if 1
#if (__ARMCC_VERSION >= 6010050)                    /* 使用AC6编译器时 */
__asm(".global __use_no_semihosting\n\t");          /* 声明不使用半主机模式 */
__asm(".global __ARM_use_no_argv \n\t");            /* AC6下需要声明main函数为无参数格式，否则部分例程可能出现半主机模式 */

#else
/* 使用AC5编译器时, 要在这里定义__FILE 和 不使用半主机模式 */
#pragma import(__use_no_semihosting)

struct __FILE
{
    int handle;
    /* Whatever you require here. If the only file you are using is */
    /* standard output using printf() for debugging, no file handling */
    /* is required. */
};

#endif

/* 不使用半主机模式，至少需要重定义_ttywrch\_sys_exit\_sys_command_string函数,以同时兼容AC6和AC5模式 */
int _ttywrch(int ch)
{
    ch = ch;
    return ch;
}

/* 定义_sys_exit()以避免使用半主机模式 */
void _sys_exit(int x)
{
    x = x;
}

char *_sys_command_string(char *cmd, int len)
{
    return NULL;
}

/* FILE 在 stdio.h里面定义. */
FILE __stdout;

/* 重定义fputc函数, printf函数最终会通过调用fputc输出字符串到串口 */
int fputc(int ch, FILE *f)
{
    while ((UART5->SR & 0X40) == 0);               /* 等待上一个字符发送完成 */

    UART5->DR = (uint8_t)ch;                       /* 将要发送的字符 ch 写入到DR寄存器 */
    return ch;
}
#endif
/***********************************************END*******************************************/

///* 接收缓冲, 最大USART_REC_LEN个字节. */
//uint8_t g_usart_rx_buf[USART_REC_LEN];

///*  接收状态
// *  bit15，      接收完成标志
// *  bit14，      接收到0x0d
// *  bit13~0，    接收到的有效字节数目
//*/
//uint16_t g_usart_rx_sta = 0;

//uint8_t g_rx_buffer[RXBUFFERSIZE];                  /* HAL库使用的串口接收缓冲 */

//UART_HandleTypeDef g_uart1_handle;                  /* UART句柄 */


/**
 * @brief       串口X初始化函数
 * @param       baudrate: 波特率, 根据自己需要设置波特率值
 * @note        注意: 必须设置正确的时钟源, 否则串口波特率就会设置异常.
 *              这里的USART的时钟源在sys_stm32_clock_init()函数中已经设置过了.
 * @retval      无
 */
//void usart_init(uint32_t baudrate)
//{
//    g_uart1_handle.Instance = USART_UX;                         /* USART1 */
//    g_uart1_handle.Init.BaudRate = baudrate;                    /* 波特率 */
//    g_uart1_handle.Init.WordLength = UART_WORDLENGTH_8B;        /* 字长为8位数据格式 */
//    g_uart1_handle.Init.StopBits = UART_STOPBITS_1;             /* 一个停止位 */
//    g_uart1_handle.Init.Parity = UART_PARITY_NONE;              /* 无奇偶校验位 */
//    g_uart1_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;        /* 无硬件流控 */
//    g_uart1_handle.Init.Mode = UART_MODE_TX_RX;                 /* 收发模式 */
//    HAL_UART_Init(&g_uart1_handle);                             /* HAL_UART_Init()会使能UART1 */
//    
//    /* 该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量 */
//    HAL_UART_Receive_IT(&g_uart1_handle, (uint8_t *)g_rx_buffer, RXBUFFERSIZE);
//}

/**
 * @brief       UART底层初始化函数
 * @param       huart: UART句柄类型指针
 * @note        此函数会被HAL_UART_Init()调用
 *              完成时钟使能，引脚配置，中断配置
 * @retval      无
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_init_struct;
	if (huart->Instance == ATK_MW8266D_UART_INTERFACE)                 /* 如果是ATK-MW8266D UART */
    {
        ATK_MW8266D_UART_TX_GPIO_CLK_ENABLE();                              /* 使能UART TX引脚时钟 */
        ATK_MW8266D_UART_RX_GPIO_CLK_ENABLE();                              /* 使能UART RX引脚时钟 */
        ATK_MW8266D_UART_CLK_ENABLE();                                      /* 使能UART时钟 */
        
        gpio_init_struct.Pin        = ATK_MW8266D_UART_TX_GPIO_PIN;         /* UART TX引脚 */
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;                      /* 复用推挽输出 */
        gpio_init_struct.Pull       = GPIO_NOPULL;                          /* 无上下拉 */
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate  = ATK_MW8266D_UART_TX_GPIO_AF;          /* 复用为USART3 */
        HAL_GPIO_Init(ATK_MW8266D_UART_TX_GPIO_PORT, &gpio_init_struct);    /* 初始化UART TX引脚 */
        
        gpio_init_struct.Pin        = ATK_MW8266D_UART_RX_GPIO_PIN;         /* UART RX引脚 */
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;                      /* 复用推挽输出 */
        gpio_init_struct.Pull       = GPIO_NOPULL;                          /* 无上下拉 */
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate  = ATK_MW8266D_UART_RX_GPIO_AF;          /* 复用为USART3 */
        HAL_GPIO_Init(ATK_MW8266D_UART_RX_GPIO_PORT, &gpio_init_struct);    /* 初始化UART RX引脚 */
        
        HAL_NVIC_SetPriority(ATK_MW8266D_UART_IRQn, 0, 0);                  /* 抢占优先级0，子优先级0 */
        HAL_NVIC_EnableIRQ(ATK_MW8266D_UART_IRQn);                          /* 使能UART中断通道 */
        
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);                          /* 使能UART接收中断 */
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);                          /* 使能UART总线空闲中断 */
    }
	else if (huart->Instance == ATK_MO1218_UART_INTERFACE)                  /* 如果是ATK-MO1218 UART */
    {
        ATK_MO1218_UART_TX_GPIO_CLK_ENABLE();                               /* 使能UART TX引脚时钟 */
        ATK_MO1218_UART_RX_GPIO_CLK_ENABLE();                               /* 使能UART RX引脚时钟 */
        ATK_MO1218_UART_CLK_ENABLE();                                       /* 使能UART时钟 */
        
        gpio_init_struct.Pin        = ATK_MO1218_UART_TX_GPIO_PIN;          /* UART TX引脚 */
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;                      /* 复用推挽输出 */
        gpio_init_struct.Pull       = GPIO_NOPULL;                          /* 无上下拉 */
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate  = ATK_MO1218_UART_TX_GPIO_AF;           /* 复用为USART3 */
        HAL_GPIO_Init(ATK_MO1218_UART_TX_GPIO_PORT, &gpio_init_struct);     /* 初始化UART TX引脚 */
        
        gpio_init_struct.Pin        = ATK_MO1218_UART_RX_GPIO_PIN;          /* UART RX引脚 */
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;                      /* 复用推挽输出 */
        gpio_init_struct.Pull       = GPIO_NOPULL;                          /* 无上下拉 */
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate  = ATK_MO1218_UART_RX_GPIO_AF;           /* 复用为USART3 */
        HAL_GPIO_Init(ATK_MO1218_UART_RX_GPIO_PORT, &gpio_init_struct);     /* 初始化UART RX引脚 */
        
        HAL_NVIC_SetPriority(ATK_MO1218_UART_IRQn, 0, 0);                   /* 抢占优先级0，子优先级0 */
        HAL_NVIC_EnableIRQ(ATK_MO1218_UART_IRQn);                           /* 使能UART中断通道 */
        
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);                          /* 使能UART接收中断 */
        __HAL_UART_ENABLE_IT(huart, UART_IT_IDLE);                          /* 使能UART总线空闲中断 */
    }
	 else if (huart->Instance == ATK_MW1278D_UART_INTERFACE)                 /* 如果是ATK-MW1278D UART */
    {
        ATK_MW1278D_UART_TX_GPIO_CLK_ENABLE();                              /* 使能UART TX引脚时钟 */
        ATK_MW1278D_UART_RX_GPIO_CLK_ENABLE();                              /* 使能UART RX引脚时钟 */
        ATK_MW1278D_UART_CLK_ENABLE();                                      /* 使能UART时钟 */
        
        gpio_init_struct.Pin        = ATK_MW1278D_UART_TX_GPIO_PIN;         /* UART TX引脚 */
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;                      /* 复用推挽输出 */
        gpio_init_struct.Pull       = GPIO_NOPULL;                          /* 无上下拉 */
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate  = ATK_MW1278D_UART_TX_GPIO_AF;          /* 复用为USART3 */
        HAL_GPIO_Init(ATK_MW1278D_UART_TX_GPIO_PORT, &gpio_init_struct);    /* 初始化UART TX引脚 */
        
        gpio_init_struct.Pin        = ATK_MW1278D_UART_RX_GPIO_PIN;         /* UART RX引脚 */
        gpio_init_struct.Mode       = GPIO_MODE_AF_PP;                      /* 复用推挽输出 */
        gpio_init_struct.Pull       = GPIO_NOPULL;                          /* 无上下拉 */
        gpio_init_struct.Speed      = GPIO_SPEED_FREQ_HIGH;                 /* 高速 */
        gpio_init_struct.Alternate  = ATK_MW1278D_UART_RX_GPIO_AF;          /* 复用为USART3 */
        HAL_GPIO_Init(ATK_MW1278D_UART_RX_GPIO_PORT, &gpio_init_struct);    /* 初始化UART RX引脚 */
        
        HAL_NVIC_SetPriority(ATK_MW1278D_UART_IRQn, 0, 0);                  /* 抢占优先级0，子优先级0 */
        HAL_NVIC_EnableIRQ(ATK_MW1278D_UART_IRQn);                          /* 使能UART中断通道 */
        
        __HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);                          /* 使能UART接收中断 */
    }
}


 

 




