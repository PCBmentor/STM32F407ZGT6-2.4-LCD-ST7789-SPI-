#ifndef __LCD_H
#define __LCD_H

#include "sys.h"                                      // 系统相关头文件
#include "stm32f4xx_gpio.h"                           // GPIO驱动头文件
#include "stm32f4xx_rcc.h"                            // RCC时钟驱动头文件
#include "stm32f4xx_spi.h"                            // SPI驱动头文件

#define LCD_WIDTH       320                           // LCD横向分辨率为320
#define LCD_HEIGHT      240                           // LCD纵向分辨率为240

#define LCD_SPI                 SPI2                  // ST7789使用SPI2
#define LCD_SPI_CLK             RCC_APB1Periph_SPI2   // SPI2挂载在APB1总线上

#define LCD_CS_PIN              GPIO_Pin_5             // LCD_CS连接PC5
#define LCD_CS_PORT             GPIOC                 // CS所在GPIO端口为GPIOC
#define LCD_CS_CLK              RCC_AHB1Periph_GPIOC  // GPIOC时钟

#define LCD_DC_PIN              GPIO_Pin_12            // LCD_DC连接PB12
#define LCD_DC_PORT             GPIOB                 // DC所在GPIO端口为GPIOB
#define LCD_DC_CLK              RCC_AHB1Periph_GPIOB  // GPIOB时钟

#define LCD_RST_PIN             GPIO_Pin_14            // LCD_RST连接PB14
#define LCD_RST_PORT            GPIOB                 // RST所在GPIO端口为GPIOB
#define LCD_RST_CLK             RCC_AHB1Periph_GPIOB  // GPIOB时钟

#define LCD_BL_PIN              GPIO_Pin_3             // LCD背光连接PB3
#define LCD_BL_PORT             GPIOB                 // 背光所在GPIO端口为GPIOB
#define LCD_BL_CLK              RCC_AHB1Periph_GPIOB  // GPIOB时钟

#define LCD_CS_LOW()        GPIO_ResetBits(LCD_CS_PORT, LCD_CS_PIN)   // CS输出低电平，选中LCD
#define LCD_CS_HIGH()       GPIO_SetBits(LCD_CS_PORT, LCD_CS_PIN)     // CS输出高电平，取消选中LCD

#define LCD_DC_LOW()        GPIO_ResetBits(LCD_DC_PORT, LCD_DC_PIN)   // DC输出低电平，表示发送命令
#define LCD_DC_HIGH()       GPIO_SetBits(LCD_DC_PORT, LCD_DC_PIN)     // DC输出高电平，表示发送数据

#define LCD_RST_LOW()       GPIO_ResetBits(LCD_RST_PORT, LCD_RST_PIN) // RST输出低电平，使LCD复位
#define LCD_RST_HIGH()      GPIO_SetBits(LCD_RST_PORT, LCD_RST_PIN)   // RST输出高电平，结束LCD复位

#define LCD_BL_ON()         GPIO_SetBits(LCD_BL_PORT, LCD_BL_PIN)     // 背光输出高电平，打开背光
#define LCD_BL_OFF()        GPIO_ResetBits(LCD_BL_PORT, LCD_BL_PIN)   // 背光输出低电平，关闭背光

#define WHITE       0xFFFF                                // RGB565白色
#define BLACK       0x0000                                // RGB565黑色
#define BLUE        0x001F                                // RGB565蓝色
#define RED         0xF800                                // RGB565红色
#define GREEN       0x07E0                                // RGB565绿色
#define CYAN        0x07FF                                // RGB565青色
#define MAGENTA     0xF81F                                // RGB565洋红色
#define YELLOW      0xFFE0                                // RGB565黄色
#define GRAY        0x8410                                // RGB565灰色
#define ORANGE      0xFD20                                // RGB565橙色
#define PINK        0xF81F                                // RGB565粉色

void LCD_Init(void);                                      // 初始化LCD
void LCD_WriteCommand(uint8_t cmd);                       // 向ST7789发送一个8位命令
void LCD_WriteData8(uint8_t data);                        // 向ST7789发送一个8位数据
void LCD_WriteData16(uint16_t data);                      // 向ST7789发送一个16位RGB565数据
void LCD_DisplayOn(void);                                 // 打开LCD显示
void LCD_DisplayOff(void);                                // 关闭LCD显示
void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);// 设置LCD显示窗口，设置窗口起始Y坐标，设置窗口结束X坐标，设置窗口结束Y坐标
void LCD_SetCursor(uint16_t x, uint16_t y);               // 设置LCD当前光标X坐标，设置LCD当前光标Y坐标
void LCD_Clear(uint16_t color);                           // 使用指定颜色清空整个LCD
void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color);// 设置像素点X坐标，设置像素点Y坐标，设置像素点颜色
void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);// 填充区域起始X坐标，填充区域起始Y坐标，填充区域结束X坐标，填充区域结束Y坐标，填充区域颜色
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);// 直线起点X坐标，直线起点Y坐标，直线终点X坐标，直线终点Y坐标，直线颜色
void LCD_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);// 矩形左上角X坐标，矩形左上角Y坐标，矩形右下角X坐标，矩形右下角Y坐标，矩形边框颜色
void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t ch, uint8_t size, uint16_t color, uint16_t bgcolor);// 字符起始X坐标，字符起始Y坐标，要显示的ASCII字符，字体大小，字符前景色，字符背景色
void LCD_ShowString(uint16_t x, uint16_t y, uint8_t size, uint16_t color, uint16_t bgcolor, const char *str);// 字符串起始X坐标，字符串起始Y坐标，字体大小，字符串颜色，字符串背景颜色，要显示的字符串
void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color);// 数字起始X坐标，数字起始Y坐标，要显示的数字，数字显示位数，字体大小，数字颜色
void LCD_BackLight(uint8_t on);                           // 控制LCD背光开关

#endif
