#ifndef OBELISK_CONFIG_H
#define OBELISK_CONFIG_H

#define NUM_LEDS 18
#define COLOURS_PER_LED 3

#define TIMER_RESOLUTION_MS 5

#define COLOURS_PER_LED 3
#define PWM_DUTY_STEPS 10

/* #define COLOUR_ORDER_RRGGBB */
#define COLOUR_ORDER_GGRRBB

/*********************************************************************************
 *  Calculated values
 ********************************************************************************/

#ifdef COLOUR_ORDER_GGRRBB
  #define INDEX_GREEN 0
  #define INDEX_RED 1
  #define INDEX_BLUE 2
#else
  #define INDEX_RED 0
  #define INDEX_GREEN 1
  #define INDEX_BLUE 2
#endif

#endif
