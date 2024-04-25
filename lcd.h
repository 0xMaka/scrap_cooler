/*
  #@title  : maka_lcd
  #@author : maka
  #@notice : Custom driver with some abstractions. 
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"

#include <string_view>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
#include "config.h"

// commands
// --------- - - - - - - - - - - -- -
const int CLEARDISPLAY        = 0x01;
const int RETURNHOME          = 0x02;
const int ENTRYMODESET        = 0x04;
const int DISPLAYCONTROL      = 0x08;
const int CURSORSHIFT         = 0x10;
const int FUNCTIONSET         = 0x20;
const int SETCGRAMADDR        = 0x40;
const int SETDDRAMADDR        = 0x80;

// display entry mode
const int ENTRYSHIFTINCREMENT = 0x01;
const int ENTRYLEFT           = 0x02;

// display and cursor control
const int BLINKON             = 0x01;
const int CURSORON            = 0x02;
const int DISPLAYON           = 0x04;

// display and cursor shift
const int MOVERIGHT           = 0x04;
const int DISPLAYMOVE         = 0x08;

// function set
const int _5x10DOTS            = 0x04;
const int _2LINE               = 0x08;
const int _8BITMODE            = 0x10;

// backlight control
const int BACKLIGHT           = 0x08;

// enable latch
const int ENABLE_BIT          = 0x04;

// device address (0x27/0x3f depending on component)
static int addr = 0x27;

// some constants
#define DATA       1
#define COMMAND    0
#define MAX_LINES  2
#define MAX_CHARS  16

// abstractions
//-- - - - -----
// --------- - - - - - - - - - - -- -
const auto write = [] (const auto &&src, const auto len, const auto nostop) -> int {
  return i2c_write_blocking(i2c_default, addr, src, len, nostop);
};

const auto read = [] (auto &&dst, const auto len, const auto nostop) -> int {
  return i2c_read_blocking(i2c_default, addr, dst, len, nostop);
};

//-- - -
auto write_byte(uint8_t val) -> void {
  write(&val, 1, false);
};

auto toggle_enable(uint8_t val) -> void {
  sleep_us(500); write_byte(val |  ENABLE_BIT);
  sleep_us(500); write_byte(val & ~ENABLE_BIT);
  sleep_us(500);
};

auto send_byte(uint8_t val, int reg) -> void {
  uint8_t high = reg | (val & 0xF0)        | BACKLIGHT;
  uint8_t low  = reg | ((val << 4) & 0xF0) | BACKLIGHT;
  write_byte(high); toggle_enable(high);
  write_byte(low);  toggle_enable(low);
};

const auto command = [] (const uint8_t x) -> void { send_byte(x, COMMAND); };
const auto data    = [] (const uint8_t x) -> void { send_byte(x, DATA); };

const auto clear()    -> void { command(CLEARDISPLAY); };
const auto home()     -> void { command(RETURNHOME); };
const auto on()       -> void { command(DISPLAYCONTROL | DISPLAYON); };
const auto off()      -> void { command(DISPLAYCONTROL &~ DISPLAYON); };
const auto set_line() -> void { command(FUNCTIONSET | _2LINE); };
const auto set_left() -> void { command(ENTRYMODESET | ENTRYLEFT); };
const auto set_pos(
  int line, 
  int pos
)               -> void { command((line == 0) ? pos + 0x80 : pos + 0xC0); };
auto reset() -> void { clear(); set_left() ;};

const auto put_char(const char x) -> void { data(x) ;};

auto print_cstr(const char *xs) -> void {
  std::ranges::for_each(std::string_view(xs), put_char);
};

auto svc = [] (const std::string &s) { std::vector<char> v { s.begin(), s.end() }; return v; };
auto print_str(const std::string &xs) { std::ranges::for_each(svc(xs), put_char); };

constexpr int indent(const std::string &str) {
  return std::round((double)(16 - std::size(str))/2);
};

auto put_mid(const std::string str, const int row) -> void {
  int ctr = indent(str);
  set_pos(row, ctr);
  print_str(str);
};

auto get_pairs(const std::vector<std::string> &lines) {
  std::vector<std::pair<std::string,std::string>> pairs;
  const int len = std::size(lines);
  const auto aux = [&] (auto &&aux, const int idx) {
    switch (idx == len) {
      case 1 : return pairs;
      case 0 : 
        pairs.push_back(std::pair{ lines[idx],lines[idx+1] }); 
        return aux(aux, idx+2);
    };
  }; return aux(aux,0);
};

auto custom_char(const std::vector<uint8_t> &pattern, const int slot) -> void{
  const int _slot = slot & 0x07;
  command(SETCGRAMADDR | _slot << 3);
  std::ranges::for_each(pattern, put_char);
};

// --------- - - - - - - - - - - -- -

void lcd_init() {
  command(0x03);
  command(0x03);
  command(0x03);
  command(0x02);
  command(ENTRYMODESET | ENTRYLEFT);
  command(FUNCTIONSET | _2LINE);
  command(DISPLAYCONTROL | DISPLAYON);
  clear();
}

auto lcd_quick_init() -> void {
  i2c_init(i2c_default, 100 * 1000);
  gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_PIN);
  gpio_pull_up(SCL_PIN);
  lcd_init();
};

// --------- - - - - - - - - - - -- -

