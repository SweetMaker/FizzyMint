/*******************************************************************************
StarDestroyer.ino - Takes a FizzyMint and creates a StarDestroyer.

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

void eventHandler(uint16_t eventId, uint8_t source, uint16_t eventInfo);

static const SigGen::SAMPLE lazerBoltValue[] PROGMEM = { 0,200,255,200,150,100,50,0 };
static const SigGen::SAMPLE turboLazerSound[] PROGMEM = { NOTE_E7_US, NOTE_D7_US, NOTE_E6_US, NOTE_C6_US };

SigGen lazerBoltLightValueSig(lazerBoltValue, NUM_SAM(lazerBoltValue), 500, 0);
SigGen lazerBoltValueSineSig(sineWave255, NUM_SAM(sineWave255), 50, 0);
SigScaler lazerBoltLightScaler;
SigMixer lazerBoltLightMixer;

SigGen turboLazerSoundSig(turboLazerSound, NUM_SAM(turboLazerSound), 500, 0);
SigGen sineWaveSig(sineWave255, NUM_SAM(sineWave255), 50, 0);
SigScaler sineScaler;
SigMixer sigMixer;

SigGen engineSineGen(sineWave255, NUM_SAM(sineWave255), 5000, 0);
SigScaler engineScaler;
StaticGen engineStatic;
SigStopper engineStop;

SigGen greenLight(sineWave255, NUM_SAM(sineWave255), 5000, 0);

enum
{
	LazerSoundRed
}EventRefVal;


void setup()
{
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

	engineSineGen.configOutput(&engineScaler);
	engineScaler.configLinearScale(128, 128);
	engineScaler.configOutput(&engineStop);
	engineStop.configOutput(fizzy.light + BLUE_LIGHT);
	engineStatic.configOutput(engineStop.stopCtrl);
	engineStatic.configDuty_256(20);
	engineStatic.configPeriod_ms(20);

	engineSineGen.start();

	greenLight.start(0, fizzy.light + GREEN_LIGHT);

}


void loop()
{
	fizzy.update();
}

void eventHandler(uint16_t eventId, uint8_t source, uint16_t eventInfo)
{
	switch (eventId)
	{
		case SwitchDriver::SWITCH_TURNED_ON:
		Serial.println("Switch On");
		lazerBoltLightValueSig.start(1);
		lazerBoltValueSineSig.start();
		lazerBoltLightMixer.start();

		turboLazerSoundSig.start(1);
		sineWaveSig.start();
		sigMixer.start();

		fizzy.tunePlayer.mute();
		break;

		case SwitchDriver::SWITCH_HELD_ON:
		static uint8_t songIndex = 0;
		switch (songIndex++ % 2) {
			case 0:
			fizzy.tunePlayer.playTune(imperialMarchNotes, NUM_NOTE(imperialMarchNotes), IMPERIAL_MARCH_TEMPO);
			break;

			case 1:
			fizzy.tunePlayer.playTune(starWarsNotes, NUM_NOTE(starWarsNotes), STARWARS_TEMPO);
			break;

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

		case TunePlayer::TUNE_NEXT_NOTE:
		greenLight.update(500);
		break;
	}
}
