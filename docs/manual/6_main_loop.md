## Main Loop Overview ##

The file main.ino is the entry point for microcontroller code, and as such drives the output LED strip. The file can be roughly split into 5 sections:

1. Initialization
2. Helper Functions
3. Post-Processing
4. Driver Functions
5. Program Loops

This manual section will go over these parts of the main file in order.

---

### Initialization ###

The first 100 or so lines of the file manage imports, initialize variables, and define critical function headers. These are put into groups of similar variables, and have enough in-line documentation available for an end user. The most critical function here is setup(). This function is immediately ran upon microcontroller boot, and performs the following actions:

- Starts the LED strip.
- Initialize the pin attached to the ESP32's built-in LED.
- Start serial communication.
- Add interrupts for the hardware button.
- Load and verify saves stored in non-volatile storage.

An additional feature of setup() is that it runs the function for starting the web server, which hosts the Nanolux web application. This code is wrapped in an #ifdef macro checking if the macro ENABLE\_WEB\_SERVER is defined, which allows the web app to be easily disabled.

---

### Helper Functions ###

The main.ino file has several functions called "helpers" that only main.ino uses. Often, helper functions will either encapsulate code that is used repeatedly throughout the file or encapsulate code that would be clunky to use directly.

#### calculate\_layering() ####

This function takes two input CRGB array pointers, blurs them together, and places the output into an output CRGB array pointer. Used for both pattern layering and the per-pattern output smoothness parameter.

#### reverse\_buffer() ####

This function takes in a CRGB array pointer and, for a supplied length, reverses the values contained within. This maps [0] <-> [len-1], This maps [1] <-> [len-2], and so on. This is used inside both postprocessing effects.

#### unfold\_buffer() ####

The purpose of this function is to take a buffer and a specified length, and "unfold" it to the other side of the buffer. There's some extra math that depends on if the value the length is derived (the actual length of the buffer we care about) is even or odd. In particular, if we performed the even unfolding algorithm in the odd case, there would be an extra pixel at the end of the pattern buffer.

I would highly advise taking a look at this function directly, as its under 10 lines.

#### print\_buffer() ####

This function simply prints out the values stored within the supplied CRGB buffer to serial. Used for displaying LED output on the simulator.

---

### Post-Processing ###

The Nanolux codebase currently supports two main methods of postprocessing to the LED strip output. First, the user is able to reverse the LED strip visually. Secondly, the user is able to mirror the pattern around it's center. Both of these postprocessing effects can be applied at the same time as well. These effects are controlled from the function process\_pattern().

#### process\_pattern() ####

The main goal of process\_pattern() is to apply both effects without writing excessively long code. The general order of logic in the function can be explained as such:

- If the pattern is to be mirrored, cut the processed length in half.
- If the pattern is to be reversed, reverse the pattern's buffer.
	- This has the effect of "un-reversing" a buffer that is already reversed.
	- This allows the pattern handler function to process the pattern as normal.
- Run the pattern handler.
- If the pattern is to be reversed, reverse the pattern's buffer again.
- If the pattern is to be mirrored, run unfold\_buffer() to mirror it.

---

### Driver Functions ###

The driver functions are low-level functions that perform the logic behind pattern layering or strip splitting, the two main "modes" of the device.

#### run\_strip\_splitting() ####

run\_strip\_splitting() is the function ran when the Audiolux is in multi-pattern mode. It splits the strip into a number of sections, ranging between 1-4, but the upper limit is configurable through a macro. After calculating the number of pixels to assign per segment, the following loop is ran for each running pattern:

- Run process\_pattern() for the pattern currently being addressed.
- Copy the processed segment within the associated Strip Buffer to the first LED strip output buffer.
- Scale the processed segment's brightness within the first LED strip output buffer.
- Blur the smoothed output buffer and the first output buffer together on just the segment allocated to the current pattern.

Once these steps have been ran for each running pattern, the smoothed output buffer is full and ready to be output to the LED strip. Separating the smoothing and brightness adjusting into multiple steps and constraining them to particular segment allows every running pattern to have a different amount of applied smoothing and set brightness.

#### run\_pattern\_layering() ####

Pattern layering takes the first two running patterns and combines them, so that one is running above the other with configurable transparency between the two layers. This currently only works for 2 patterns and any more that are set up do not run.

- Run process\_pattern() for both patterns.
- Copy the buffer for each pattern contained in their respective Strip_Buffer object into temporary CRGB arrays.
- Set the brightness of the temporary CRGB array for each pattern.
- Layer the two arrays together.
- Blend the layered CRGB buffer and the smoothed output buffer to smooth the output.

---

### Program Loops ###

The Nanolux codebase runs most code iteratively: i.e., every 10-200 milliseconds or so, part of the program is run again, allowing each run of the program to build off of the previous run. This allows patterns on the LED strip to change with time, not just input audio. There are two main loops in main.ino: loop() and audio/_analysis(). The latter runs all functions needed to sample audio and pull out features, such as volume and frequency. This loop effectively just runs functions from elsewhere in the program.

loop() is a bit different. It is automatically run repeatedly by the microcontroller (audio\_analysis() itself is ran by loop()) and is the "top" of the program. Effectively, this means that all code is ran by and returns to loop, aside from interrupts. Loop is responsible for the following activities:

- Loop timing
	- This ensures all program loops take approximately the same amount of time to complete.
	- At the end of the loop, if not enough time has elapsed, the program will wait until it has.
- Pattern resetting
	- If the web/mobile app has requested a change that requires a reset, all patterns, their strip buffers, and the output buffers will be reset to their initial state.
- Reset button control
	- If the physical button on the Audiolux board is held down for 10 seconds (configurable) the device will reset all saved patterns and configurations.
- Controlling which driver function to run based off user selection
- Displaying to the LED strip
- Outputting to the simulator
- Receiving updates from the web server

---

### The API ###

The API is covered in a page on the Audiolux wiki, which you can check out here: [https://github.com/OPEnSLab-OSU/Nanolux/wiki/Hardware-API](https://github.com/OPEnSLab-OSU/Nanolux/wiki/Hardware-API "Hardware API")