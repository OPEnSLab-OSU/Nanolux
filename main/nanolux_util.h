#ifndef NANOLUX_UTIL_H
#define NANOLUX_UTIL_H

void IRAM_ATTR buttonISR();

void check_button_state();

int remap( double x,double oMin,double oMax,double nMin,double nMax );

int largest(double arr[], int n);

void IRAM_ATTR button_down();

void IRAM_ATTR button_up();

#endif