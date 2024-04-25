/*
  #@title  : nuc_fan
  #@author : maka
  #@notice : Project to utilise an old 4 pin pc fan, rehoused in a gutted Corsair power supply.
  Build uses a bidirectional logic converter for interfacing between the 3.3v mcu and the 5v lcd.
  A voltage booster is used to drive the fan at 12v. Features a thermal resitor to take temprature readings,
  a button input for switching between 3 fan speeds, providing feedback on the lcd. 

  Uses the standard library/c++ sdk, but no 3rd party libraries.
*/

#include <stdio.h>
#include <format>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include "config.h"
#include "lcd.h"
#include "button.h"
#include "pwm.h"
#include "thermo.h"

const std::vector<std::string> comments{ 
  "low speed", "medium speed", "high speed"
};

const auto thermo_string() -> std::string {
  return std::string{ std::format("{:.1f}", get_temp()) + "C" };
};

const auto process_state(const button_t &button, const pwm_t &pwm, const int &curr) -> void {
  switch(!button_peek(button)) {
    case 1 : { 
      const auto next = curr >= 2 ? low : curr + 1;
      switch_state(pwm, next); 
      busy_wait_ms(LAZY_DELAY); clear();
      process_state(button, pwm, next);
    }
    case 0 :
      put_mid(comments[curr], TOP_ROW);
      put_mid(thermo_string(), BOTTOM_ROW);
      busy_wait_ms(LAZY_DELAY); 
      process_state(button, pwm, curr);
  }
};

auto main() -> int {
  stdio_init_all();
  lcd_quick_init();
  adc26_init();

  put_mid("Makas NUC Node",0);
  sleep_ms(100);
  clear();

  const pwm_t pwm_fan{ FAN }; 
  pwm_enable(pwm_fan);
  pwm_wrap_and_compare(pwm_fan, 500, 250);
  const button_t button{ BUTTON };
  
  process_state(button, pwm_fan, low);

  return 0;
};
