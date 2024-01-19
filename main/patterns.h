#ifndef PATTERNS_H
#define PATTERNS_H

typedef struct{

  int frame = 0;                 // for spring mass
  double amplitude = 0;          //for spring mass 2
  int tempHue = 0;
  int vol_pos = 0;
  int pix_pos = 0;
  uint8_t genre_smoothing[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int genre_pose = 0;
  double max1[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max2[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max3[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max4[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  double max5[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int maxIter = 0;
  double velocity = 0;
  double acceleration = 0;
  double smoothing_value[10] = {0,0,0,0,0,0,0,0,0,0};
  int location = 70;
  double velocities[5] = {0,0,0,0,0};
  double accelerations[5] = {0,0,0,0,0};
  int locations[5] = {70,60,50,40,30};
  double vRealSums[5] = {0,0,0,0,0};

} Pattern_History;

void nextPattern();

void setColorHSV(CRGB* leds, byte h, byte s, byte v);

void freq_hue_vol_brightness();

void freq_confetti_vol_brightness();

void volume_level_middle_bar_freq_hue();

void freq_hue_trail();

void blank();

void spring_mass_1();

void spring_mass_2();

void spring_mass_3 ();

void classical();

void pix_freq();

void mirror_pix_freq();

void send_wave();

void math();

void band_brightness();

void advanced_bands();

void basic_bands();

void eq();

void show_drums();

void show_formants();

void noisy();

void formant_band();

void alt_drums();

void formant_test();

void Fire2012WithPalette();

void saturated_noise();

void saturated_noise_hue_octaves();

void saturated_noise_hue_shift();

void saturated_noise_compression();

void groovy_noise();

void groovy_noise_hue_shift_change();

void sin_hue_trail();

void freq_hue_trail_mid();

void freq_hue_trail_mid_blur();

void talking_hue();

void talking_formants();

void talking_moving();

void bounce_back();

void glitch();

void glitch_talk();

void glitch_sections();

void echo_ripple();

#endif