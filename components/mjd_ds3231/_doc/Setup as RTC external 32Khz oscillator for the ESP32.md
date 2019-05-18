# SOP: setup the RTC board as an external oscillator 32Khz for the ESP32

## General
The ESP32's internal 150Hz crystal oscillator is not very accurate for e.g. data loggers which wake up from deep sleep at regular intervals. It deviates +-1 minute every 4 hours.

Luckily the ESP32 also supports a more accurate RTC external 32kHz crystal oscillator.

**The SQW pin** of the RTC board is the 32K square wave output signal. It is **Open-Drain** so you need to attach **a 10K pullup resistor** to read this signal from a microcontroller pin.

@important The ZS-042 RTC board and the Chronodot RTC board can be used for this purpose. The black/yellow RTC board cannot be used as such because the 32K output pin of the DS3231 chip is not exposed on the PCB.

@important The battery on the RTC board is not used if you only deploy this board as an RTC external 32Khz crystal oscillator for the ESP32.

@important The setup to use the DS3231 as an RTC Real Time Clock via the I2C protocol, as described in the main document, is not required.



## Wiring
```
- Connect RTC board pin VCC => microcontroller pin VCC 3.3V
- Connect RTC board pin GND => microcontroller pin GND
- Connect RTC board pin "32K" => microcontroller pin GPIO#33 (ESP32 pin 32K_XN [not 32K_XP!])
- Connect RTC board pin "32K" => 10K pullup resistor => microcontroller pin VCC 3.3V
```

https://learn.adafruit.com/adafruit-ds3231-precision-rtc-breakout

The Espressif document "ESP32 Hardware Design Guidelines V2.7 of April2019" section "2.1.4.2 RTC (Optional)" specifies a different wiring using the 32K_XP pin but that doesn't work; that procedure might be correct for a crystal, but not for an oscillator. I assume that the document will be corrected.



## Setup

- Goto the project directory, for example `cd ~/my_ds3231_clock_using_lib`
- Run `make menuconfig`.
- Select "Component config  ---> ESP32-specific  ---> RTC clock source ()  --->".
- Change "Internal 150kHz RC oscillator" to "(X)  **External 32kHz oscillator**".
- Exit menuconfig.
- Start your project, for example `make flash monitor`



## Notes
- **Status Register (0Fh) Bit 3 "Enable 32kHz Output (EN32kHz)"**: This bit controls the status of the 32kHz pin. When set to logic 1, the 32kHz pin is enabled and outputs a 32.768kHz square-wave signal. When set to logic 0, the 32kHz pin goes to a high-impedance state. The initial power-up state of this bit is logic 1, and a 32.768khz square-wave signal appears at the 32khz pin after a power source is applied to the DS3231 (if the oscillator is running).
