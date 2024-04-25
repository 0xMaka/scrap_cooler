#ifndef PWM_H
#define PWM_H

#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "button.h"

constexpr auto get_channel = [] (const auto &pin) { return pin % 2 == 0 ? PWM_CHAN_A : PWM_CHAN_B ;};

struct pwm_t {
  const unsigned int pin;
  const unsigned int slice;
  const unsigned int channel;
  constexpr pwm_t(const uint8_t &pin) : pin{ pin }, slice{ pwm_gpio_to_slice_num(pin) }, channel{ get_channel(pin) } { 
    gpio_set_function(pin, GPIO_FUNC_PWM);
  };
};

const auto pwm_enable(const pwm_t &pwm) -> void {
  pwm_set_enabled(pwm.slice, true);
};

const auto pwm_wrap_and_compare(const pwm_t &pwm, const int &wrap, const int &comp) -> void{
  pwm_set_wrap(pwm.slice, wrap);
  pwm_set_chan_level(pwm.slice, pwm.channel, comp);
};

constexpr auto lo = [] (const pwm_t &pwm) -> void { pwm_wrap_and_compare(pwm, 1000, 250) ;};
constexpr auto mi = [] (const pwm_t &pwm) -> void { pwm_wrap_and_compare(pwm, 1000, 500) ;};
constexpr auto hi = [] (const pwm_t &pwm) -> void { pwm_wrap_and_compare(pwm, 1000, 750) ;};

enum { low = 0, mid = 1, high = 2 }; unsigned int fan_speed;

const auto switch_state(const pwm_t &pwm, const int &speed) -> void {
  switch(speed) {
    case low  : lo(pwm); break; 
    case mid  : mi(pwm); break;
    case high : hi(pwm); break;
    default   : mi(pwm); break;
  }
}

#endif
