#ifndef THERMO_H
#define THERMO_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include <cmath>

const auto get_temp(const double &Vo) -> double {
  const double Vi = 3.3;
  const double r25 = 10000.0;
  const double A = 1.106836861e-3;
  const double B = 2.384641754e-4;
  const double C = 0.6507394466e-7;
  
  const double rt = (Vo * r25) / (Vi - Vo);

  const double kelvin =  1 / (A + (B * std::log(rt)) + C * std::pow(std::log(rt), 3));
  const double celcius = kelvin - 273.15;
  return celcius;
};

const auto get_vout() -> double {
  const double conversion_factor = 3.3f / (1 << 12);
  const uint16_t in = adc_read();
  const double Vout = in * conversion_factor;  
  return Vout;

const auto adc26_init() -> void {
  adc_init();
  adc_gpio_init(26);
  adc_select_input(0);
};
#endif
