#include <Arduino.h>
#include "Timer.h"

Timer::Timer() {
  this->interval_end_ms = 0L;
}

Timer::Timer(int ms) {
  countdown_ms(ms);
}

bool Timer::expired() {
  return (interval_end_ms > 0L) && (millis() >= interval_end_ms);
}

void Timer::countdown_ms(unsigned long ms) {
  interval_end_ms = millis() + ms;
}

void Timer::countdown(int seconds) {
  countdown_ms((unsigned long)seconds * 1000L);
}

int Timer::left_ms() {
  return interval_end_ms - millis();
}
