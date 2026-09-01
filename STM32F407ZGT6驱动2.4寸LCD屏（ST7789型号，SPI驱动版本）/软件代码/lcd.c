#include "lcd.h"                                          // LCD驱动头文件
#include "delay.h"                                        // 延时函数头文件
#include "font.h"                                         // 字库头文件


#define ST7789_SWRESET       0x01                         // ST7789软件复位命令
#define ST7789_SLPIN        0x10                          // ST7789进入睡眠模式
#define ST7789_SLPOUT       0x11                          // ST7789退出睡眠模式
#define ST7789_NORON        0x13                          // ST7789进入正常显示模式
#define ST7789_INVOFF       0x20                          // ST7789关闭显示反色
#define ST7789_INVON        0x21                          // ST7789开启显示反色
#define ST7789_DISPOFF      0x28                          // ST7789关闭显示
#define ST7789_DISPON       0x29                          // ST7789打开显示
#define ST7789_CASET        0x2A                          // 设置显示列地址
#define ST7789_RASET        0x2B                          // 设置显示行地址
#define ST7789_RAMWR        0x2C                          // 向GRAM写入显示数据
#define ST7789_MADCTL       0x36                          // 设置内存访问控制
#define ST7789_COLMOD       0x3A                          // 设置像素格式
#define ST7789_PORCTRL      0xB2                          // 设置显示区域前后沿参数
#define ST7789_GCTRL        0xB7                          // 设置Gate控制参数
#define ST7789_VCOMS        0xBB                          // 设置VCOM电压
#define ST7789_LCMCTRL      0xC0                          // 设置LCM控制参数
#define ST7789_VDVVRHEN     0xC2                          // 设置VDV和VRH使能
#define ST7789_VRHS         0xC3                          // 设置VRH参数
#define ST7789_VDVS         0xC4                          // 设置VDV参数
#define ST7789_FRCTRL2      0xC6                          // 设置帧率控制参数
#define ST7789_PWCTRL1      0xD0                          // 设置电源控制参数
#define ST7789_PVGAMCTRL    0xE0                          // 设置正伽马校正参数
#define ST7789_NVGAMCTRL    0xE1                          // 设置负伽马校正参数


static void LCD_SPI_WriteByte(uint8_t data)              // SPI底层发送一个字节
{
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET); // 等待SPI发送缓冲区为空
    SPI_I2S_SendData(LCD_SPI, data);                     // 向SPI2发送一个字节
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_TXE) == RESET); // 等待发送缓冲区再次为空
    while (SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) == SET);  // 等待SPI总线完成数据发送
}

void LCD_WriteCommand(uint8_t cmd)                       // 向ST7789发送命令
{
    LCD_CS_LOW();                                        // CS拉低，选中LCD
    LCD_DC_LOW();                                        // DC拉低，表示当前发送的是命令
    LCD_SPI_WriteByte(cmd);                              // 通过SPI2发送命令
    LCD_CS_HIGH();                                       // CS拉高，结束本次命令传输
}

void LCD_WriteData8(uint8_t data)                        // 向ST7789发送8位数据
{
    LCD_CS_LOW();                                        // CS拉低，选中LCD
    LCD_DC_HIGH();                                       // DC拉高，表示当前发送的是数据
    LCD_SPI_WriteByte(data);                             // 通过SPI2发送8位数据
    LCD_CS_HIGH();                                       // CS拉高，结束本次数据传输
}

void LCD_WriteData16(uint16_t data)                      // 向ST7789发送16位RGB565数据
{
    LCD_CS_LOW();                                        // CS拉低，选中LCD
    LCD_DC_HIGH();                                       // DC拉高，表示当前发送的是像素数据
    LCD_SPI_WriteByte(data >> 8);                        // 先发送RGB565高8位
    LCD_SPI_WriteByte(data & 0xFF);                      // 再发送RGB565低8位
    LCD_CS_HIGH();                                       // CS拉高，结束本次数据传输
}

static void LCD_GPIO_Init(void)                           // 初始化LCD相关GPIO
{
    GPIO_InitTypeDef GPIO_InitStructure;                  // 定义GPIO初始化结构体
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE); // 开启GPIOB和GPIOC时钟

    GPIO_InitStructure.GPIO_Pin = LCD_CS_PIN;             // 选择PC5作为LCD_CS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;         // CS配置为普通GPIO输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        // 设置为推挽输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          // 设置GPIO上拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // GPIO速度设置为50MHz
    GPIO_Init(LCD_CS_PORT, &GPIO_InitStructure);          // 初始化PC5

    GPIO_InitStructure.GPIO_Pin = LCD_DC_PIN;             // 选择PB12作为LCD_DC
    GPIO_Init(LCD_DC_PORT, &GPIO_InitStructure);          // 初始化PB12

    GPIO_InitStructure.GPIO_Pin = LCD_RST_PIN;            // 选择PB14作为LCD_RST
    GPIO_Init(LCD_RST_PORT, &GPIO_InitStructure);         // 初始化PB14

    GPIO_InitStructure.GPIO_Pin = LCD_BL_PIN;             // 选择PB3作为LCD背光控制
    GPIO_Init(LCD_BL_PORT, &GPIO_InitStructure);          // 初始化PB3

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15; // 选择PB13和PB15作为SPI2引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;          // PB13和PB15配置为复用功能
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;        // SPI输出设置为推挽
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;          // SPI引脚设置为上拉
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // GPIO速度设置为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                // 初始化PB13和PB15

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2); // PB13复用为SPI2_SCK

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2); // PB15复用为SPI2_MOSI

    LCD_CS_HIGH();                                        // LCD默认取消片选
    LCD_DC_HIGH();                                        // LCD默认设置为数据状态
    LCD_RST_HIGH();                                       // LCD默认退出复位状态
    LCD_BL_OFF();                                         // 初始化阶段先关闭背光
}

static void LCD_SPI_Init(void)                           // 初始化SPI2
{
    SPI_InitTypeDef SPI_InitStructure;                    // 定义SPI初始化结构体
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);  // 开启SPI2时钟

    SPI_I2S_DeInit(LCD_SPI);                              // 复位SPI2外设配置
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx; // 设置SPI为单线发送模式
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;         // 设置STM32为SPI主机模式
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;     // SPI数据宽度设置为8位
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;             // SPI时钟空闲状态为低电平
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;           // 在第一个时钟边沿采样数据
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;             // NSS采用软件控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; // SPI时钟进行16分频
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;    // 最高位MSB先发送
    SPI_InitStructure.SPI_CRCPolynomial = 7;              // 设置SPI CRC多项式

    SPI_Init(LCD_SPI, &SPI_InitStructure);                // 将配置写入SPI2
    SPI_Cmd(LCD_SPI, ENABLE);                             // 使能SPI2
}

static void LCD_Reset(void)                            // 对ST7789进行硬件复位
{
    LCD_RST_HIGH();                                     // RST先保持高电平
    delay_ms(10);                                       // 等待10ms
    LCD_RST_LOW();                                      // RST拉低，使ST7789进入复位状态
    delay_ms(120);                                      // 保持复位120ms
    LCD_RST_HIGH();                                     // RST拉高，结束硬件复位
    delay_ms(120);                                      // 等待LCD内部稳定
}

static void ST7789_Init(void)                           // 初始化ST7789内部寄存器
{
    LCD_WriteCommand(ST7789_SWRESET);                   // 发送软件复位命令
    delay_ms(120);                                      // 等待软件复位完成

    LCD_WriteCommand(ST7789_SLPOUT);                    // 退出ST7789睡眠模式
    delay_ms(120);                                      // 等待退出睡眠模式完成

    LCD_WriteCommand(ST7789_MADCTL);                    // 设置内存访问控制
    LCD_WriteData8(0x60);                                // 设置当前屏幕扫描方向及RGB/BGR顺序

    LCD_WriteCommand(ST7789_COLMOD);                    // 设置LCD像素格式
    LCD_WriteData8(0x55);                                // 设置为RGB565，16bit每像素
    delay_ms(10);                                       // 等待像素格式设置生效

    LCD_WriteCommand(ST7789_PORCTRL);                   // 设置显示区域前后沿参数
    LCD_WriteData8(0x0C);                               // Porch参数1
    LCD_WriteData8(0x0C);                               // Porch参数2
    LCD_WriteData8(0x00);                               // Porch参数3
    LCD_WriteData8(0x33);                               // Porch参数4
    LCD_WriteData8(0x33);                               // Porch参数5

    LCD_WriteCommand(ST7789_GCTRL);                     // 设置Gate控制参数
    LCD_WriteData8(0x35);                               // 设置Gate控制值

    LCD_WriteCommand(ST7789_VCOMS);                     // 设置VCOM电压
    LCD_WriteData8(0x32);                               // 设置VCOM参数

    LCD_WriteCommand(ST7789_LCMCTRL);                   // 设置LCM控制参数
    LCD_WriteData8(0x2C);                               // 设置LCM控制值

    LCD_WriteCommand(ST7789_VDVVRHEN);                  // 开启VDV和VRH控制
    LCD_WriteData8(0x01);                               // 设置VDVVRH使能参数

    LCD_WriteCommand(ST7789_VRHS);                      // 设置VRH参数
    LCD_WriteData8(0x0F);                               // 设置VRH电压参数

    LCD_WriteCommand(ST7789_VDVS);                      // 设置VDV参数
    LCD_WriteData8(0x20);                               // 设置VDV参数值

    LCD_WriteCommand(ST7789_FRCTRL2);                   // 设置帧率控制
    LCD_WriteData8(0x0F);                               // 设置帧率参数

    LCD_WriteCommand(ST7789_PWCTRL1);                   // 设置电源控制
    LCD_WriteData8(0xA4);                               // 电源参数1
    LCD_WriteData8(0xA1);                               // 电源参数2

    LCD_WriteCommand(ST7789_PVGAMCTRL);                 // 设置正伽马校正
    LCD_WriteData8(0xD0);                               // 正伽马参数1
    LCD_WriteData8(0x00);                               // 正伽马参数2
    LCD_WriteData8(0x02);                               // 正伽马参数3
    LCD_WriteData8(0x07);                               // 正伽马参数4
    LCD_WriteData8(0x0A);                               // 正伽马参数5
    LCD_WriteData8(0x28);                               // 正伽马参数6
    LCD_WriteData8(0x32);                               // 正伽马参数7
    LCD_WriteData8(0x44);                               // 正伽马参数8
    LCD_WriteData8(0x42);                               // 正伽马参数9
    LCD_WriteData8(0x06);                               // 正伽马参数10
    LCD_WriteData8(0x0E);                               // 正伽马参数11
    LCD_WriteData8(0x12);                               // 正伽马参数12
    LCD_WriteData8(0x14);                               // 正伽马参数13
    LCD_WriteData8(0x17);                               // 正伽马参数14

    LCD_WriteCommand(ST7789_NVGAMCTRL);                 // 设置负伽马校正
    LCD_WriteData8(0xD0);                               // 负伽马参数1
    LCD_WriteData8(0x00);                               // 负伽马参数2
    LCD_WriteData8(0x02);                               // 负伽马参数3
    LCD_WriteData8(0x07);                               // 负伽马参数4
    LCD_WriteData8(0x0A);                               // 负伽马参数5
    LCD_WriteData8(0x28);                               // 负伽马参数6
    LCD_WriteData8(0x31);                               // 负伽马参数7
    LCD_WriteData8(0x54);                               // 负伽马参数8
    LCD_WriteData8(0x47);                               // 负伽马参数9
    LCD_WriteData8(0x0E);                               // 负伽马参数10
    LCD_WriteData8(0x1C);                               // 负伽马参数11
    LCD_WriteData8(0x17);                               // 负伽马参数12
    LCD_WriteData8(0x1B);                               // 负伽马参数13
    LCD_WriteData8(0x1E);                               // 负伽马参数14

    LCD_WriteCommand(ST7789_NORON);                     // 进入正常显示模式
    delay_ms(10);                                       // 等待正常显示模式生效

    LCD_WriteCommand(ST7789_INVOFF);                    // 关闭LCD显示反色
    delay_ms(10);                                       // 等待设置生效

    LCD_WriteCommand(ST7789_DISPON);                    // 打开ST7789显示输出
    delay_ms(120);                                      // 等待显示稳定
}

void LCD_Init(void)                                      // LCD总初始化函数
{
    LCD_GPIO_Init();                                     // 初始化LCD相关GPIO
    LCD_SPI_Init();                                      // 初始化SPI2
    LCD_Reset();                                         // 对ST7789进行硬件复位
    ST7789_Init();                                       // 初始化ST7789寄存器
    LCD_BL_ON();                                         // 打开LCD背光
    LCD_Clear(BLACK);                                    // 初始化完成后清屏为黑色
}

void LCD_DisplayOn(void)                                 // 打开LCD显示
{
    LCD_WriteCommand(ST7789_DISPON);                     // 发送显示开启命令
    delay_ms(10);                                       // 等待显示稳定
    LCD_BL_ON();                                         // 打开LCD背光
}

void LCD_DisplayOff(void)                                // 关闭LCD显示
{
    LCD_WriteCommand(ST7789_DISPOFF);                   // 发送显示关闭命令
    LCD_BL_OFF();                                        // 关闭LCD背光
}

void LCD_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)// 设置LCD显示窗口，设置窗口起始Y坐标，设置窗口结束X坐标，设置窗口结束Y坐标                                            
{
    LCD_WriteCommand(ST7789_CASET);                      // 发送列地址设置命令

    LCD_WriteData8(x0 >> 8);                             // 发送X起始地址高8位
    LCD_WriteData8(x0 & 0xFF);                           // 发送X起始地址低8位
    LCD_WriteData8(x1 >> 8);                             // 发送X结束地址高8位
    LCD_WriteData8(x1 & 0xFF);                           // 发送X结束地址低8位

    LCD_WriteCommand(ST7789_RASET);                      // 发送行地址设置命令

    LCD_WriteData8(y0 >> 8);                             // 发送Y起始地址高8位
    LCD_WriteData8(y0 & 0xFF);                           // 发送Y起始地址低8位
    LCD_WriteData8(y1 >> 8);                             // 发送Y结束地址高8位
    LCD_WriteData8(y1 & 0xFF);                           // 发送Y结束地址低8位

    LCD_WriteCommand(ST7789_RAMWR);                      // 发送GRAM写入命令，准备接收像素数据
}

void LCD_SetCursor(uint16_t x, uint16_t y)               // 设置LCD当前光标位置，设置LCD当前光标位置                                        
{
    LCD_SetAddressWindow(x, y, x, y);                    // 将显示窗口设置为当前一个像素点
}

void LCD_Clear(uint16_t color)                           // 使用指定颜色清空整个LCD
{
    uint32_t i;                                          // 定义循环变量
    uint32_t total;                                      // 定义需要发送的像素总数量

    total = LCD_WIDTH * LCD_HEIGHT;                      // 计算整个屏幕的像素数量

    LCD_SetAddressWindow(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);// 设置显示窗口左上角X为0，设置显示窗口左上角Y为0，设置显示窗口右下角X，设置显示窗口右下角Y

    LCD_CS_LOW();                                        // CS拉低，选中LCD
    LCD_DC_HIGH();                                       // DC拉高，进入数据模式

    for(i = 0; i < total; i++)                           // 循环发送整个屏幕的像素
    {
        LCD_SPI_WriteByte(color >> 8);                   // 发送RGB565高8位
        LCD_SPI_WriteByte(color & 0xFF);                 // 发送RGB565低8位
    }

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) == SET); // 等待SPI完成全部数据发送

    LCD_CS_HIGH();                                       // CS拉高，结束本次屏幕数据传输
}

void LCD_DrawPoint(uint16_t x, uint16_t y, uint16_t color)// 绘制像素点X坐标，绘制像素点Y坐标，绘制像素点颜色                                         
{
    if(x >= LCD_WIDTH)                                  // 判断X坐标是否超过LCD宽度
        return;                                         // 超出范围直接退出

    if(y >= LCD_HEIGHT)                                 // 判断Y坐标是否超过LCD高度
        return;                                         // 超出范围直接退出

    LCD_SetAddressWindow(x, y, x, y);                    // 将LCD显示窗口设置为当前像素点
    LCD_WriteData16(color);                              // 向当前像素点写入RGB565颜色
}

void LCD_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)// 填充区域起始X坐标，填充区域起始Y坐标，填充区域结束X坐标，填充区域结束Y坐标，填充区域颜色                                       
{
    uint32_t i;                                          // 定义循环变量
    uint32_t total;                                      // 定义填充区域像素总数量

    if(x0 >= LCD_WIDTH)                                  // 判断起始X是否超出屏幕
        return;                                         // 超出范围直接退出

    if(y0 >= LCD_HEIGHT)                                 // 判断起始Y是否超出屏幕
        return;                                         // 超出范围直接退出

    if(x1 >= LCD_WIDTH)                                  // 判断结束X是否超过屏幕
        x1 = LCD_WIDTH - 1;                              // 超出时限制到最后一列

    if(y1 >= LCD_HEIGHT)                                 // 判断结束Y是否超过屏幕
        y1 = LCD_HEIGHT - 1;                             // 超出时限制到最后一行

    if(x1 < x0)                                          // 判断X方向坐标是否合法
        return;                                         // 无效区域直接退出

    if(y1 < y0)                                          // 判断Y方向坐标是否合法
        return;                                         // 无效区域直接退出

    LCD_SetAddressWindow(x0, y0, x1, y1);                // 设置矩形显示窗口

    total = (uint32_t)(x1 - x0 + 1) *                    // 计算矩形区域X方向像素数量
            (uint32_t)(y1 - y0 + 1);                    // 计算矩形区域Y方向像素数量

    LCD_CS_LOW();                                        // CS拉低，选中LCD
    LCD_DC_HIGH();                                       // DC拉高，进入数据模式

    for(i = 0; i < total; i++)                           // 连续发送矩形区域所有像素
    {
        LCD_SPI_WriteByte(color >> 8);                   // 发送RGB565高8位
        LCD_SPI_WriteByte(color & 0xFF);                 // 发送RGB565低8位
    }

    while(SPI_I2S_GetFlagStatus(LCD_SPI, SPI_I2S_FLAG_BSY) == SET); // 等待SPI完成所有数据发送

    LCD_CS_HIGH();                                       // CS拉高，结束矩形填充
}

void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)// 直线起点X坐标，直线起点Y坐标，直线终点X坐标，直线终点Y坐标，直线颜色                                       
{
    int dx;                                              // X方向距离
    int dy;                                              // Y方向距离
    int sx;                                              // X方向移动步长
    int sy;                                              // Y方向移动步长
    int err;                                             // Bresenham算法误差值
    int e2;                                              // Bresenham临时误差值

    dx = (x1 > x0) ? (x1 - x0) : (x0 - x1);             // 计算X方向绝对距离
    dy = (y1 > y0) ? (y1 - y0) : (y0 - y1);             // 计算Y方向绝对距离
    sx = (x0 < x1) ? 1 : -1;                             // 判断X方向移动方向
    sy = (y0 < y1) ? 1 : -1;                             // 判断Y方向移动方向
    err = dx - dy;                                       // 初始化Bresenham误差值

    while(1)                                             // 循环绘制直线
    {
        LCD_DrawPoint(x0, y0, color);                    // 绘制当前像素点

        if(x0 == x1 && y0 == y1)                         // 判断是否已经到达终点
            break;                                      // 到达终点后退出循环

        e2 = 2 * err;                                    // 计算两倍误差值

        if(e2 > -dy)                                     // 判断X方向是否需要移动
        {
            err -= dy;                                   // 更新误差值
            x0 += sx;                                    // X坐标向目标方向移动
        }

        if(e2 < dx)                                      // 判断Y方向是否需要移动
        {
            err += dx;                                   // 更新误差值
            y0 += sy;                                    // Y坐标向目标方向移动
        }
    }
}

void LCD_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)// 矩形左上角X坐标，矩形左上角Y坐标，矩形右下角X坐标，矩形右下角Y坐标，矩形边框颜色                                      
{
    LCD_DrawLine(x0, y0, x1, y0, color);                 // 绘制矩形上边
    LCD_DrawLine(x0, y0, x0, y1, color);                 // 绘制矩形左边
    LCD_DrawLine(x0, y1, x1, y1, color);                 // 绘制矩形下边
    LCD_DrawLine(x1, y0, x1, y1, color);                 // 绘制矩形右边
}

void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t ch, uint8_t size, uint16_t color, uint16_t bgcolor)// 字符起始X坐标，字符起始Y坐标，要显示的ASCII字符，字体大小，字符前景色，字符背景色                                    
{
    uint8_t temp;                                       // 保存当前字模数据
    uint8_t t;                                          // 字模字节循环变量
    uint8_t t1;                                         // 字模位循环变量
    uint16_t y0;                                        // 保存字符初始Y坐标
    uint16_t csize;                                     // 当前字符字模占用的字节数量

    if(ch < ' ' || ch > '~')                            // 判断是否为可显示ASCII字符
        return;                                         // 非可显示字符直接退出

    y0 = y;                                             // 保存字符起始Y坐标

    csize = (size / 8 + ((size % 8) ? 1 : 0)) *         // 根据字体高度计算字模字节数量
            (size / 2);                                 // 根据字体宽度计算字模字节数量

    ch -= ' ';                                          // 将ASCII字符转换为字库数组下标

    for(t = 0; t < csize; t++)                          // 逐字节读取当前字符字模
    {
        if(size == 12)                                  // 判断是否使用12号字体
        {
            temp = asc2_1206[ch][t];                    // 读取12号字体字模
        }
        else if(size == 16)                             // 判断是否使用16号字体
        {
            temp = asc2_1608[ch][t];                    // 读取16号字体字模
        }
        else if(size == 24)                             // 判断是否使用24号字体
        {
            temp = asc2_2412[ch][t];                    // 读取24号字体字模
        }
        else                                            // 其他字体大小不支持
        {
            return;                                     // 不支持时退出函数
        }

        for(t1 = 0; t1 < 8; t1++)                       // 逐位处理当前字模数据
        {
            if(temp & 0x80)                             // 判断当前字模最高位是否为1
            {
                LCD_DrawPoint(x, y, color);             // 当前像素显示前景色
            }
            else                                        // 当前字模位为0
            {
                LCD_DrawPoint(x, y, bgcolor);           // 当前像素显示背景色
            }

            temp <<= 1;                                 // 字模左移一位，准备处理下一位
            y++;                                         // Y坐标向下移动一个像素

            if((y - y0) >= size)                        // 判断当前字模列是否已经达到字体高度
            {
                y = y0;                                 // Y坐标恢复到字符起始位置
                x++;                                    // X坐标向右移动一列
                break;                                  // 当前字模列处理完成
            }
        }
    }
}

void LCD_ShowString(uint16_t x, uint16_t y, uint8_t size, uint16_t color, uint16_t bgcolor, const char *str)// 字符串起始X坐标，字符串起始Y坐标，字体大小，字符串颜色，字符串背景颜色，要显示的字符串                                   
{
    while(*str)                                       // 判断字符串是否到达结束符
    {
        LCD_ShowChar(x, y, *str, size, color, bgcolor); // 显示当前ASCII字符

        x += size / 2;                                // X坐标移动到下一个字符位置

        if(x >= LCD_WIDTH)                            // 判断是否超过LCD横向显示范围
        {
            x = 0;                                    // X坐标回到最左侧
            y += size;                                // Y坐标向下移动一个字符高度
        }

        if(y >= LCD_HEIGHT)                           // 判断是否超过LCD纵向显示范围
            break;                                    // 超出屏幕后停止显示

        str++;                                        // 指向字符串中的下一个字符
    }
}

void LCD_ShowNum(uint16_t x, uint16_t y, uint32_t num, uint8_t len, uint8_t size, uint16_t color)// 数字起始X坐标，数字起始Y坐标，要显示的数字，数字显示位数，字体大小，数字颜色                                    
{
    uint8_t i;                                        // 数字循环变量
    uint32_t div;                                     // 当前数字位对应的除数

    div = 1;                                          // 初始化除数为1

    for(i = 1; i < len; i++)                          // 根据显示位数计算最高位除数
    {
        div *= 10;                                    // 每增加一位，除数乘10
    }

    for(i = 0; i < len; i++)                          // 逐位显示数字
    {
        uint8_t digit;                                // 保存当前数字位

        digit = (num / div) % 10;                     // 提取当前数字位

        LCD_ShowChar(x + i * size / 2,                // 计算当前数字显示X坐标
                     y,                               // 使用当前Y坐标
                     digit + '0',                     // 将数字转换为ASCII字符
                     size,                            // 设置字体大小
                     color,                           // 设置数字颜色
                     BLACK);                          // 数字背景设置为黑色

        div /= 10;                                    // 除数缩小10倍，准备处理下一位
    }
}

void LCD_BackLight(uint8_t on)                        // 控制LCD背光
{
    if(on)                                             // 判断是否需要打开背光
    {
        LCD_BL_ON();                                   // PB3输出高电平，打开背光
    }
    else                                               // 判断为关闭背光
    {
        LCD_BL_OFF();                                  // PB3输出低电平，关闭背光
    }
}
