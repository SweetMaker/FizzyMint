/*******************************************************************************
Light.ino - Takes a FizzyMint and creates various light effects.

Copyright(C) 2017 Howard James May

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.

Contact me at sweet.maker@outlook.com

*******************************************************************************/

#include "FizzyMint.h"
#include "TuneLib.h"
#include "SigMixer.h"
#include "StaticGen.h"

FizzyMint fizzy;

void systemCheck(void);
void powerUp(void);
void goToStandby(void);
void standby(void);
void operational(void);
void firing(void);

void eventHandler(uint16_t eventId, uint8_t source, uint16_t eventInfo);

static const SigGen::SAMPLE lazerBoltValue[] PROGMEM = { 0,200,255,200,150,100,50,0 };
static const SigGen::SAMPLE lazerBoltSound[] PROGMEM = { NOTE_E7_US, NOTE_D7_US, NOTE_E6_US, NOTE_C6_US };

SigGen lazerBoltLightValueSig(lazerBoltValue, NUM_SAM(lazerBoltValue), 500, 0);
SigGen lazerBoltValueSineSig(sineWave255, NUM_SAM(sineWave255), 50, 0);
SigScaler lazerBoltLightScaler;
SigMixer lazerBoltLightMixer;

SigGen turboLazerSoundSig(lazerBoltSound, NUM_SAM(lazerBoltSound), 500, 0);
SigGen sineWaveSig(sineWave255, NUM_SAM(sineWave255), 50, 0);
SigScaler sineScaler;
SigMixer sigMixer;

SigGen capacitorSineGen(sineWave255, NUM_SAM(sineWave255), 5000, 0);
SigScaler capacitorScaler;
StaticGen capacitorStatic;
SigStopper capacitorStop;

enum
{
	LazerSoundRed
}EventRefVal;


void setup()
{
	Serial.begin(115200);
	Serial.println("Lazer starting up");

	fizzy.configEventHandlerCallback(eventHandler);

	lazerBoltLightValueSig.configOutput(&lazerBoltLightMixer);
	lazerBoltValueSineSig.configOutput(&lazerBoltLightScaler);
	lazerBoltLightScaler.configLinearScale(128, -64);
	lazerBoltLightScaler.configOutput(&lazerBoltLightMixer);
	lazerBoltLightMixer.configOutput(fizzy.light + RED_LIGHT);

	turboLazerSoundSig.configOutput(&sigMixer);
	turboLazerSoundSig.configEventRef(LazerSoundRed);
	sineWaveSig.configOutput(&sineScaler);
	sineScaler.configLinearScale(128, -64);
	sineScaler.configOutput(&sigMixer);
	sigMixer.configOutput(&fizzy.buzzer);
	sigMixer.stop();

	capacitorSineGen.configOutput(&capacitorScaler);
	capacitorScaler.configLinearScale(128, 128);
	capacitorScaler.configOutput(&capacitorStop);
	capacitorStop.configOutput(fizzy.light + GREEN_LIGHT);
	capacitorStatic.configOutput(capacitorStop.stopCtrl);
	capacitorStatic.configDuty_256(20);
	capacitorStatic.configPeriod_ms(20);

	capacitorSineGen.start();
}


void loop()
{
//	systemCheck();
	fizzy.update();
}


void eventHandler(uint16_t eventId, uint8_t source, uint16_t eventInfo)
{
	switch (eventId)
	{
		case SwitchDriver::SWITCH_TURNED_ON:
		{
			Serial.println("Switch On");
			lazerBoltLightValueSig.start(1);
			lazerBoltValueSineSig.start();
			lazerBoltLightMixer.start();

			turboLazerSoundSig.start(1);
			sineWaveSig.start();
			sigMixer.start();

			fizzy.tunePlayer.mute();
		}
		break;

		case SwitchDriver::SWITCH_HELD_ON:
		{
			static uint8_t songIndex = 0;
			switch (songIndex++ % 2) {
			case 0:
				fizzy.tunePlayer.playTune(imperialMarchNotes, NUM_NOTE(imperialMarchNotes), IMPERIAL_MARCH_TEMPO);
				break;

			case 1:
				fizzy.tunePlayer.playTune(starWarsNotes, NUM_NOTE(starWarsNotes), STARWARS_TEMPO);
				break;
			}

		}
		break;

		case SwitchDriver::SWITCH_STILL_HELD_ON:
		fizzy.tunePlayer.stop();
		break;

		case SigGen::SIG_GEN_FINISHED:
		if (eventInfo == LazerSoundRed)
		{
			sigMixer.stop();
			lazerBoltLightMixer.stop();
			lazerBoltValueSineSig.stop();

			fizzy.tunePlayer.unMute();
			fizzy.buzzer.stop();
		}
		break;

	}
}


void systemCheck(void)
{
	SigGen sine(sineWave255, NUM_SAM(sineWave255), 500, 0);
	SigScaler scaler(512, NOTE_A0_US);
	Serial.println("System Check");
	sine.start(0, &scaler);
	scaler.configOutput(&fizzy.buzzer);
	fizzy.updateDelay(500);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (uint8_t led_id = 0; led_id <= i; led_id++)
				fizzy.light[led_id].turnOn();
			fizzy.updateDelay(300);
			for (uint8_t led_id = 0; led_id <= i; led_id++)
				fizzy.light[led_id].turnOff();
			fizzy.updateDelay(300);
		}
	}
	sine.stop();

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			for (uint8_t led_id = 0; led_id <= i; led_id++)
				fizzy.light[led_id].turnOn();
			fizzy.buzzer.playPeriod_us(NOTE_A6_US - (i * 30), 250);
			fizzy.updateDelay(300);
			fizzy.buzzer.playPeriod_us(NOTE_A6_US - (i * 30), 250);
			for (uint8_t led_id = 0; led_id <= i; led_id++)
				fizzy.light[led_id].turnOff();
			fizzy.updateDelay(300);
		}
	}





}

void powerUp(void)
{


}

void goToStandby(void)
{


}

void standby(void)
{

}

void operational(void)
{

}

void firing(void)
{

}
