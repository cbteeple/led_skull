# led_skull
A sweet RGB LED skull project for Halloween!

This code is a bit of a hack job based on the code for my [LED Drink Coasters](https://github.com/cbteeple/LED-Coaster)

## Dependencies

### Tools / Supplies
- Soldering iron + solder
- Hot glue gun + hot glue
- 3D printer (for the spikes)

### Hardware
- Cardboard skull mask [Designed by Dali Lomo](https://dali-lomo.blogspot.com/2015/10/express-halloween-diy-cardboard-skull.html)
- Electronics
  - Individually-adressable RGB LEDs, you can find strips of them pretty cheap on [Amazon](https://www.amazon.com/s?k=addressable+rgb+led+strips+1m), and even cheaper on [Ali-Express](https://www.aliexpress.com/item/33015096252.html?spm=a2g0o.productlist.0.0.55084e13q7J5BI&algo_pvid=27a48be5-a9ef-488e-9eb3-445130077884&algo_expid=27a48be5-a9ef-488e-9eb3-445130077884-5&btsid=83aeec9e-7e1b-49e1-a086-d7d23edd4a0c&ws_ab_test=searchweb0_0,searchweb201602_6,searchweb201603_52) (with questionable quality)
  - Arduino Nano (or some other small arduino-capable MCU)
  - Some small-gauge ribbon cable
  - A small button
  - A small USB power bank

### Software
- Various Arduino libraries
    - [Adafruit NeoPixel](https://github.com/adafruit/Adafruit_NeoPixel) by adafruit
    - [EEPROMAnything](https://github.com/collin80/EEPROMAnything) by collin80

## Installation
1. Build the cardbaord skull and slightly modify it so the back of the head is more open
2. Print some cool spikes
3. Add LEDs to the inside of the forehead, chin, and inside the spikes.
4. Wire it up to the arduino
    - _Eyes and Mouth_: Forehead and Chin strand of LEDS goes on pin 6
    - _Hair_: Spike strand of LEDs goes on pin 5
    - _Power for LEDs_: connect to the 5V and ground pins 
    - _Button_: connect between pin 2 and ground
5. Upload the code
6. Enjoy your new rave skull mask!

## Usage
### Light Modes
- Static Color (head & hair can be different colors)
- Pulsing/Breathing (head & hair can be differnt colors)
- Robocop (head & hair can be differnt colors)
- Smooth rainbow color wheel (all LEDs in sync)
- Choppy rainbow color wheel (colors update in a cool pulse from the back of the hair to the front)
- Rave spikes!!! (Fast smooth color wheel from back to front!)

### Change Brightness
1. Press and hold the button longer than 1 sec + release
2. The whole skull will blink white
3. Use the button to increment the brightness. Possible settings include:
    - [8%, 18%, 25%, 50%, 75%, 100%]
4. Wait for 1.5 sec and the program will resume at that brightness
