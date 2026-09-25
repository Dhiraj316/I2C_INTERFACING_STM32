# ADC_INTERFACING_I2C_ANALOG_STM32

This repository contains a complete multi-device I2C application developed for the STM32F407VETx microcontroller. The project demonstrates how to configure a single I2C bus to simultaneously interface with a BMP180 Temperature/Pressure sensor, an SSD1306 OLED display, and a standard I2C LCD character display using the STM32 HAL framework.

## Hardware & Programmer
* **Board:** STM32F407 Development Board.
* **Peripherals:** 
  * BMP180 (Temperature & Pressure Sensor)
  * SSD1306 OLED Display (I2C version)
  * 16x2 or 20x4 LCD with I2C Backpack
* **Programmer:** ST-LINK V2
* **Wiring Setup (Shared I2C Bus):** 
  * All `GND` pins -> STM32 `GND`
  * All `VCC` pins -> STM32 `3.3V` or `5V` (Check individual module tolerances; LCDs often require 5V, while BMP180 requires 3.3V).
  * All `SCL` pins -> STM32 `PB6` (I2C1_SCL)
  * All `SDA` pins -> STM32 `PB7` (I2C1_SDA)

### 1. Project Creation
### 2. IOC Pin & I2C Configuration
1. Open the `.ioc` Device Configuration Tool.
2. Go to **System Core > SYS** and set **Debug** to **Serial Wire**.
3. Go to **Connectivity > I2C1**.
4. Set **I2C** to **I2C**.
5. In the **Parameter Settings** for I2C1, configure:
   * **I2C Speed Mode:** Standard Mode
   * **I2C Clock Speed:** `100000` Hz (100 kHz)
6. Ensure `PB6` and `PB7` are configured for I2C1.
7. Save (`Ctrl+S`) and click **Yes** to generate the initialization code[cite: 4].

### 3. Application Code Integration
Copy the provided external library files (`ssd1306.c/h`, `fonts.c/h`, `bmp180.c/h`, `i2c-lcd.c/h`) into your `Core/Src` and `Core/Inc` folders[cite: 4]. 
/* USER CODE BEGIN WHILE */
  while (1)
  {
      // 1. Read the sensor data
      float temp = BMP180_GetTemp();
      float pressure = BMP180_GetPress(0);
      // 2. The Float-to-Integer Trick
      int temp_whole = (int)temp;
      int temp_decimal = (int)((temp - temp_whole) * 10);
      int press_whole = (int)(pressure / 100);
      char tempStr[30];
      char pressStr[30];
      sprintf(tempStr, "Temp: %d.%d C   ", temp_whole, temp_decimal);
      sprintf(pressStr, "Pres: %d hPa   ", press_whole);
      // 4. Update the OLED Display
      SSD1306_Clear();
      SSD1306_GotoXY(5, 10);
      SSD1306_Puts(tempStr, &Font_11x18, 1);
      SSD1306_GotoXY(5, 35);
      SSD1306_Puts(pressStr, &Font_11x18, 1);
      SSD1306_UpdateScreen();
      // 5. Update the LCD Display
      lcd_put_cur(0, 0);
      lcd_send_string(tempStr);
      lcd_put_cur(1, 0);
      lcd_send_string(pressStr);
      HAL_Delay(1000);


  ### Build and Flash
Click the Build (Hammer) icon and verify the binary generates without errors[cite: 4].
Connect your ST-LINK and target board via USB.
Click the Run (Play) icon.
Both displays will show the startup text, followed by live, synchronized temperature and atmospheric pressure readings.

### I uploaded the zip file of this project in case there is any issue. just extract the zip file and import in the stmcube IDE for the reference.
