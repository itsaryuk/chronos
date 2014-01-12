// system
#include "project.h"
// driver
#include "counter.h"
#include "ports.h"
#include "display.h"
#include "bmp_as.h"

//#include "adc12.h"
//#include "timer.h"

// logic
#include "user.h"
// Global Variable section
struct counter sCounter;
void reset_counter(void)
{
   sCounter.state = MENU_ITEM_NOT_VISIBLE;
 sCounter.count = 0;
 sCounter.style = 0;
}

void mx_counter(u8 line)
{
 // change walk mode or run mode
 display.flag.update_counter = 1;
 sCounter.style = !sCounter.style;
}
void sx_counter(u8 line)
{
 // reset counter
 sCounter.count = 0;
 display.flag.update_counter = 1;
}
void display_counter(u8 line, u8 update)
{
 u8 * str;
 // Redraw line
 switch( update ) {
 case DISPLAY_LINE_UPDATE_FULL:
     sCounter.data[0] = sCounter.data[1] = sCounter.data[2] = 0;
  sCounter.sum = 0;
  sCounter.rise_state = 0;
  // Start sensor
  bmp_as_start();
  // Set icon

  // Menu item is visible
  sCounter.state = MENU_ITEM_VISIBLE;

  // Display result in xx.x format
 case DISPLAY_LINE_UPDATE_PARTIAL:
  // Display result in xx.x format
  str = int_to_array(sCounter.count, 4, 0);
  display_chars(LCD_SEG_L1_3_0, str, SEG_ON);

  display.flag.update_counter = 0;
  if ( sCounter.style ) {
     display_symbol(LCD_SYMB_ARROW_UP, SEG_ON);
     display_symbol(LCD_SYMB_ARROW_DOWN, SEG_OFF);
  } else {
     display_symbol(LCD_SYMB_ARROW_DOWN, SEG_ON);
         display_symbol(LCD_SYMB_ARROW_UP, SEG_OFF);
  }
  break;
 case DISPLAY_LINE_CLEAR:
  bmp_as_stop();

  // Menu item is not visible
  sCounter.state = MENU_ITEM_NOT_VISIBLE;
  // Clear function-specific symbols
  display_symbol(LCD_SYMB_ARROW_UP, SEG_OFF);
  display_symbol(LCD_SYMB_ARROW_DOWN, SEG_OFF);
  break;
 }
}
u8 is_counter_measurement(void)
{
   return sCounter.state == MENU_ITEM_VISIBLE;
}
#define WALK_COUNT_THRESHOLD  400
#define RUN_COUNT_THRESHOLD  300
void do_count(void)
{
 u16 threshold = (sCounter.style ? RUN_COUNT_THRESHOLD: WALK_COUNT_THRESHOLD );
 if (( sCounter.high - sCounter.low) > threshold ) {
  sCounter.count ++;
  display.flag.update_counter = 1;
 }
}
void do_counter_measurement(void)
{
 extern u16 convert_acceleration_value_to_mgrav(u8 value);
 u8 i;
 u16 accel_data, sum1;
 // Get data from sensor
 bmp_as_get_data(sCounter.xyz);
 for ( i = 0, sum1 = 0; i < 3; i ++ ) {
  accel_data = convert_acceleration_value_to_mgrav(sCounter.xyz[i]);
  // Filter acceleration
#if 0
  accel_data = (u16)((accel_data * 0.2) + (sCounter.data[i] * 0.8));
#endif
  accel_data = (u16)((accel_data * 0.1) + (sCounter.data[i] * 0.9));
  sum1 += accel_data;
  // Store average acceleration
  sCounter.data[i] = accel_data;
 }
 if ( sCounter.sum == 0 ) {
     sCounter.low = sum1;
     sCounter.high = sum1;
 } else {
    switch ( sCounter.rise_state) { // init state
    case 0:
        if ( sum1 > sCounter.sum ) {
            sCounter.rise_state = 1;
            sCounter.low = sCounter.sum;
        } else {
            sCounter.rise_state = 2;
            sCounter.high = sCounter.sum;
        }
        break;
    case 1:
    if ( sum1 < sCounter.sum ) { // change direction
       sCounter.high = sCounter.sum;
       sCounter.rise_state = 2;
    }
    break;
    case 2:
        if ( sum1 > sCounter.sum ) {
           sCounter.low = sCounter.sum;
           sCounter.rise_state = 1;
              do_count();
        }
        break;
    }
    }
 sCounter.sum = sum1;
 // Set display update flag
// display.flag.update_counter = 1;
}
