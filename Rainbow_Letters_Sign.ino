////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/// Fast Adressable Bitbang LED Library Sample Program
/// To use, install this version or newer of FABLED:
/// https://github.com/sonyhome/FAB_LED/tree/b8a89fcf4ed4840a5a6e2b9ecd795a28ff1582cc
/// Copyright (c)2017 Dan Truong
///
/// You may use any part of this code, at the condition that you reference
/// my work, acknowledge me and advertise clearly FAB_LED and where to get it.
///
////////////////////////////
/// Rainbow Letters Sign ///
////////////////////////////
///
/// This program defines a 3 letter sign built with WS2811 LEDs (three 5050 LEDs
/// per bulb, 30mm), the number of LEDs for each letter is defined in a table.
///
/// The sign has 3 modes it rotates through:
///
/// * Fuse: a white pixel moves across the letters and explodes in a flash at
///   the end. This allocates only a black and a white pixel in memory (6 bytes)
/// * Color cycle: uses color wheels methods to cycle each letter through hues.
///   Because RGB color rainbows are boring, I use both a RGB and a CYM color
///   wheel. Furthermore I change randomly between each. This uses 1 pixel per
///   letter plus a bit more, for 5 bytes per letters (15 bytes total). Oh also
///   the color rotation speed is randomised to make the letters out of sync.
///   Each letter's pattern is independent of the others. You will see the color
///   change, but won't be able to tell what colors it goes through, the colors
///   will also be different from the ones seen in a simple RGB color wheel.
/// * random color: the letters are colored with random colors. To make this fun
///   the number of pixels updated each cycle increases over time up to the
///   point where they all change, and then flash. Also as the rate of change
///   increases, the duration decreases. Since colors are random, this mode
///   uses an array of pixels, one per LED (104) for a total of 312 bytes. This
///   is the traditional method Adafruit and FastLED use, but limits the number
///   of LEDs that can be driven.
/// 
/// All these routines have some extra code hacked in them to handle the
/// transition between each mode to make it less abrupt. It's dirty and not very
/// good transitions either, but whatever. :)
///
/// This code demonstrates an actual use of FAB_LED to drive a 7ft by 4.5ft sign
/// using 104 0.78W/12V LEDs. The controller is an AtTiny85 using port B4 (pin3)
/// It is connected to the LED power via a buck converter, and drives the LED
/// data pin without any other component. The LEDs are WS2811 30mm bulbs.
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <FAB_LED.h>

////////////////////////////////////////////////////////////////////////////////
/// @brief
/// Helper routine to calculate the next color for a rainbow effect
/// Allows the wheel to run between minBright and maxBright.
/// If minBright != 0, then the first pixel must have all color values to
/// minBright except one which has maxBright.
/// Note: we force gcc to inline to meet the timing constraints of the LED strip
////////////////////////////////////////////////////////////////////////////////
static void colorWheel(uint8_t incStep, uint8_t & R, uint8_t & G, uint8_t & B, uint8_t M, uint8_t m) __attribute__ ((always_inline));
static inline void colorWheel(uint8_t incStep, uint8_t & R, uint8_t & G, uint8_t & B, uint8_t maxBright = 255, uint8_t minBright = 0)
{
  const uint8_t maxB = maxBright - minBright;
  const uint8_t Rin = R - minBright;
  const uint8_t Gin = G - minBright;
  const uint8_t Bin = B - minBright;

  // Avoid getting stuck
  if (Bin != 0 && Gin != 0 && Rin != 0)
  {
    if (Bin < Gin)
    {
      if (Rin < Bin)
        R--;
      else
        B--;
    }
    else if (Rin < Gin)
    {
      if (Rin < Bin)
        R--;
      else
        B--;
    }
    else // (Gin < Rin or Bin)
    {
      G--;
    }
    return;
  }
  
  if (Bin == 0 && Rin != 0) {
    R = (Rin <= incStep)      ? minBright : (Rin - incStep + minBright);
    G = (Gin >= maxB-incStep) ? maxBright : (Gin + incStep + minBright);
    return;
  }
  if (Rin == 0 && Gin != 0) {
    G = (Gin <= incStep)      ? minBright : (Gin - incStep + minBright);
    B = (Bin >= maxB-incStep) ? maxBright : (Bin + incStep + minBright);
    return;
  }
  if (Gin == 0 && Bin != 0) {
    B = (Bin <= incStep)      ? minBright : (Bin - incStep + minBright);
    R = (Rin >= maxB-incStep) ? maxBright : (Rin + incStep + minBright);
    return;
  }
}

static void cymColorWheel(uint8_t incStep, uint8_t & R, uint8_t & G, uint8_t & B, uint8_t M, uint8_t m) __attribute__ ((always_inline));
static inline void cymColorWheel(uint8_t incStep, uint8_t & R, uint8_t & G, uint8_t & B, uint8_t maxBright = 255, uint8_t minBright = 0)
{
  const uint8_t maxB = maxBright - minBright;
  const uint8_t Rin = R - minBright;
  const uint8_t Gin = G - minBright;
  const uint8_t Bin = B - minBright;

  // Avoid getting stuck
  if (Bin != maxB && Gin != maxB && Rin != maxB)
  {
    if (Bin > Gin)
    {
      if (Rin > Bin)
        R++;
      else
        B++;
    }
    else if (Rin > Gin)
    {
      if (Rin > Bin)
        R++;
      else
        B++;
    }
    else // (Gin > Rin or Bin)
    {
      G++;
    }
    return;
  }

  // Normal wheelcolor
  if (Bin == maxB && Gin != maxB) {
    R = (Rin <= incStep)      ? minBright : (Rin - incStep + minBright);
    G = (Gin >= maxB-incStep) ? maxBright : (Gin + incStep + minBright);
    return;
  }
  if (Rin == maxB && Bin != maxB) {
    G = (Gin <= incStep)      ? minBright : (Gin - incStep + minBright);
    B = (Bin >= maxB-incStep) ? maxBright : (Bin + incStep + minBright);
    return;
  }
  if (Gin == maxB && Rin != maxB) {
    B = (Bin <= incStep)      ? minBright : (Bin - incStep + minBright);
    R = (Rin >= maxB-incStep) ? maxBright : (Rin + incStep + minBright);
    return;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Global variables
////////////////////////////////////////////////////////////////////////////////
static const uint8_t numLetters = 3;
static const uint8_t letter[numLetters] = {26,36,42};
//static const uint8_t numPixels = letter[0] + letter[1] + letter[2]; // 104
#define NUM_PIXELS 104
ws2811<B,4> myLeds;
#define PIX rgb
PIX pixels[NUM_PIXELS] = {};

////////////////////////////////////////////////////////////////////////////////
/// @brief Cycle the rainbow on each letter, but each letter at a different
/// offset of the rainbow, and sometimes at a different speed.
////////////////////////////////////////////////////////////////////////////////
static inline bool rainbowLetters(uint8_t brightness)
{
  const uint16_t timeOutDefault = 8000;
  static uint16_t timeOut = timeOutDefault;
  static uint8_t inc[numLetters] = {1,1,1};
  static PIX pix[numLetters] ={};
  static bool cymk[numLetters] = {false, false, false};

  // Initialize the colors of each letter once at 1st call
  static bool init = false;
  if (!init)
  {
    init = true;
    pix[0].g = brightness;
    pix[1].r = brightness;
    pix[2].b = brightness;
  }

  // Display the LEDs
  myLeds.begin();
  for (uint8_t l = 0; l < numLetters ; l++) {
    const uint8_t numPixels = letter[l];
    for (uint8_t i = 0; i < numPixels; i++) {
      myLeds.send(1, &pix[l]);
    }
   }
  myLeds.end();

  // Rotate the colors based on the pixel's previous color.
  for (uint8_t l = 0; l < numLetters ; l++) {
    if (cymk[l])
      cymColorWheel(inc[l], pix[l].r, pix[l].g, pix[l].b);
    else // rgb
      colorWheel(inc[l], pix[l].r, pix[l].g, pix[l].b);
  }

  // Change the color rotation randomly
  uint8_t r = random(255);
  if (r % (1<<4) == 1)
  {
    r >>= 4;
    inc[r % numLetters] = (r>>2);
    if (r % 4 == 3)
      cymk[r % numLetters] = !cymk[r % numLetters];
  }

  if (--timeOut == 0)
  {
    // Copy the last color to the array for the next mode
    uint8_t base = 0;
    for (uint8_t l = 0; l < numLetters ; l++) {
      const uint8_t numPixels = letter[l];
      for (uint8_t i = 0; i < numPixels; i++) {
        pixels[base+l] = pix[l];
      }
      base += numPixels;
    }
    timeOut = timeOutDefault;
    return true;
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Fuse()
/// A single pixel runs across all LEDs, then flash 100% white and fades.
/// flash maybe from both ends travelling back to middle at max speed before the fade?
static inline bool fuse(uint8_t brightness)
{
  static uint8_t index = 0;
  const PIX black = {0,0,0};
  static PIX white = {brightness,brightness,brightness};

  if (index < NUM_PIXELS)
  {
    // Move the fuse
    myLeds.begin();
    for (uint8_t i = 0; i < NUM_PIXELS; i++)
    {
      if (i != index)
      {
        myLeds.send(1, &black);
      }
      else
      {
        myLeds.send(1, &white);
      }
    }
    myLeds.end();
    index++;
  }
  else
  {
    // end of sequence flash
    myLeds.begin();
    for (uint8_t i = 0; i < NUM_PIXELS; i++)
    {
        myLeds.send(1, &white);
    }    
    myLeds.end();

    // handle flash decay
    if (white.r != 0)
    {
      white.r >>= 1;
      white.g >>= 1;
      white.b >>= 1;
    }
    else
    {
      // Prepare next iteration
      white = {brightness,brightness,brightness};
      index = 0;
      delay(500);
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// Random color
/// Change one pixel to a random color, repeat.
// Increase the # of pixels changes per cycle till all are changed
// when all are, flash white/black very fast.
////////////////////////////////////////////////////////////////////////////////
static inline bool randomColors(uint8_t brightness)
{
  static uint8_t iter = 0;
  static uint8_t numChanged = 99;
  if (++iter < NUM_PIXELS-numChanged+1)
  {
    for (uint8_t i = 0; i < numChanged; i++)
    {
      uint8_t p = random(NUM_PIXELS);
      pixels[p].r = random(brightness);
      pixels[p].g = random(brightness);
      pixels[p].b = random(brightness);
    }
    myLeds.draw(NUM_PIXELS, pixels);
  }
  else if (numChanged < NUM_PIXELS+3)
  {
    //myLeds.grey(NUM_PIXELS,0);
    //delay(5);
    numChanged += 3;
    iter = 0;
  }
  else // numChanged >= NUM_PIXELS
  {
    // Flash
    for (int i = 0; i < 3; i++)
    {
      myLeds.grey(NUM_PIXELS,0);
      myLeds.grey(NUM_PIXELS,255);
      delay(2);
    }
    // Go back to as if nothing happened
    myLeds.draw(NUM_PIXELS, pixels);
    pixels[0] = {64,64,64};
    // Then fade to zero except fuse pixel
    for (uint8_t i = 0; i < 8; i++)
    {
      for (uint8_t p = 1; p < NUM_PIXELS; p++)
      {
        pixels[p].g >>= 1;
      }      
      myLeds.draw(104, pixels);
      delay(90);
    }
    for (uint8_t i = 0; i < 8; i++)
    {
      for (uint8_t p = 1; p < NUM_PIXELS; p++)
      {
        pixels[p].b >>= 1;
      }      
      myLeds.draw(NUM_PIXELS, pixels);
      delay(70);
    }
    for (uint8_t i = 0; i < 8; i++)
    {
      for (uint8_t p = 1; p < NUM_PIXELS; p++)
      {
        pixels[p].r >>= 1;
      }      
      myLeds.draw(NUM_PIXELS, pixels);
      delay(90);
    }
    delay(1000);
    // pixels is zeroed out now.
    iter = 0;
    numChanged = 3;
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @letterFill()
/// Fill each letter with its color

////////////////////////////////////////////////////////////////////////////////
/// @brief This method is automatically called once when the board boots.
////////////////////////////////////////////////////////////////////////////////
void setup()
{
	// Turn off first 1000 LEDs
  myLeds.clear(1000);
  myLeds.grey(1, 64);
  delay(1000);
  //myLeds.clear(10);

	// Configure a strobe signal to Port B5 for people who
	// use oscilloscopes to look at the signal sent to the LEDs
	// Port B5 corresponds to the Arduino Uno pin13 (LED).
	//DDRB |= 1U << 5;
	//PORTB &= ~(1U << 5);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief This method is automatically called repeatedly after setup() has run.
/// It is the main loop, and it calls all the other demo methods declared below.
////////////////////////////////////////////////////////////////////////////////
void loop()
{
  // 0 : fuse to
  // 1 : rainbow letters to
  // 2 : random colors
  static uint8_t mode = 2;

  switch (mode)
  {
  case 0:
    mode = (fuse(255)) ? 1 : 0;
    break;
  case 1:
    mode = (rainbowLetters(2)) ? 2 : 1;
    break;
  case 2:
    mode = (randomColors(90)) ? 0 : 2;
    break;
  }

//  delay(2);
}
