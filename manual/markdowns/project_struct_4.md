##Project Structure##

###Microcontroller###

All microcontroller code is located in the repository's "main" folder. The entry point of the program is the file main.ino, which calls all other parts of the microcontroller codebase.

#####Constants and Globals#####

There are 2 files dedicated to initializing variables and/or macros: globals.h and nanolux\_types.h. Globals contains mostly global variables that aren't utilized in main.ino, in addition to defining the Pattern struct and its accompanying array. nanolux\_types contains nearly macros utilized as constant values by the codebase.

#####Audio Analysis#####

main.ino calls the audio analysis pipeline, which consists of audio processing functions from the files core\_analysis.h and ext\_analysis.h. The core\_analysis files perform the basic audio analysis that most patterns pull from. The ext\_analysis files perform audio analysis processes that are used by only a few patterns, but are more complex.

#####Utility#####

The nanolux\_util files contain a variety of "helper" functions. The functions in these files are used all throughout the program and deal with hardware and Nanolux-specific custom mathematical operators, among other actions.


#####Patterns#####

The files patterns.h and patterns.cpp contain the actual code to run the pattern algorithms themselves. Their function pointers are handled by the Pattern array in globals.h, so this file is rarely directly addressed in the code. These files pull from the palettes.h to get color palettes for a few pattern algorithms.


#####Storage#####

The files storage.h and storage.cpp contain the code needed to save and load system setting configurations, patterns, and strip configurations. As stated before, storage.h contains the three data structures used for containing these three data categories.

#####Networking#####

Nanolux network communications are managed in the files api.h and webServer.h. The file api.h contains the request handlers for controlling pattern and strip settings, while webServer.h contains the code initializing the web app, bridging to local network connections, starting the built-in router, and manages the API for updating networking-specific device settings. You can find more on the API on its accompanying wiki page.

###Web App###

All web app code is included in the path WebApp/src. It can roughly be divided into the following segments:

- Encapsulated Components
- Control Components
- Routes
	- Device Settings
	- Wifi Settings
- The API

#### Encapsulated Components ####

Encapsulated components are components that

1. Perform a singular function
2. "Encapsulate" functionality of a simpler element (say, a drop down list) so it can be reused without issue

There are far too many to go over here in depth, so here is a list of all encapsulated components with brief descriptions for each:

- Config Drop Down
	- Allows the user to select a configuration for a running pattern (such as volume controlled vs frequency controlled)
- Header
	- Allows the user to navigate between the settings and WiFi pages
- Multi Range Slider
	- Allows the user to specify two values that compose a range
- Network Modal
	- Presents a description of the current connectivity status between the web app and device
- Numeric Slider
	- Allows the user to enter a value via a slider
- Palette Selector
	- Allows the user to style the web app's coloring
- Password
	- Allows the user to input a WiFi password to connect to a local network
- Pattern Modal
	- Presents a comprehensive list of pattern names with a brief description for each
- Patterns
	- Presents a drop down list of pattern names supplied from the device
- Redirect Modal
	- Allows the user to redirect to a configured device's URL
- Save Entry
	- Allows the user to save and load a pattern from NVS
- Single Chooser
	- Allows the user to select one or none of of an option list.
- Spinner
	- Presents a simple visual to convey that the web app is processing a user's request
- Text Input
	- Allows the user to input and submit text for configuring WiFi settings
- Toast
	- Presents an error or status message to the user
- Tooltip
	- Presents popup descriptions when specific UI components are hovered
- WiFi Selector
	- Allows the user to scan for and select a WiFi network to connect to

Let's use Numeric Slider as an example for the structure of an encapsulated component. It has the following parameters input to the element itself. Here is a snippet of the JSDocs header from the element:

	/**
	 * @brief A UI element that creates a draggable slider and an readout
	 * showing the value set by the slider.
	 * 
	 * @param label The label that is displayed alongside the slider.
	 * @param min The minimum value selectable by the slider.
	 * @param max The maximum value selectable by the slider.
	 * @param initial The initial value shown by the slider.
	 * @param structure_ref The string reference to store values at.
	 * @param update A function to update an external data structure.
	 * 
	 * @returns The UI element itself.
	 */

The essential values that separate an encapsulated component from a regular component are the structure\_ref and update parameters. As seen in a later section, most values are sent to the Audiolux device through data structures, which helps cut down on the number of API calls and makes adding new parameters easier. Effectively, "structure ref" is the element in that structure the component is responsible for modifying. The parameter update contains a function which performs that modification and sets a flag in the parent control component:

    const valueChanged = async event => {
        current.value = event.target.value;
        update(structure_ref, current.value);
    }

From the PatternSettings control component:

	/**
	 * @brief Updates a parameter in the pattern data structure with a new value.
	 * @param ref The string reference to update in the data structure
	 * @param value The new value to update the data structure with
	 */
	const update = (ref, value) => {
		if(!loading){		
			setData((oldData) => {
				let newData = Object.assign({}, oldData);
				newData[ref] = value;
				return newData;
			})
		}	
		setUpdated(true);
	}

#### Control Components ####

Control components are responsible for housing a related collection of encapsulated components and coordinating their API connection to the Audiolux device itself. There are currently 3 in the web app:

- Pattern Settings
- Strip Settings
- System Settings

System settings is instantiated separately from the other two, but Strip Settings itself contains an instance of Pattern Settings.

Control components revolve around the concept of a singular data structure. This data structure is updated with new data from the device upon connection, and sends out updates to the device when the user changes a parameter on the web UI. Here is an example of one, taken from Pattern Settings:

	// Pattern-level data structure
	const [data, setData] = useState({
		idx: 0,
		hue_max: 255,
		hue_min: 0,
		brightness: 255,
		smoothing: 0,
		postprocess: 0,
	});

As stated earlier, whenever a child encapsulated component causes the data structure to update, the parent control component sets a flag that enables the web app to send an API call to the device. This is controlled by a basic interval that checks for that flag.

#### Routes / Subpages ####

The concept of Routes in a web application concept are essentially subpages. There are two routes in the web application at this time: one for controlling device settings and one for controlling WiFi settings.

##### Device Settings Route #####

The web page for controlling device settings controls strip, pattern, and system data that does not involve WiFi. It creates the following objects:

- Strip Settings
	- Contains a Pattern Settings object inside of it
- System Settings
- 3 Save Entry objects for the 3 save slots on the device.

##### WiFi Settings Route #####

This route could benefit from a refactor, and has a lot of repeated code. However, it is also incredibly difficult to understand and probably requires a great deal of time before it can be modified.