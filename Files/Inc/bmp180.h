#ifndef BMP180_H
#define BMP180_H

#include "stm32f4xx_hal.h"

// Ensure this matches the I2C port you are using (I2C1)
extern I2C_HandleTypeDef hi2c1;
#define BMP180_I2C &hi2c1

// BMP180 standard I2C address
#define BMP180_ADDRESS 0xEE

// Function Prototypes
void BMP180_Init(void);
float BMP180_GetTemp(void);
float BMP180_GetPress(int oss);

#endif
