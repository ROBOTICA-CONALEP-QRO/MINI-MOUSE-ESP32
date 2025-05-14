# ESP32 DevKitC V4 (30-Pin) Pinout

Descripción detallada de los pines para la placa de desarrollo ESP32 de 30 pines, basada en la configuración común de DevKitC V4.

## Pines del Lado Izquierdo (Vistos desde arriba, con el puerto USB a la izquierda)

| Pin Físico | Nombre ESP32 | Funciones Principales                                     | Notas                                       |
|------------|--------------|-----------------------------------------------------------|---------------------------------------------|
| 1          | EN           | Enable (Reset). Activo bajo.                            | Mantener HIGH para operación normal.        |
| 2          | GPIO36       | SENSOR_VP, ADC1_CH0, RTC_GPIO0                            | **Solo Entrada**.                           |
| 3          | GPIO39       | SENSOR_VN, ADC1_CH3, RTC_GPIO3                            | **Solo Entrada**.                           |
| 4          | GPIO34       | IO34, ADC1_CH6, RTC_GPIO4                                 | **Solo Entrada**.                           |
| 5          | GPIO35       | IO35, ADC1_CH7, RTC_GPIO5                                 | **Solo Entrada**.                           |
| 6          | GPIO32       | IO32, ADC1_CH4, TOUCH9, RTC_GPIO9                         | Entrada/Salida digital, ADC, Táctil.        |
| 7          | GPIO33       | IO33, ADC1_CH5, TOUCH8, RTC_GPIO8                         | Entrada/Salida digital, ADC, Táctil.        |
| 8          | GPIO25       | IO25, ADC2_CH8, DAC_1, RTC_GPIO6                          | Entrada/Salida digital, ADC, **DAC1**.      |
| 9          | GPIO26       | IO26, ADC2_CH9, DAC_2, RTC_GPIO7                          | Entrada/Salida digital, ADC, **DAC2**.      |
| 10         | GPIO27       | IO27, ADC2_CH7, TOUCH7, RTC_GPIO17                        | Entrada/Salida digital, ADC, Táctil.        |
| 11         | GPIO14       | IO14, ADC2_CH6, TOUCH6, RTC_GPIO16, MTMS, **HSPICLK**     | Entrada/Salida digital, ADC, Táctil, SPI.   |
| 12         | GPIO12       | IO12, ADC2_CH5, TOUCH5, RTC_GPIO15, MTDI, **HSPIQ (MISO)** | Entrada/Salida digital, ADC, Táctil, SPI.   |
| 13         | GPIO13       | IO13, ADC2_CH4, TOUCH4, RTC_GPIO14, MTCK, **HSPID (MOSI)** | Entrada/Salida digital, ADC, Táctil, SPI.   |
| 14         | GND          | Tierra (Ground)                                           | Conexión a 0V.                              |
| 15         | VIN          | Voltaje de Entrada                                        | Generalmente 5V desde USB o fuente externa. |

## Pines del Lado Derecho (Vistos desde arriba, con el puerto USB a la izquierda)

| Pin Físico | Nombre ESP32 | Funciones Principales                                     | Notas                                       |
|------------|--------------|-----------------------------------------------------------|---------------------------------------------|
| 16         | GPIO23       | IO23, **VSPIMOSI**                                        | Entrada/Salida digital, SPI.                |
| 17         | GPIO22       | IO22, VSPIWP, **SCL (I2C)**, RTC_GPIO11, MTDI             | Entrada/Salida digital, I2C (Reloj).        |
| 18         | GPIO1        | IO1, **TXD0 (UART0)**, CLK_OUT1, EMAC_TX_CLK              | Entrada/Salida digital, UART (Transmisión). |
| 19         | GPIO3        | IO3, **RXD0 (UART0)**, CLK_OUT2                           | Entrada/Salida digital, UART (Recepción).   |
| 20         | GPIO21       | IO21, VSPIHD, **SDA (I2C)**, RTC_GPIO10, EMAC_TX_EN       | Entrada/Salida digital, I2C (Datos).        |
| 21         | GPIO19       | IO19, **VSPIQ (MISO)**, U0CTS, EMAC_TXD0                  | Entrada/Salida digital, SPI.                |
| 22         | GPIO18       | IO18, **VSPICLK**, U0RTS, EMAC_TXD1                       | Entrada/Salida digital, SPI (Reloj).        |
| 23         | GPIO5        | IO5, **VSPICS0**, HS1_DATA6, EMAC_RX_CLK                  | Entrada/Salida digital, SPI (Chip Select).  |
| 24         | GPIO17       | IO17, HS1_DATA5, U2TXD, EMAC_RXD0                         | Entrada/Salida digital, UART2 (TX).         |
| 25         | GPIO16       | IO16, HS1_DATA4, U2RXD, EMAC_RXD1                         | Entrada/Salida digital, UART2 (RX).         |
| 26         | GPIO4        | IO4, ADC2_CH0, TOUCH0, RTC_GPIO10, HS1_DATA1, EMAC_TX_ER  | Entrada/Salida digital, ADC, Táctil.        |
| 27         | GPIO0        | IO0, ADC2_CH1, TOUCH1, RTC_GPIO11, CLK_OUT3, EMAC_TX_CLK  | Entrada/Salida digital, ADC, Táctil. **Boot**. |
| 28         | GPIO2        | IO2, ADC2_CH2, TOUCH2, RTC_GPIO12, HSPIWP, HS2_DATA0      | Entrada/Salida digital, ADC, Táctil. **Boot**. |
| 29         | GPIO15       | IO15, ADC2_CH3, TOUCH3, MTDO, **HSPICS0**, RTC_GPIO13     | Entrada/Salida digital, ADC, Táctil, SPI.   |
| 30         | 3V3          | Salida de 3.3V                                            | Alimentación regulada de 3.3V.              |

---

## Resumen de Uso de Pines

### Pines de Propósito General (GPIO)

Estos pines son los más flexibles y pueden usarse como **entrada o salida digital** para tareas comunes como controlar LEDs, leer botones, o interactuar con sensores digitales simples, siempre que no se necesite su función especial en ese momento.

*   **Lado Izquierdo:** GPIO32, GPIO33, GPIO25, GPIO26, GPIO27, GPIO14, GPIO12, GPIO13
*   **Lado Derecho:** GPIO23, GPIO22, GPIO1, GPIO3, GPIO21, GPIO19, GPIO18, GPIO5, GPIO17, GPIO16, GPIO4, GPIO0, GPIO2, GPIO15

*Nota: Algunos de estos pines tienen restricciones o funciones importantes durante el arranque (ver Pines Especiales).*

### Pines Especiales

Estos pines tienen funciones hardware dedicadas o características particulares.

*   **ADC (Conversor Analógico-Digital):** Para leer voltajes analógicos (sensores de luz, potenciómetros, etc.).
    *   **ADC1 (Usable con WiFi):** GPIO36, GPIO39, GPIO34, GPIO35, GPIO32, GPIO33
    *   **ADC2 (NO usable con WiFi):** GPIO25, GPIO26, GPIO27, GPIO14, GPIO12, GPIO13, GPIO4, GPIO0, GPIO2, GPIO15
    *   **Pines Solo Entrada:** GPIO36, GPIO39, GPIO34, GPIO35 (No pueden configurarse como salida).
*   **DAC (Conversor Digital-Analógico):** Para generar voltajes analógicos (salida de audio simple, control de voltaje).
    *   GPIO25 (DAC1)
    *   GPIO26 (DAC2)
*   **Sensores Táctiles Capacitivos (TOUCH):** Para detectar el tacto humano.
    *   GPIO32, GPIO33, GPIO27, GPIO14, GPIO12, GPIO13, GPIO4, GPIO0, GPIO2, GPIO15
*   **SPI (Serial Peripheral Interface):** Comunicación serial rápida con periféricos (pantallas, lectores SD, sensores).
    *   **HSPI:** GPIO14 (CLK), GPIO12 (MISO), GPIO13 (MOSI), GPIO15 (CS)
    *   **VSPI:** GPIO18 (CLK), GPIO19 (MISO), GPIO23 (MOSI), GPIO5 (CS)
*   **I2C (Inter-Integrated Circuit):** Comunicación serial de dos hilos común para muchos sensores y módulos.
    *   GPIO22 (SCL - Reloj)
    *   GPIO21 (SDA - Datos)
*   **UART (Comunicación Serial Asíncrona):**
    *   **UART0:** GPIO1 (TX), GPIO3 (RX) - Usado por defecto para programar y el Monitor Serie.
    *   **UART2:** GPIO17 (TX), GPIO16 (RX)
*   **Pines de Arranque (Strapping Pins):** Afectan cómo arranca el ESP32. ¡Cuidado al usarlos!
    *   **GPIO0:** Poner a GND al arrancar para entrar en modo Bootloader/Flash.
    *   **GPIO2:** Debe estar LOW o flotante al arrancar para modo normal.
    *   **GPIO12 (MTDI):** Puede afectar el voltaje de la flash interna. Evitar si es posible.
    *   **GPIO15 (MTDO):** Emite mensajes de boot.
*   **Enable (EN):** Pin de Reset. Poner a GND momentáneamente para reiniciar el ESP32.

### Pines de Alimentación

*   **VIN:** Entrada de 5V (típicamente desde USB).
*   **3V3:** Salida regulada de 3.3V para alimentar sensores y módulos. **¡El ESP32 funciona a 3.3V!**
*   **GND:** Tierra (0V). Conectar todos los GNDs juntos.

---

**Importante:**

*   **Nivel Lógico:** El ESP32 opera a **3.3V**. Conectar pines de entrada directamente a 5V puede dañarlo. Usa divisores de voltaje o conversores de nivel lógico si interactúas con dispositivos de 5V.
*   **Corriente:** No excedas la corriente máxima por pin (consulta el datasheet del ESP32, usualmente alrededor de 12mA por pin de forma segura).
*   **ADC2 y WiFi:** No puedes usar `analogRead()` en los pines ADC2 mientras el WiFi esté activo. Usa los pines ADC1 en su lugar.
