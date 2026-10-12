#include "ssd1306.h"
#include "string.h"

static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static SSD1306_t SSD1306;

static void ssd1306_WriteCommand(uint8_t command) {
    HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x00, 1, &command, 1, 10);
}

uint8_t SSD1306_Init(void) {
    HAL_Delay(100);
    ssd1306_WriteCommand(0xAE); // Display off
    ssd1306_WriteCommand(0x20); // Set Memory Addressing Mode
    ssd1306_WriteCommand(0x10); // 00,Horizontal Addressing Mode
    ssd1306_WriteCommand(0xB0); // Set Page Start Address for Page Addressing Mode
    ssd1306_WriteCommand(0xC8); // Set COM Output Scan Direction
    ssd1306_WriteCommand(0x00); // Set low column address
    ssd1306_WriteCommand(0x10); // Set high column address
    ssd1306_WriteCommand(0x40); // Set start line address
    ssd1306_WriteCommand(0x81); // Set contrast control register
    ssd1306_WriteCommand(0xFF);
    ssd1306_WriteCommand(0xA1); // Set segment re-map 0 to 127
    ssd1306_WriteCommand(0xA6); // Set normal display
    ssd1306_WriteCommand(0xA8); // Set multiplex ratio
    ssd1306_WriteCommand(0x3F); //
    ssd1306_WriteCommand(0xA4); // 0xa4,Output follows RAM content
    ssd1306_WriteCommand(0xD3); // Set display offset
    ssd1306_WriteCommand(0x00); // No offset
    ssd1306_WriteCommand(0xD5); // Set display clock divide ratio/oscillator frequency
    ssd1306_WriteCommand(0xF0); // Set divide ratio
    ssd1306_WriteCommand(0xD9); // Set pre-charge period
    ssd1306_WriteCommand(0x22); //
    ssd1306_WriteCommand(0xDA); // Set com pins hardware configuration
    ssd1306_WriteCommand(0x12);
    ssd1306_WriteCommand(0xDB); // Set vcomh
    ssd1306_WriteCommand(0x20); // 0x20,0.77xVcc
    ssd1306_WriteCommand(0x8D); // Set DC-DC enable
    ssd1306_WriteCommand(0x14); //
    ssd1306_WriteCommand(0xAF); // Turn on SSD1306 panel

    SSD1306_Fill(Black);
    SSD1306_UpdateScreen();
    SSD1306.Initialized = 1;
    return 1;
}

void SSD1306_Fill(SSD1306_COLOR color) {
    uint32_t i;
    for(i = 0; i < sizeof(SSD1306_Buffer); i++) {
        SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
    }
}

void SSD1306_Clear(void) {
    SSD1306_Fill(Black);
}

void SSD1306_UpdateScreen(void) {
    uint8_t i;
    for(i = 0; i < 8; i++) {
        ssd1306_WriteCommand(0xB0 + i);
        ssd1306_WriteCommand(0x00);
        ssd1306_WriteCommand(0x10);
        HAL_I2C_Mem_Write(&SSD1306_I2C_PORT, SSD1306_I2C_ADDR, 0x40, 1, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH, 100);
    }
}

void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color) {
    if(x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) return;
    if(color == White) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }
}

void SSD1306_GotoXY(uint16_t x, uint16_t y) {
    SSD1306.CurrentX = x;
    SSD1306.CurrentY = y;
}

char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR color) {
    uint32_t i, b, j;
    if (SSD1306_WIDTH <= (SSD1306.CurrentX + Font->FontWidth) || SSD1306_HEIGHT <= (SSD1306.CurrentY + Font->FontHeight)) return 0;

    for(i = 0; i < Font->FontHeight; i++) {
        b = Font->data[(ch - 32) * Font->FontHeight + i];
        for(j = 0; j < Font->FontWidth; j++) {
            if((b << j) & 0x8000) SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR) color);
            else SSD1306_DrawPixel(SSD1306.CurrentX + j, (SSD1306.CurrentY + i), (SSD1306_COLOR)!color);
        }
    }
    SSD1306.CurrentX += Font->FontWidth;
    return ch;
}

char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR color) {
    while (*str) {
        if (SSD1306_Putc(*str, Font, color) != *str) return *str;
        str++;
    }
    return *str;
}
