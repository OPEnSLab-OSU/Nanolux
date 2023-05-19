# Nanolux
Cymaspace [Audiolux](https://www.cymaspace.org/audiolux/) Opensource code

The Audiolux is a product developed by Portland-based company CymaSpace that visualizes music or other performing arts on LED strips, enabling the deaf and hard of hearing to participate in these events. The Audiolux 1 is a pre-existing product, so the goal of this project is to make improvements such as adding user configurable modes, increasing the number of visualization patterns on the LEDs, making configuration wireless, and improving accuracy of pitch detection. Please check out our [Wiki](https://github.com/OPEnSLab-OSU/Nanolux/wiki) for more user info .

# Install the Arduino Desktop IDE
Dev environment:
* To get step-by-step instructions select one of the following link accordingly to your operating system.

Windows (http://docs.arduino.cc/software/ide-v1/tutorials/Windows)

Mac OS (http://docs.arduino.cc/software/ide-v1/tutorials/macOS)

Linux (http://docs.arduino.cc/software/ide-v1/tutorials/Linux)

# Dependencies
You will also need to also install FastLED, ArduinoFFT Library.
See Wiki Page for including dependencies in your project.
* ArduinoFFT Library (1.5.6): https://github.com/kosme/arduinoFFT
* ArduinoJson (6.20.1): https://arduinojson.org/
* FastLED (3.4.0): http://fastled.io/ (You can also find this in the /dependencies)
* LittleFS_esp32 (1.0.6): https://github.com/lorol/LITTLEFS
* WiFiWebServer (1.10.0): https://github.com/khoih-prog/WiFiWebServer

All dependencies are installable through the Library Manager of the Arduino IDE, or with the 
[Arduino CLI](https://arduino.github.io/arduino-cli/0.28/) tool.


# Install Board
You will need to install the ESP32 board driver for developing on this board.
* Install ESP32 Board: https://www.hackster.io/abdularbi17/how-to-install-esp32-board-in-arduino-ide-1cd571
You will also need to check the port connection with ESP32 Board.
* Serial Connection with ESP32: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/establish-serial-connection.html


# Memory model
After setting up the board, you will need to select a memory model with enough room for the libraries. In the Arduino IDE, 
go to Tools->Partition Scheme and make sure "No OTA (Large APP)" is selected.


# Contributors
2021-2022 Team:
John Zontos (zontosj@oregonstate.edu)

Evan Cochran (cochraev@oregonstate.edu)

Longjie Guan (guanl@oregonstate.edu)

Ming Wei (weimi@oregonstate.edu)

2022-2023 Team:

Andrew Gingerich (gingeria@oregonstate.edu)

Luke Goldsworthy (goldswol@oregonstate.edu)

Mac Handley (handlem@oregonstate.edu)

Vishnu Kalidas (kalidasv@oregonstate.edu)

Jorden Mortenson (mortejor@oregonstate.edu)

David Mora (morad@oregonstate.edu)


