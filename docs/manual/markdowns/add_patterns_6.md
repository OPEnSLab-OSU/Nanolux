## Adding New Patterns##
---

This is a more code-heavy section, so I would recommend brushing up on your C/C++ if you need to.

Adding new patterns to a device running Nanolux is really easy, and doesn't take that much knowledge about programming aside from the basics. This tutorial will navigate the process of adding a pattern that leverages most things you can do with Nanolux.

### Stating the Problem###

---

It's often a good idea to precisely state out what you want a pattern to do, in the shortest amount possible. For this demo, we'll be working off of the following statement:

	Pattern Name: Bar Fill

	Brief Description: With either volume or frequency, the lit LEDs will climb with increasing volume or frequency,
	and lower if the volume or frequency drops. The hue will be equal to the minimum hue at the lowest pixel and
	equal to the maximum hue at the highest one. The brightness and saturation will remain at their maximums.

	Configurations: Volume (default), Frequency (1)

###ESP32 Implementation###
---
There's a couple of rules to follow when actually implementing a pattern:

-  Only modify values inside of the strip buffer variable.
-  Don't hold up the program (no calls to delay()).
-  Only access positions inside the LED buffer that are within the length you are allowed to modify (the function parameter "len").

Let's start with defining the basic structure of our pattern (in patterns.cpp). Don't forget to create the header function in patterns.h!

	#define VOLUME 0
	#define FREQUENCY 1
	/// @brief Displays a pattern that occupies "lower" pixels at lower values,
	/// and "higher" pixels at higher values.
	/// @param buf Pointer to the Strip_Buffer structure, holds LED buffer and history variables.
	/// @param len The length of LEDs to process
	/// @param params Pointer to Pattern_Data structure containing configuration options.
	void bar_fill(Strip_Buffer * buf, int len, Pattern_Data* params){
	
	  switch(params->config) {
	
	    case VOLUME: default: {
	
	
	      break;
	    }
	    case FREQUENCY: {
	
	
	      break;
	    }
	  }
	}

Patterns in our Pattern Library follow a general basic formula.

Call the Audio Analysis getter functions and assign the result to variables. For example (float volume = audioAnalysis.getVolume()). These functions are dependent on the different aspects of the audio sample that the pattern would like to showcase. The functions that are available to call are listed in the audio_analysis.h file. Further insight into these modules can be found in the
From these variables, create a method to determine which LED's should display which color. This is generally determined by figuring out what HSV value would would showcase the state of the audio sample. For example, the Equalizer function maps the LED strip to varying hues as it goes down the strip. It then determines the Value value, or brightness, based on the amplitude of the frequency bin associated with that LED.
Assign the determined HSV values to the LED. This is most commonly done by setting an item from the buf->leds array to the HSV value of your choice. For example, buf->leds[i] = CHSV(0, 255, 255) sets the LED at position i to red.
HSV Values:

Hue - Color, from Red wrapping back around to Red (0 - 255)
Saturation - How vibrant the color is. The higher the value, the more vivid the hue. The lower the value, the closer it is to a grey. (0-255)
Value - Also called brightness. Simply how bright the LED will be. (0 - 255) HSV values are normally saved in code with CHSV(H value, S value, V value)
Useful Functions:

- blur1d(buf->leds, len, int v). This function blurs the HSV values of the LED strip. The amount that it blurs is dependent on the int v value. The other arguments are generally used as they are.
- fadeToBlackBy(buf->leds, len, int v). This function takes the V values of each LED along the strip and subtracts int v from each of them.
map/remap(int value, int oldMin, int oldMax, int newMin, int newMax). This function takes a value, and reproportions it from its old range to a new range. Useful for taking values like frequency and volume and setting them to HSV values or the length of the LED strip.
- rgb2hsv_approximate(buf->leds[i]). This function takes an RGB value and converts it to an HSV value. This function is useful if you want to check the past HSV value of the buf->leds array. For example, SplashTheory uses this to check what the LED strip looked like last call. This function is necessary because buf->leds saves each cell as an RGB value.
- blend(CHSV color1, CHSV color2, amountToBlend, BlendDirection). This function outputs a color value that's a blend of color1 into color2 by the amount of amountToBlend. BlendDirection are a set of enums that can be found in FastLED's website.

Notes:
- getFhue and getVbrightness are helper functions from inside of our code. They simply utilize the remap function to set Frequency to a hue value or Volume to a brightness value. getFhue divides the log of each frequency value by log 2. This is good practice anytime you utilize frequency values, regardless of using getFhue or not.
- If you need a Static Array to remember previous LED configurations, use MAX_LEDS to initialize.
- Feedback from Cymaspace indicates that simpler, more obviously connected to the audio is more desirable. Feedback from other sources prefer more complex, visually interesting patterns.


This is a good "clean" initial structure that doesn't go overboard with extra code. Cases are clearly labeled using macros, and the Doxygen header is present describing the pattern. You can check out the final pattern in patterns.cpp if you wish to see the final version.

Next, we need to add the pattern to the global patterns array. This is located in globals.h,

	Pattern mainPatterns[]{
	    { 0, "None", true, blank},
	    { 1, "Pixel Frequency", true, pix_freq},
	    { 2, "Confetti", true, confetti},
	    { 3, "Hue Trail", true, hue_trail},
	    { 4, "Saturated", true, saturated},
	    { 5, "Groovy", true, groovy},
	    { 6, "Talking", true, talking},
	    { 7, "Glitch", true, glitch},
	    { 8, "Bands", true, bands},
	    { 9, "Equalizer", true, eq},
	    { 10, "Tug of War", true, tug_of_war},
	    { 11, "Rain Drop", true, random_raindrop},
	    { 12, "Fire 2012", true, Fire2012},
	};
	int NUM_PATTERNS = 13;

And we need to make the following change:

	Pattern mainPatterns[]{
		{ 0, "None", true, blank},
		...
		...
		{ 12, "Fire 2012", true, Fire2012},
    	{ 13, "Bar Fill", true, bar_fill},
	};
	int NUM_PATTERNS = 14;

The ESP32 code is ready to run the pattern, but the web app needs a minor change as well.

### Web App Implementation ###
---
The change is pretty simple: we need to add the pattern configurations (volume, frequency), pattern name (Bar Fill), and a brief pattern description (LEDs climb with increasing volume or frequency while hue is set to the minimum hue at the lowest pixel and the maximum hue at the highest one) into the web app. First, navigate to the file at the relative path WebApp/src/components/config\_drop\_down/index.js and find the following object:

    const configs = [
        ["None"],
        ...
		...
        ["Default"]
    ]

You'll need to add the new pattern's configurations at the end of this list, which is the list of configurations for all patterns.

    const configs = [
        ["None"],
        ...
		...
        ["Default"],
		["Volume", "Frequency"]
    ]

Then, navigate to the file at the relative path WebApp/src/components/pattern\_modal/index.js and find the following object:

	const PATTERN_INFOS = [
		{ title: 'Pixel Frequency',      desc: 'A Pixel will change position based on volume.' },
		...
		...
		{ title: 'Delta Heat',           desc: 'A heat map of the frequency bins based on amplitude change.' },
	];

Again, you'll need to add to the end of this list, which is the list of pattern names and descriptions for all patterns. Add the new pattern's name and description.

	const PATTERN_INFOS = [
		{ title: 'Pixel Frequency',      desc: 'A Pixel will change position based on volume.' },
		...
		...
		{ title: 'Delta Heat',           desc: 'A heat map of the frequency bins based on amplitude change.' },
		{ title: 'Bar Fill',             desc: 'LEDs climb with increasing volume or frequency while hue is set to the minimum hue at the lowest pixel and the maximum hue at the highest one.' },
	];

---

If you followed this guide step-by-step, your pattern should now be selectable in the web app!