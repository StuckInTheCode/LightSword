#pragma once
#define NUM_LEDS 20         // number of leds in the strip
#define BTN_TIMEOUT 800     // button waiting time until pressed processing
#define BRIGHTNESS 200      // maximum
#define SWING 100           // minimum angular velocity
#define STRONG_SWING 200    
#define BLOW 45				// minimum acceleration
#define HARD_BLOW 60      
#define SOUND 9000

#include <SD.h>
#include <TMRpcm.h>         

class Sound
{
public:
	//object for sounds
	TMRpcm sounds;
	boolean soundEnable;
	unsigned long sound_timer;							//time of main sound playing
	unsigned long swing_timer, swing_timeout;           //time of light effect
	boolean strike_flag, swing_flag, lightFlag;

	Sound();
	~Sound();

	void blow(unsigned long ACC);
	void swing(unsigned long GYR);

	void On();
	void Off();
	void Mute();
};

