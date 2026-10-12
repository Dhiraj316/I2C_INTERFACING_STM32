# I2C_INTERFACING_STM32

This repository contains a complete multi-device I2C application developed for the STM32F407VETx microcontroller. The project demonstrates how to configure a single I2C bus to simultaneously interface with a BMP180 Temperature/Pressure sensor, an SSD1306 OLED display, and a standard I2C LCD character display using the STM32 HAL framework.

 BMP180 + SSD1306 OLED + 16x2 I2C LCD
 
---

---


---

---

##  Wiring / Connections

All three modules connect to the **same two wires**, SCL and SDA, plus power and ground.

```
 STM32F4 Board              BMP180          SSD1306 OLED      LCD 16x2 (I2C)
 -------------             ---------        -------------     ---------------
 PB6 (I2C1_SCL) ---------- SCL ------------ SCL ------------- SCL
 PB7 (I2C1_SDA) ---------- SDA ------------ SDA ------------- SDA
 3V3 --------------------- VIN / VCC ------ VCC
 5V ------------------------------------------------------------ VCC
 GND --------------------- GND ------------ GND ------------- GND
```

### Pin table

| STM32 Pin | Function | BMP180 | OLED | LCD backpack |
|-----------|----------|--------|------|--------------|
| PB6 | I2C1_SCL | SCL | SCL | SCL |
| PB7 | I2C1_SDA | SDA | SDA | SDA |
| 3V3 | Power | VIN / VCC | VCC | none |
| 5V | Power | none | none | VCC |
| GND | Ground | GND | GND | GND |

> If you picked **PB8 / PB9** for I2C1 in CubeMX instead, wire SCL to PB8 and SDA to PB9.
> Some LCD backpacks use `0x3F` instead of `0x27`, and some OLEDs use `0x3D` instead of `0x3C`. If a display stays blank, see [Troubleshooting](#11-troubleshooting).
---

##  STM32CubeMX / .ioc Configuration

Create a new project in STM32CubeIDE (**File → New → STM32 Project**), pick your MCU or board, and open the `.ioc` file. Then set the following.

### Step 1: System Core → RCC

| Setting | Value |
|---------|-------|
| High Speed Clock (HSE) | **Disable** |
| Low Speed Clock (LSE) | **Disable** |

The project runs on the internal HSI oscillator only.

### Step 2: System Core → SYS

| Setting | Value |
|---------|-------|
| Debug | **Serial Wire** |
| Timebase Source | **SysTick** (default) |

This is needed for `HAL_Delay()` and for flashing with ST-Link.

### Step 3: Connectivity → I2C1

Set **I2C** to `I2C`, then under **Parameter Settings**:

| Parameter | Value |
|-----------|-------|
| I2C Speed Mode | Standard Mode |
| I2C Clock Speed (Hz) | **100000** |
| Fast Mode Duty Cycle | **Duty cycle Tlow/Thigh = 2** |
| Primary Address Length | 7-bit |
| Dual Address Acknowledged | Disabled |
| Primary Slave Address | 0 |
| General Call Address Detection | Disabled |
| Clock No Stretch Mode | Disabled |

**Pins** (check these in the Pinout view):

| Pin | Signal |
|-----|--------|
| **PB6** | `I2C1_SCL` |
| **PB7** | `I2C1_SDA` |

The handle name must stay **`hi2c1`**, because all the driver libraries refer to it. Leave GPIO settings at their defaults (Alternate Function Open-Drain, no internal pull, very high speed). No interrupts or DMA are needed.

### Step 4: Clock Configuration tab

| Setting | Value |
|---------|-------|
| Clock source | **HSI RC (16 MHz)** |
| System Clock Mux | **HSI** (PLL is not used) |
| HCLK | **16 MHz** |
| AHB / APB1 / APB2 prescalers | **/1** |

Everything runs at 16 MHz, so Flash latency is 0 wait states.

if you want then keep the ioc. setting default or varry as per you needed.
Click **Generate Code** (or press `Ctrl + S` in CubeIDE and accept the prompt).

---

##  Adding the Driver Libraries

The project uses four small driver libraries, each a `.c` + `.h` pair.

| Library | Files | Used for |
|---------|-------|----------|
| SSD1306 OLED driver | `ssd1306.c`, `ssd1306.h`, `fonts.c`, `fonts.h` | OLED graphics and text |
| BMP180 driver | `bmp180.c`, `bmp180.h` | Temperature and pressure |
| I2C LCD driver | `i2c-lcd.c`, `i2c-lcd.h` | 16x2 LCD via PCF8574 |

**How to add them:**

1. Copy every `.h` file into `Core/Inc/`.
2. Copy every `.c` file into `Core/Src/`.
3. In CubeIDE, right-click the project and choose **Refresh** (`F5`).
4. Open each driver and make sure it uses your I2C handle (`hi2c1`) and the right I2C address (see the table in [section 2](#i2c-addresses-used)):
   - `i2c-lcd.c`: LCD address (`0x4E`, or `0x7E` for the PCF8574A)
   - `bmp180.c`: BMP180 address (`0xEE`)
   - `ssd1306.h`: OLED address (`0x78`) and size (`128 x 64`)

---

##  Writing the Code

CubeMX generates `main.c`. Add your own code **only between the `USER CODE BEGIN` / `USER CODE END` markers**, otherwise CubeMX will erase it the next time you regenerate.

### .1 Includes

```c
/* USER CODE BEGIN Includes */
#include "ssd1306.h"
#include "fonts.h"
#include "bmp180.h"
#include "i2c-lcd.h"
#include <stdio.h>
/* USER CODE END Includes */
```

### .2 Initialization (before the main loop)

```c
/* USER CODE BEGIN 2 */
  HAL_Delay(100);

  // 1. Initialize the OLED
  SSD1306_Init();
  SSD1306_Clear();

  // 2. Initialize the LCD
  lcd_init();

  // 3. Initialize the BMP180
  BMP180_Init();

  // 4. Print startup messages on both screens
  SSD1306_GotoXY(10, 20);
  SSD1306_Puts("SYSTEM READY", &Font_11x18, 1);
  SSD1306_UpdateScreen();

  lcd_put_cur(0, 0);
  lcd_send_string("HELLO");
  lcd_put_cur(1, 0);
  lcd_send_string("RCPIT");

  HAL_Delay(1500);

  // Clear the LCD before starting the sensor loop
  lcd_send_cmd(0x01);
  HAL_Delay(5);
/* USER CODE END 2 */
```

### .3 Main loop

```c
/* USER CODE BEGIN WHILE */
while (1)
{
  // 1. Read the sensor data
  float temp = BMP180_GetTemp();
  float pressure = BMP180_GetPress(0);

  // 2. The float-to-integer trick (no float printf needed)
  int temp_whole   = (int)temp;
  int temp_decimal = (int)((temp - temp_whole) * 10);
  int press_whole  = (int)(pressure / 100);   // Pa -> hPa

  // 3. Format the text (trailing spaces erase old characters on the LCD)
  char tempStr[30];
  char pressStr[30];

  sprintf(tempStr,  "Temp: %d.%d C   ", temp_whole, temp_decimal);
  sprintf(pressStr, "Pres: %d hPa   ", press_whole);

  // 4. Update the OLED
  SSD1306_Clear();
  SSD1306_GotoXY(5, 10);
  SSD1306_Puts(tempStr, &Font_11x18, 1);
  SSD1306_GotoXY(5, 35);
  SSD1306_Puts(pressStr, &Font_11x18, 1);
  SSD1306_UpdateScreen();

  // 5. Update the LCD
  lcd_put_cur(0, 0);
  lcd_send_string(tempStr);
  lcd_put_cur(1, 0);
  lcd_send_string(pressStr);

  // 6. Wait before polling the sensor again
  HAL_Delay(1000);

  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
}
/* USER CODE END 3 */
```

The rest of `main.c` (`HAL_Init()`, `SystemClock_Config()`, `MX_GPIO_Init()`, `MX_I2C1_Init()`, `Error_Handler()`) is generated by CubeMX from your `.ioc`, so do not edit it by hand.

---

##  Build and Flash

1. Connect the STM32 board to your PC with USB (ST-Link).
2. **Project → Build Project** (`Ctrl + B`). It should finish with `0 errors`.
3. **Run → Run** (or **Debug**, `F11`) to flash the board.
4. After flashing, press the **RESET** button if the program does not start by itself.

---

##  Expected Output

**At startup (about 1.5 seconds)**

| OLED | LCD |
|------|-----|
| `SYSTEM READY` | `HELLO` / `RCPIT` |

**After that (updates every second)**

| Display | Line 1 | Line 2 |
|---------|--------|--------|
| OLED | `Temp: 27.4 C` | `Pres: 1008 hPa` |
| LCD | `Temp: 27.4 C` | `Pres: 1008 hPa` |

Values depend on your room and altitude. Pressure is **absolute** pressure, so it is lower than the usual 1013 hPa sea-level value if you live at a higher altitude.

---

##  Project Structure

```
STM32_BMP180_OLED_LCD/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── ssd1306.h
│   │   ├── fonts.h
│   │   ├── bmp180.h
│   │   └── i2c-lcd.h
│   └── Src/
│       ├── main.c            <- application code (this project)
│       ├── ssd1306.c
│       ├── fonts.c
│       ├── bmp180.c
│       └── i2c-lcd.c
├── Drivers/                  <- STM32 HAL + CMSIS (auto-generated)
├── STM32_BMP180_OLED_LCD.ioc <- CubeMX configuration
└── README.md
```

---

### I uploaded the zip file of this project in case there is any issue. just extract the zip file and import in the stmcube IDE for the reference.
