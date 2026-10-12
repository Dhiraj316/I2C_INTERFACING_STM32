#include "bmp180.h"

// Variables to store the sensor's factory calibration data
short AC1, AC2, AC3, B1, B2, MB, MC, MD;
unsigned short AC4, AC5, AC6;
long B5;

void BMP180_ReadCalibrationData(void) {
    uint8_t calibData[22];

    // Read all 22 calibration registers starting from 0xAA
    HAL_I2C_Mem_Read(BMP180_I2C, BMP180_ADDRESS, 0xAA, 1, calibData, 22, HAL_MAX_DELAY);

    // Merge the 8-bit registers into 16-bit variables
    AC1 = (calibData[0] << 8)  | calibData[1];
    AC2 = (calibData[2] << 8)  | calibData[3];
    AC3 = (calibData[4] << 8)  | calibData[5];
    AC4 = (calibData[6] << 8)  | calibData[7];
    AC5 = (calibData[8] << 8)  | calibData[9];
    AC6 = (calibData[10] << 8) | calibData[11];
    B1  = (calibData[12] << 8) | calibData[13];
    B2  = (calibData[14] << 8) | calibData[15];
    MB  = (calibData[16] << 8) | calibData[17];
    MC  = (calibData[18] << 8) | calibData[19];
    MD  = (calibData[20] << 8) | calibData[21];
}

void BMP180_Init(void) {
    // We must read the calibration data once on startup
    BMP180_ReadCalibrationData();
}

float BMP180_GetTemp(void) {
    uint8_t dat = 0x2E;
    // Request temperature measurement
    HAL_I2C_Mem_Write(BMP180_I2C, BMP180_ADDRESS, 0xF4, 1, &dat, 1, 100);
    HAL_Delay(5); // Wait for measurement to complete

    uint8_t raw[2];
    HAL_I2C_Mem_Read(BMP180_I2C, BMP180_ADDRESS, 0xF6, 1, raw, 2, 100);

    long UT = (raw[0] << 8) | raw[1];

    // Standard Bosch Temperature Compensation Algorithm
    long X1 = ((UT - (long)AC6) * (long)AC5) >> 15;
    long X2 = ((long)MC << 11) / (X1 + (long)MD);
    B5 = X1 + X2;
    float temp = ((B5 + 8) >> 4) / 10.0;

    return temp;
}

float BMP180_GetPress(int oss) {
    uint8_t dat = 0x34 + (oss << 6);
    // Request pressure measurement
    HAL_I2C_Mem_Write(BMP180_I2C, BMP180_ADDRESS, 0xF4, 1, &dat, 1, 100);
    HAL_Delay(5 + (oss * 3)); // Wait time depends on accuracy setting

    uint8_t raw[3];
    HAL_I2C_Mem_Read(BMP180_I2C, BMP180_ADDRESS, 0xF6, 1, raw, 3, 100);

    long UP = ((raw[0] << 16) | (raw[1] << 8) | raw[2]) >> (8 - oss);

    // Standard Bosch Pressure Compensation Algorithm
    long B6, X1, X2, X3, B3, p;
    unsigned long B4, B7;

    B6 = B5 - 4000;
    X1 = ((long)B2 * ((B6 * B6) >> 12)) >> 11;
    X2 = ((long)AC2 * B6) >> 11;
    X3 = X1 + X2;
    B3 = ((((long)AC1 * 4 + X3) << oss) + 2) / 4;

    X1 = ((long)AC3 * B6) >> 13;
    X2 = ((long)B1 * ((B6 * B6) >> 12)) >> 16;
    X3 = ((X1 + X2) + 2) >> 2;
    B4 = ((unsigned long)AC4 * (unsigned long)(X3 + 32768)) >> 15;
    B7 = ((unsigned long)UP - B3) * (unsigned long)(50000UL >> oss);

    if (B7 < 0x80000000) p = (B7 * 2) / B4;
    else p = (B7 / B4) * 2;

    X1 = (p >> 8) * (p >> 8);
    X1 = (X1 * 3038) >> 16;
    X2 = (-7357 * p) >> 16;
    p = p + ((X1 + X2 + (long)3791) >> 4);

    return (float)p;
}
