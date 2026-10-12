#ifndef ssd1306_H
#define ssd1306_H

#include "stm32f4xx_hal.h"
#include "fonts.h"

// I2C Configuration
#define SSD1306_I2C_PORT		hi2c1
#define SSD1306_I2C_ADDR        0x78 // Try 0x7A if 0x78 shows static
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

extern I2C_HandleTypeDef SSD1306_I2C_PORT;

typedef enum {
    Black = 0x00,
    White = 0x01
} SSD1306_COLOR;

typedef struct {
    uint16_t CurrentX;
    uint16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} SSD1306_t;

uint8_t SSD1306_Init(void);
void SSD1306_UpdateScreen(void);
void SSD1306_Fill(SSD1306_COLOR color);
void SSD1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color);
void SSD1306_GotoXY(uint16_t x, uint16_t y);
char SSD1306_Putc(char ch, FontDef_t* Font, SSD1306_COLOR color);
char SSD1306_Puts(char* str, FontDef_t* Font, SSD1306_COLOR color);
void SSD1306_Clear(void);

#endif
