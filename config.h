#ifndef CONFIG_H
#define CONFIG_H
// lcd device address (0x27/0x3f depending on component)
const int LCD_ADDR = 0x27;  // unused currently

const int BUTTON = 1;
const int FAN = 2;
const int SDA_PIN = 4;
const int SCL_PIN = 5;

const int TOP_ROW = 0;
const int BOTTOM_ROW = 1; 

const int LAZY_DELAY = 250; // replace with interrupt
#endif
