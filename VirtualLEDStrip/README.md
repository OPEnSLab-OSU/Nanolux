# Requirments
OpenCV,
NumPy,
PySerial,
PySimpleGUI,
Pillow

You no longer need to ensure the LED strip is the correct length: this is done automatically by the simulator.

# Usage
1. Uncomment the flag "VIRTUAL_LED_STRIP", this will begin writing out to the serial port, ensure that the serial monitor is closed.
2. Ensure the flag "DEBUG" is commented. This can cause instability in the simulator.
2. Ensure that the ESP32 is plugged into your machine, on communication port 4(COM4).
3. Then simply run the python script

# Future Revisions
1. Abstract all NanoLux features into a class.
2. Move all UI code into a separate ui.py file.
3. Move all reads from serial to a separate thread.
4. Improve auto disconnection logic.
