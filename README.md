## Project Description
This project demonstrates the basics of using the MJD component "mjd_ds3231" for the following RTC Real Time Clock boards:
1. The blue circular ChronoDot V2 Real Time Clock Module (recommended).
2. A barebone DS3231MZ+ SOIC-8 150_mil chip on a SOIC-8 breakout board (soldered manually).
3. The blue rectangular large ZS042 DS3231 RTC Real Time Clock Module.
4. The black smaller model (with yellow capacitor) for Raspberry Pi RTC Real Time Clock Module.



Use this project to get insights in how to use this component.

Goto the subdirectory "components/mjd_ds3231" for installation/wiring/usage instructions, data sheets, FAQ, photo's, etc. for the hardware and software.



## Related ESP-IDF projects

`esp32_ds3231_32khz_oscillator_using_lib` This project demonstrates the hardware and software setup to use a DS3231 as an external 32KHz external oscillator for the ESP32 (and get accurate timings in ESP32's deep sleep as well).



## What are the HW SW requirements of the ESP32 MJD Starter Kit?

### Hardware

- A decent ESP development board. I suggest to buy a popular development board with good technical documentation and a significant user base. Examples: [LOLIN D32](https://wiki.wemos.cc/products:d32:d32),  [Adafruit HUZZAH32](https://www.adafruit.com/product/3405),  [Espressif ESP32-DevKitC](http://espressif.com/en/products/hardware/esp32-devkitc/overview), [Pycom](https://pycom.io/hardware/).
- The peripherals that are used in the project. The README.md of each component contains a section "Shop Products".

### Software: ESP-IDF v3.2

- A working installation of the **Espressif ESP-IDF *V3.2* development framework**** (detailed instructions @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).

```
mkdir ~/esp
cd    ~/esp
git clone -b v3.2 --recursive https://github.com/espressif/esp-idf.git esp-idf-v3.2
```

- A C language editor or the Eclipse IDE CDT (instructions also @ http://esp-idf.readthedocs.io/en/latest/get-started/index.html).



## Running the example
```
cd ~/esp32_ds3231_clock_using_lib
make menuconfig
make flash monitor
```



## Reference: the ESP32 MJD Starter Kit SDK

Do you also want to create innovative IoT projects that use the ESP32 chip, or ESP32-based modules, of the popular company Espressif? Well, I did and still do. And I hope you do too.

The objective of this well documented Starter Kit is to accelerate the development of your IoT projects for ESP32 hardware using the ESP-IDF framework from Espressif and get inspired what kind of apps you can build for ESP32 using various hardware modules.

Go to https://github.com/pantaluna/esp32-mjd-starter-kit

