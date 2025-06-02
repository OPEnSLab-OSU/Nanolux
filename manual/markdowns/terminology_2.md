## Terminology Overview ##

During the 2023-2024 project cycle, new features were implemented that required major changes to both project structure and terminology. These features persist throughout the 2024-2025 development cycle.

- **Pattern:** An algorithm that controls LED light output for a portion of the LED strip.
- **Strip:** The entire LED strip.
- **Strip Buffer:** Data generated from the previous pattern iteration that will be used to generate the next pattern iteration.
- **Config Data:** Data related to the state of the system.
- **Strip Data:** Configuration data that affects all currently-running patterns. Contains multiple instances of Pattern Data within it.
- **Pattern Data:** Configuration data that affects a single running pattern.
- **Save Slot:** A spot in non volatile storage (NVS) that can store a single strip data instance.

The Nanolux project has many code structures named after the terminology they represent. For example, in the storage.h file, there are 3 structs named "Config\_Data," "Strip\_Data," and "Pattern\_Data."

	typedef struct{
	
	  uint8_t idx = 0; /// The selected pattern name to run.
	  uint8_t brightness = 255; /// The pattern's brightness.
	  uint8_t smoothing = 0; /// How smoothed pattern light changes are.
	  uint8_t minhue = 0;
	  uint8_t maxhue = 255;
	  uint8_t config = 0; // diffrent configs
	  uint8_t postprocessing_mode = 0; // The current mode for postprocessing
	  
	} Pattern_Data;
	  
	
	/// A structure holding strip configuration data.
	typedef struct{
	
	  uint8_t alpha = 0; /// How transparent the top pattern is in Z-layering.
	  uint8_t noise_thresh = 0; /// The minimum noise floor to consider as audio.
	  uint8_t mode = 0; /// The currently-running pattern mode (splitting vs layering).
	  uint8_t pattern_count = 1; /// The number of patterns this config has.
	  Pattern_Data pattern[PATTERN_LIMIT]; /// Data for all patterns loaded.
	
	} Strip_Data;
	
	/// A structure holding system configuration data.
	typedef struct{
	
	  uint8_t length = 60; /// The length of the LED strip.
	  uint8_t loop_ms = 40; /// The number of milliseconds one program loop takes.
	  uint8_t debug_mode = 0; /// The currently selected debug output mode.
	  bool init = true; /// If the loaded config data is valid.
	  char pass[16] = ""; // The current device password
	
	} Config_Data;

As seen in the above code snippet, Strip\_Data contains multiple instances of Pattern\_Data. This process is to enable easy LED strip saving and loading, as the only structure that needs to be explicitly transferred to NVS is the running instance of Strip\_Data.

Nanolux also uses a "Strip Buffer" structure to store the previous state of the LED strip. This structure holds the buffer used by that pattern along with variables used in calculation, such as the mass and position of a virtual spring. This structure is located in patterns.h.

Each currently running pattern on the strip has an accompanying Strip Buffer object. The relationship between a Strip Buffer object and a Pattern Data object is purely associative: the only piece of information that links them is a common array index.

Nanolux also uses libraries or features of the micro controller that could use additional definition.

- **ESP32:** An Arduino-compatible microcontroller with built-in WiFi and Bluetooth.
- **NVS:** Stands for "non-volatile storage." NVS is persistent memory built into the ESP32 itself that persists through power cycles.
- **FastLED:** An Arduino-compatible library capable of outputting to an LED strip. Contains many useful functions, such as blend().