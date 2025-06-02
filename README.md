# Nanolux
Cymaspace [Audiolux](https://www.cymaspace.org/audiolux/) Opensource code

The Audiolux is a product developed by Portland-based company CymaSpace that visualizes music or other performing arts on LED strips, enabling the deaf and hard of hearing to participate in these events. The Audiolux 1 is a pre-existing product, so the goal of this project is to make improvements such as adding user configurable modes, increasing the number of visualization patterns on the LEDs, making configuration wireless, and improving accuracy of pitch detection.

#### Please check out our [Wiki](https://github.com/OPEnSLab-OSU/Nanolux/wiki) for more user info and documetation.

# Install the Arduino Desktop IDE
Dev environment:
* To get step-by-step instructions select one of the following link accordingly to your operating system.

Windows (http://docs.arduino.cc/software/ide-v1/tutorials/Windows)

Mac OS (http://docs.arduino.cc/software/ide-v1/tutorials/macOS)

Linux (http://docs.arduino.cc/software/ide-v1/tutorials/Linux)


## Dependencies

### 1. Using the **`dependencies`** branch (Recommended)
We have bundled all of the manually required libraries into a single `libraries/` folder on the **`dependencies`** branch. That way, you don’t have to install each one by hand. See the next section if you want/need to manually install them.

1. **Clone (or fetch) the repo and switch to `dependencies`:**
   ```bash
   git clone https://github.com/OPEnSLab-OSU/Nanolux.git
   cd Nanolux
   git fetch origin
   git checkout dependencies
   ```
2. **Locate the `libraries/` folder** at the root of this branch. It contains:
   - ESPAsyncWebServer (v1.2.3)
   - AsyncTCP (v1.1.4)
   - AudioPrism
   - All other custom or manually‑required libraries as seen in the next section
3. **Copy (or symlink) that `libraries/` folder into your Arduino sketchbook’s “libraries” directory.**  
   - On Windows, your sketchbook is usually:  
     ```
     C:\Users\<YourUserName>\Documents\Arduino\libraries
     ```
   - On macOS/Linux, it’s typically:  
     ```
     ~/Arduino/libraries
     ```
   For example:
   ```bash
   # if your sketchbook is ~/Arduino
   cp -r libraries ~/Arduino/
   ```

   Once that’s done, Arduino IDE will automatically pick up those libraries the next time you open it.

4. **Switch back to `main` (if you want to compile/upload the main firmware):**
   ```bash
   git checkout main
   ```
   Because your `main/` folder references those libraries by name, Arduino can now find them.

### 2. Installing via Arduino IDE / Library Manager (Alternate)
You will also need several libraries. See the [IDE Setup Guide](https://github.com/OPEnSLab-OSU/Nanolux/wiki/Arduino-IDE-Setting-Guide) Wiki Page for steps on how to install these libraries.

- Ai ESP32 Rotary Encoder (1.6)
- ArduinoJson (7.1.0)
- FastLED (3.7.0)
- fast4ier (1.0.0)
- ESP32 Board Library (2.0.17)

All dependencies above are installable through the Library Manager of the Arduino IDE, or with the 
[Arduino CLI](https://arduino.github.io/arduino-cli/0.28/) tool.

The following dependencies must be installed manually. Refer to Wiki for more info.

* ESPAsynWebServer (1.2.3): https://github.com/me-no-dev/ESPAsyncWebServer
* AsyncTCP (1.1.4): https://github.com/me-no-dev/AsyncTCP
* AudioPrism: https://github.com/udellc/AudioPrism


# Install Board
You will need to install the ESP32 board driver for developing on this board.
* Install ESP32 Board: https://www.hackster.io/abdularbi17/how-to-install-esp32-board-in-arduino-ide-1cd571
You will also need to check the port connection with ESP32 Board.
* Serial Connection with ESP32: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/establish-serial-connection.html


# Memory model
After setting up the board, you will need to select a memory model with enough room for the libraries. In the Arduino IDE, 
go to Tools->Partition Scheme and make sure "No OTA (Large APP)" is selected.

# Using the AudioLux Device
Once the device is set up correctly, it can be powered on using its micro USB and fed audio through the 3.5mm audio jack. To configure the device, either use the rotary encoder to cycle through patterns and modes or use the AudioLux web app (ensure the web application is deployed by following these steps in the Wiki: [The Web App](https://github.com/OPEnSLab-OSU/Nanolux/wiki/The-Web-App)). Once deployed by the device, the web app can be connected to by opening your device's Wifi settings and selecting the AudioLux Wifi (`audiolux unsecured`) that is hosted by the AudioLux device, and then opening the web app by going to `http://192.168.4.1/` if the device has not been configured before or `http://audiolux.local` if the device has been configured. Once connected, the AudioLux device can be configured freely using all of the web app's functionality and/or the device's rotary encoder.


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

1. The most up to date atble release is located in the [Main](https://github.com/OPEnSLab-OSU/Nanolux/tree/main/main) directory. 
This is the code that should be uploaded to the device. Make sure to follow the webapp wiki first however to upload the code
needed to run the webapp.

2. If you wish to disable the webapp, you can comment out the flag "ENABLE_WEB_SERVER". Make note to which analog ports are being used,
disabling the webapp however should not give a boost in performance, given that the webapp is asynch and on a seperate core.


