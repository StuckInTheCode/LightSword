#include "Sound.h"

Sound::Sound() {
	sounds.speakerPin = 9;							//setup read sounds from sd
	sounds.setVolume(5);
	sounds.quality(1);
	SD.begin(8);
	sound_timer = -9000;
}

Sound::~Sound()
{
}

void Sound::blow(unsigned long ACC) {
	if ((ACC > BLOW) && (ACC < HARD_BLOW)) { // if acceleration enough to play blow;
		sounds.play("BL.wav");               // play blow
		if (DEBUG)
			Serial.println(F("BLOW_"));
		sound_timer = millis() - SOUND + 560;
		strike_flag = 1;
	}
	if (ACC >= HARD_BLOW) {                    //if acceleration enough to play hard blow;
		sounds.play("SBL.wav");               // play strong blow sound
		if (DEBUG)
			Serial.println(F("BLOWS"));
		sound_timer = millis() - SOUND + 705;
		strike_flag = 1;
	}
}

void Sound::swing(unsigned long GYR) {
	if (GYR > 80 && (millis() - swing_timeout > 100)) {
		swing_timeout = millis();
		if (((millis() - swing_timer) > 500) && !strike_flag && swing_flag) {
			if ((GYR > SWING) && (GYR < STRONG_SWING)) {
				//if velocity was enough to play swing
				sounds.play("SW.wav");                     // play swing sound
				sound_timer = millis() - SOUND + 390;
				swing_timer = millis();
				swing_flag = 0;
			}
			if (GYR >= STRONG_SWING) {
				// if velocity was enough to play strong swing
				sounds.play("SSW.wav");                   //  play strong swing sound
				sound_timer = millis() - SOUND + 360;
				swing_timer = millis();
				swing_flag = 0;
			}
		}
	}
}

void Sound::On()
{
	sounds.play("On.wav");       // play on sound
}

void Sound::Off()
{
	sounds.play("Off.wav");       // play off sound
}

void Sound::Mute() {
	sounds.disable();				//disable all sounds
}