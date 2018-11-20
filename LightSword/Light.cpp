#include "Light.h"

Light::Light()
{
	FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
	FastLED.setBrightness(100);
	nowNumber = 0;
}


Light::~Light()
{
}

void Light::setMaxBrightness()
{
	FastLED.setBrightness(BRIGHTNESS);
}

void Light::showVoltage(byte capacity)
{
	setColor(nowNumber);
	byte capacityCount = map(capacity, 100, 0, (NUM_LEDS / 4 - 1), 1);  // calculate count of leds
	char i0 = 0;
	char i1 = NUM_LEDS / 2 - 1;
	char i2 = NUM_LEDS / 2;
	char i3 = NUM_LEDS - 1;
	for (i0, i1, i2, i3; i0 <= capacityCount; i0++, i1--, i2++, i3--) {
		//set every 4th led to the current color
		setPixel(i0, red, green, blue);
		setPixel(i1, red, green, blue);
		setPixel(i2, red, green, blue);
		setPixel(i3, red, green, blue);
		FastLED.show();
		delay(25);
	}

}
void Light::lightEffect(boolean swordState) {
	if (swordState && ((millis() - effect_timer) >= 50) & !lightFlag) {      // show 
		if (nowNumber == NUM_LEDS / 4)
		{
			nowNumber = 0;
			setPixel(NUM_LEDS / 4 - 1, red, green, blue);                   //set prev pixels to the main color
			setPixel(NUM_LEDS / 4, red, green, blue);
			setPixel((3 * NUM_LEDS) / 4 - 1, red, green, blue);
			setPixel((3 * NUM_LEDS) / 4, red, green, blue);
		}

		char i0 = nowNumber;
		char i1 = NUM_LEDS / 2 - nowNumber - 1;
		char i2 = NUM_LEDS / 2 + nowNumber;
		char i3 = NUM_LEDS - nowNumber - 1;

		if (nowNumber) {
			setPixel(i0 - 1, red, green, blue);                              //set prev pixels to the main color
			setPixel(i1 + 1, red, green, blue);
			setPixel(i2 - 1, red, green, blue);
			setPixel(i3 + 1, red, green, blue);
		}
		//set 3 next pixels to an another color  
		setPixel(i0, abs(red - 80), abs(green - 80), abs(blue - 80));
		setPixel(i1, abs(red - 80), abs(green - 80), abs(blue - 80));
		setPixel(i2, abs(red - 80), abs(green - 80), abs(blue - 80));
		setPixel(i3, abs(red - 80), abs(green - 80), abs(blue - 80));

		FastLED.show();
		nowNumber++;
	}
}


void Light::setPixel(int index, byte red, byte green, byte blue) {
	leds[index].r = red;
	leds[index].g = green;
	leds[index].b = blue;
}

void Light::setRGB() {
	char i0 = 0;
	char i1 = NUM_LEDS / 2 - 1;
	char i2 = NUM_LEDS / 2;
	char i3 = NUM_LEDS - 1;

	redOffset = 255;
	greenOffset = 0;
	blueOffset = 0;

	setPixel(i0++, redOffset, greenOffset, blueOffset);
	setPixel(i1--, redOffset, greenOffset, blueOffset);
	setPixel(i2++, redOffset, greenOffset, blueOffset);
	setPixel(i3--, redOffset, greenOffset, blueOffset);

	redOffset = 205;
	greenOffset = 0;
	blueOffset = 50;

	setPixel(i0++, redOffset, greenOffset, blueOffset);
	setPixel(i1--, redOffset, greenOffset, blueOffset);
	setPixel(i2++, redOffset, greenOffset, blueOffset);
	setPixel(i3--, redOffset, greenOffset, blueOffset);

	redOffset = 255;
	greenOffset = 0;
	blueOffset = 255;

	setPixel(i0++, redOffset, greenOffset, blueOffset);
	setPixel(i1--, redOffset, greenOffset, blueOffset);
	setPixel(i2++, redOffset, greenOffset, blueOffset);
	setPixel(i3--, redOffset, greenOffset, blueOffset);

	redOffset = 0;
	greenOffset = 0;
	blueOffset = 255;

	setPixel(i0++, redOffset, greenOffset, blueOffset);
	setPixel(i1--, redOffset, greenOffset, blueOffset);
	setPixel(i2++, redOffset, greenOffset, blueOffset);
	setPixel(i3--, redOffset, greenOffset, blueOffset);

	redOffset = 100;
	greenOffset = 255;
	blueOffset = 0;

	setPixel(i0, redOffset, greenOffset, blueOffset);
	setPixel(i1, redOffset, greenOffset, blueOffset);
	setPixel(i2, redOffset, greenOffset, blueOffset);
	setPixel(i3, redOffset, greenOffset, blueOffset);
	FastLED.show();
}

// set all pixels to the inputed color
void Light::setAll(byte red, byte green, byte blue) {
	for (int i = 0; i < NUM_LEDS; i++) {
		setPixel(i, red, green, blue);
	}
	FastLED.show();
}

//animation of power on
void Light::lightOn() {
	char i0 = 0;
	char i1 = NUM_LEDS / 2 - 1;
	char i2 = NUM_LEDS / 2;
	char i3 = NUM_LEDS - 1;
	for (i0, i1, i2, i3; i0 <= (NUM_LEDS / 4 - 1); i0++, i1--, i2++, i3--) {  //set every 4th led to the current color
		setPixel(i0, red, green, blue);
		setPixel(i1, red, green, blue);
		setPixel(i2, red, green, blue);
		setPixel(i3, red, green, blue);
		FastLED.show();
		delay(25);
	}
}


// animation of power off
void Light::lightOff() {
	char i0 = NUM_LEDS / 4 - 1;
	char i1 = NUM_LEDS / 4;
	char i2 = (NUM_LEDS * 3) / 4 - 1;
	char i3 = (NUM_LEDS * 3) / 4;
	for (i0, i1, i2, i3; i0 >= 0; i0--, i1++, i2--, i3++) {
		setPixel(i0, 0, 0, 0);
		setPixel(i1, 0, 0, 0);
		setPixel(i2, 0, 0, 0);
		setPixel(i3, 0, 0, 0);
		FastLED.show();
		delay(10);
	}
}


void Light::setColor(byte c) {
	switch (c) {
		// 0 - red, 1 - yellow, 2 - green, 3 - sky, 4 - blue, 5 - puple
	case 0:
		red = 255;
		green = 0;
		blue = 0;
		break;
	case 1://pink
		red = 255;
		green = 255;
		blue = 0;
		break;
	case 2://blue
		red = 0;
		green = 255;
		blue = 0;
		break;
	case 3://green
		red = 0;
		green = 0;
		blue = 255;
		break;

	case 4://sky
		red = 0;
		green = 255;
		blue = 255;
		break;

	case 5://yellow
		red = 255;
		green = 0;
		blue = 255;
		break;

	}
}
