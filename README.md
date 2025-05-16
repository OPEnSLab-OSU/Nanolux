# Nanolux
Cymaspace [Audiolux](https://www.cymaspace.org/audiolux/) Opensource code

The Audiolux is a product developed by Portland-based company CymaSpace that visualizes music or other performing arts on LED strips, enabling the deaf and hard of hearing to participate in these events. The Audiolux 1 is a pre-existing product, so the goal of this project is to make improvements such as adding user configurable modes, increasing the number of visualization patterns on the LEDs, making configuration wireless, and improving accuracy of pitch detection.

Please check out our [Wiki](https://github.com/OPEnSLab-OSU/Nanolux/wiki) for more user info and documetation.

# Install the Arduino Desktop IDE
Dev environment:
* To get step-by-step instructions select one of the following link accordingly to your operating system.

Windows (http://docs.arduino.cc/software/ide-v1/tutorials/Windows)

Mac OS (http://docs.arduino.cc/software/ide-v1/tutorials/macOS)

Linux (http://docs.arduino.cc/software/ide-v1/tutorials/Linux)

# Dependencies
You will also need several libraries. See the [IDE Setup Guide](https://github.com/OPEnSLab-OSU/Nanolux/wiki/Arduino-IDE-Setting-Guide) Wiki Page for steps on how to install these libraries.

- Ai ESP32 Rotary Encoder (1.6)
- ArduinoJson (7.1.0)
- FastLED (3.7.0)
- ArduinoFFT (2.0.2)
- ESP32 Board Library (2.0.17)


All dependencies are above installable through the Library Manager of the Arduino IDE, or with the 
[Arduino CLI](https://arduino.github.io/arduino-cli/0.28/) tool.

The following dependencies must be installed manually. Refer to Wiki for more info.

* ESPAsynWebServer (1.2.3): https://github.com/me-no-dev/ESPAsyncWebServer
* AsyncTCP (1.1.4): https://github.com/me-no-dev/AsyncTCP


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

2023-2024 Team:

Andrew Adisoemarta (adisoema@oregonstate.edu)

Anshul Batish (batisha@oregonstate.edu)

Caleb Shilling (shillinc@oregonstate.edu)

Can Yang (yangca@oregonstate.edu)

Scout Hawkey (hawkeys@oregonstate.edu)

2024-2025 Team:

Andrew Craeton (craetona@oregonstate.edu)

Theodore Ngo (ngoth@oregonstate.edu)

Matthew Manuguid (manuguim@oregonstate.edu)

Nicholas Wooldridge (wooldrni@oregonstate.edu)


# Some Important Things To Note

1. The most current up to date code is located in the [Main](https://github.com/OPEnSLab-OSU/Nanolux/tree/main/main) directory. 
This is the code that should be uploaded to the device. Make sure to follow the webapp wiki first however to upload the code
needed to run the webapp.

2. If you wish to disable the webapp, you can comment out the flag "ENABLE_WEB_SERVER". Make note to which analog ports are being used,
disabling the webapp however should not give a boost in performance, given that the webapp is asych and on a seperate core.


