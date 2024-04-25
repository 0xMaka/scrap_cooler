> Under Construction | Fixtures need to be made more permanent, code needs to be recomposed now the components are done, and some features want to be added. 
> But it works pretty effectively and I am getting busy with something for a protocol so who knows what gets done.
> Throwing it out there in case it can be of use to others. 
> Released under GPL3 - Don't hurt yourself. 

## Maka's Scrap Cooler - A custom fan enclosure for an Ethereum Nuc node
--- 
- Housed in the repurposed shell of a Corsair CX500M power supply
- Made with an old 4 pin PC fan, driven at 12 volts via a voltage booster
- Controlled via PWM from a Pico driven at 3.3 volts
- Feedback is given through a 2 line LCD, driven at 5 volts. Connected via a bidirectional logic-level shifter for reliable communication with the MCU
- Build uses a thermistor to take temperature readings of the enclosure, and features 3 speeds selected via a push button

Uses the Pico C++ SDK but there are no 3rd party libraries.

<table class="fixed-align">
<tbody>
  <tr>
<td>
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/25884ae2-4687-4d3e-8d6b-a57fe817fa40'/>
</td>
<td>     
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/c8db2f3a-e734-41c6-9e85-e2b1c3319f4e'/>
</td>
  </tr>
  <tr>
<td>
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/cf584c35-be00-4f18-815b-4c30b8f37e6b'/> 
</td>
<td>
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/403c242f-6d0c-42d9-8265-88a833c3c84c'/>
</td>
  </tr>
</tbody>
</table>

---

### Build Notes:

#### Thermistors

The most common ways to measure temperature using a thermistor are:
- Use a lookup table provided by the manufacturer, hard code it and pull the nearest value from the table when reading a voltage.
- Obtain resistances at 3 temperatures of interest `r1,r2,r3`, `t1,t2,t3` and use them to calculate the *Steinhart-Hart model coefficients*, then with the voltage measured across the thermistor and the *Steinhart-Hart equation*, get the temperature in kelvin and convert from there.


> **Note**:
Measure the thermistor while unpowered at three different temperatures. 
Temperatures should be evenly spaced and at least 10 degrees apart.

The coefficients shouldn't really be copied as they are specific to a given manufacturing process and material.
If not having access to a datasheet or just wanting more accurate readings, we can take our own at different temperatures and use those.


The Steinhart-Heart equation:

$$
T = 1 / A+B ln(R) + C [ln(R)]^3
$$

Determining the 3 reference temperatures:

$$
1/T1=A+B ln(R_1) + C[ln(R_1)]3 \\
$$

$$
1/T2=A+B ln(R_2) + C[ln(R_2)]3
$$

$$
1/T3= A + B ln(R_3) + C[ln(R_3)]3
$$

Solving for coefficients:

$$
C=\frac{X_3 (Y_1-Y_2) - X_1(Y_1-Y_2) + Y_1 X_1 - Y_1 X_2 - Y_3 X_1 + Y_3X_2}
{({y_3})^{3} (y_1-Y_2) - (y_1)(y_2)^3 + (y_1)^4 - (y_1)^3(y_1−y_2) + (Y_3)(Y_2)^3 - (Y_3)(Y_1)^3}
$$

$$
B = \frac{(x_1-x_2) + C(y_2)^3-C(y_1)^3}{(Y_1-Y_2)}
$$

$$
A= x_2-By_2-C(y_2)^3
$$

Where:
<table class="fixed-align">
<tbody>
  <tr>
<td>

$X_1 = 1/Tlow $ 
<br>
$X_2 = 1/Tmid$
<br>
$Y_1 = InRTIN$
<br>
$Y_2 = InRT$
<br>
$X_3 = 1/Thigh$
<br>
$Y_3 = In RThigh$

</td>
<td>     

$T_i = Kelvin Temperature = t_i (°C) + 273.15$
<br>
$Tlow  = Low temperature calibration point $
<br>
$Tmid  = Mid temperature calibration point $
<br>
$Thigh = High temperature calibration point$
<br>
$R_1 = Resistance in ohms at temperature T_i$

</td>
  </tr>
</tbody>
</table>

- https://ew.lens.unifi.it/notes/About_Steinhart-Hart_Equation.pdf

---

My math is my embarrassment so don't be put off, it isn't as heavy as it looks and we can offload the heaviest of it.
We can get the temperature in kelvin by summing the coefficients A, B times base 10 log of *rt*, and C times base 10 log *rt*^3
```cpp
  // ...  
  const double r25 = 10000.0;  // resistance at 25 degrees C
 //...
  const double rt = (Vo * r25) / (Vi - Vo);  //temp in terms of resitance
  const double kelvin =  1 / (A + (B * std::log(rt)) + C * std::pow(std::log(rt), 3));
```

For an initial set up we can use the values from the datasheet with the calculator from Stanford Research Systems, to obtain the coefficients:
![image](https://github.com/0xMaka/scrap_cooler/assets/12489182/7f71be1f-cc1d-4b63-8341-242db17006fe)
- https://www.thinksrs.com/downloads/programs/Therm%20Calc/NTCCalibrator/NTCcalculator.htm
- https://www.eaa.net.au/PDF/Hitech/MF52type.pdf

Our standard implementation:
```cpp
const auto get_temp(const double &Vo) -> double {
  const double Vi = 3.3;
  const double r25 = 10000.0;
  const double A = 1.106836861e-3;
  const double B = 2.384641754e-4;
  const double C = 0.6507394466e-7;

  const double rt = (Vo * r25) / (Vi - Vo);

  const double kelvin =  1 / (A + (B * std::log(rt)) + C * std::pow(std::log(rt), 3));
  const double celsius = kelvin - 273.15;
  return celsius;
};
```

##### ADC 
To measure a non binary state on the pin we'll need to use an analogue to digital converter of the MCU, and to read the voltage level across the thermistor we'll need to hook up a voltage divider.

![image](https://github.com/0xMaka/scrap_cooler/assets/12489182/7eda184c-6fed-4b67-ab3d-e0692eec985f)

This is a simple circuit that uses a resistor with the same resistance as the thermistor.
For an NPC thermistor the connection goes thermistor to ground, thermistor to register, resistor to the reference voltage. The line to the ADC sits between the thermistor and the resistor.

---
#### 4 Wire Fans

![image](https://github.com/0xMaka/scrap_cooler/assets/12489182/764a59cb-eb26-476f-b936-cd063e9e7d64)

- Tachometer 
Provides two pulses per revolution, and is a way to take a measurement from the fan.
This line needs to be pulled up to 12 volts to read the PWM and isn't really necessary for us.
- (PWM Pin) 
This is the input for PWM pulses and how we will control the speed of the fan.

Main factors to note:
- Base frequency is 25kHz and is acceptable from 21kHz to 28kHz
- Input has a TTL (Transistor-transistor logic) level and includes pull-up resistors to 5V or to 3.3V. 
- - The signal is not inverted so a 100% duty cycle equates to max revolutions of the fan. 
- Disconnected PWM signal will run fan at max revolutions.
  


http://www.pavouk.org/hw/fan/en_fan4wire.html 

---

#### PWM
PWM is probably one of the simpler interface methods and if you've ever built any kind of robot it should be familiar.

Essentially we'll have a `duty cycle` representing the percentage of time that the line will be held high or low for a given period.
Here we are duty high so by holding the line high for a greater percentage of time we will spin the fan faster.

The *period of a pulse* is defined as the sum of active and idle time.

$$
{t_{active} + t_{idle}}
$$

The frequency of the signal is the rate of switching:

$$
f = \frac{1}{T}
$$

The duty cycle is the ratio between the active time and the period:

$$
D = \frac{t_{active}}{T}= \frac{t_{active}}{t_{active} + t_{idle}}
$$

> **PICO Note**: The Pico has 8 PWM slices, each slice has 2 channels.
Channel A is always on an even numbered pin, channel B is always on an odd numbered pin. Only B channels can read input PWM. 

![image](https://github.com/0xMaka/scrap_cooler/assets/12489182/38812ca1-235a-43da-be75-1ec6e8414d89)

---
#### LCD

Have always loved these displays.
Key notes on the hardware side here are the use of I2C and the lack of shared voltage, yes the LCD will run at 3.3 volts but then the backlight is much lower. To get a nice bright display we want to run at the full 5 volts, requiring the use of 2 bi directional level shifters. 

This is for a couple of reasons, we want both the MCU and the LCD controller to be able to reliably interprate a high vs a low signal. It will depend on components and you can refer to the datasheet, but in some cases a device that is expecting a 5 volt high won't consider a line pulled to 3.3 volts to be a 1, in other cases it being beyond 2.5v will be enough. 

The other is that we'd rather not be feeding 5 volts into the pins of the MCU.

In a pinch we can build bi directional level shifter with two 2N7000 mosfets and four 10k pull up resistors.
- Connect gates to 3.3v, sources to pico sdc/scl, drains to lcd sda/scl.
- 1 pull up for each line, source side to 3.3v, drain to 5v.

![image](https://github.com/0xMaka/scrap_cooler/assets/12489182/81bc8e1b-bb1a-4b31-90a6-92062e9a6f25)


##### Display Types
Most common are the Hitachi 2 line displays, and depending on ROM will have 1 of 2 character sets:
- HD44780UA00, the standard (Japanese) version, includes katakana characters and some Greek letters and mathematical symbols
- HD44780UA02, a European version, includes the complete set of Greek, Cyrillic and Western European characters (with diacritics)

The Japanese version is far more prevalent, while the European version remains sort after as features many *novel* characters like currency symbols and arrows, a heart, bell or music note, freeing the CGROM for other custom characters.

<table class="fixed-align">
<tbody>
  <tr>
<td>
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/b1005b3f-d77a-469f-9656-f31898a05e20'/> 
</td>
<td>     
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/ac3a5bda-81f1-402a-968f-22c218848615'/>
</td>
  </tr>
</tbody>
</table>

##### Instructions
There is an instruction set that has 10 bits that can either be 1 or 0 depending on what we want to do.
The top 2 bits RS and R/W are for the Register Select and Read/Write Mode. In a vast majority of implementations there won't be a need to read and so the pin is often held low, but in some cases may want to read the busy flag, address counter or to extract a character.

The register select pin specifies if the lower byte should be interpreted as a command or a read/write access.

##### command register and a data register
Things like powering on/off, clearing the screen, placing the cursor are handled by setting bits in the command register. 

By loading the data register with the character bits we want to print, then clocking in the data and toggling an enable latch (like a shift register), we can print the characters to the screen.

- DDRAM | Display Data RAM
- -  Stores display data represented in 8-bit character codes
- CGRAM | Character Generator RAM 
- - writes custom characters (for 5 × 8 dots, eight character patterns can be written)

<table class="fixed-align">
<tbody>
  <tr>
<td>
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/67335d58-7b70-4c2f-bc8a-2b9ace04a9b0'/> 
</td>
<td>     
<img src='https://github.com/0xMaka/scrap_cooler/assets/12489182/f88314f6-aa1b-4469-a2d4-571df587fefb'/>
</td>
  </tr>
</tbody>
</table>
A common way to interface is in 4-bit mode, where the upper and lower nibbles of a byte are sent one after the other as most significant bits:

```cpp
auto send_byte(uint8_t val, int reg) -> void {
  uint8_t high = reg | (val & 0xF0)        | BACKLIGHT;
  uint8_t low  = reg | ((val << 4) & 0xF0) | BACKLIGHT;
  write_byte(high); toggle_enable(high);
  write_byte(low);  toggle_enable(low);
};
```
- https://www.datasheetq.com/pdf-view/HCD44780-Hitachi
---
... To be expanded.

> **Disclaimer** *Everything is only to the best of my understanding at the time, if you blow something up or get an electric shock it is on you. DYOR bro.*
