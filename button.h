#ifndef BUTTON_H
#define BUTTON_H

struct button_t {
  const unsigned int pin;
  constexpr button_t(const unsigned int &pin) : pin{ pin } {
    gpio_init(pin);
    gpio_set_dir(pin, 0);
    gpio_pull_up(pin); 
  };
};

const bool button_peek(const button_t &button) { return gpio_get(button.pin); };

#endif
