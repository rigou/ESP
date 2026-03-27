/* rgTimer.h
** 08-01-2012
*/
#ifndef rgTimer_h
#define rgTimer_h
#include <Arduino.h>

long Timer(int id_int, int command_int, long delay_ulng);

const int TIMER_RESET=0;
const int TIMER_GET=1;
const int TIMER_SET=2;
const int TIMER_COUNT=5;

#define Expired(x)  (Timer((x), TIMER_GET, 0) >= 0)

#endif
