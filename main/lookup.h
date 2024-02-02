#include <stdint.h>

typedef struct{

  uint8_t pattern_1 = 0; // Index number of which pattern is current
  uint8_t pattern_2 = 0;
  uint8_t noise_thresh = 0;
  uint8_t alpha = 0;
  uint8_t mode = 0;
  uint8_t brightness = 255;
  uint8_t smoothing = 0;

} Pattern_Params;