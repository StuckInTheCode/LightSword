#pragma once

//defined parameters
#define NUM_LEDS 20         // number of leds in the strip
#define BRIGHTNESS 200      // maximum
#include "FastLED.h"        // library for the strip
#include <EEPROM.h>         // memory

class Light
{
public:
	// arrray of leds
	CRGB leds[NUM_LEDS];
	byte nowNumber;
	byte LEDcolor;
	byte curColor, red, green, blue, redOffset, greenOffset, blueOffset;
	boolean lightFlag;						//effect enable
	unsigned long effect_timer;             //time of light effect

	Light();
	~Light();
	void setMaxBrightness();
	//show voltage as leds
	void showVoltage(byte capacity);

	void lightEffect(boolean swordState);

	void setPixel(int index, byte red, byte green, byte blue);
	void setRGB();

	// set all pixels to the inputed color
	void setAll(byte red, byte green, byte blue);
	//animation of power on
	void lightOn();
	// animation of power off
	void lightOff();

	void setColor(byte c);

};

