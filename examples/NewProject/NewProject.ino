/*******************************************************************************
NewProject.ino - A 'blank' project to jumpstart your FizzyMint development

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
#include <EEPROM.h>
#include <FizzyMint.h>
#include "TuneLib.h"

FizzyMint fizzyMint;

void myEventHandler(uint16_t eventId, uint8_t src, uint16_t eventInfo);

void setup()
{
	/* Start Serial at a speed (Baud rate) of 112500 Bytes per second */
	Serial.begin(112500);
	Serial.println("SweetMaker says Hi");

	fizzyMint.configEventHandlerCallback(myEventHandler);
}

void loop()
{
	fizzyMint.update();
}

static const SigGen::SAMPLE alarm[] PROGMEM = { NOTE_G7_US, NOTE_B7_US, NOTE_D7_US };
SigGen alarmGen(alarm, NUM_SAM(alarm), 90, SigGen::INTERPOLATE_OFF);

void myEventHandler(uint16_t eventId, uint8_t eventRef, uint16_t eventInfo)
{
	switch (eventId)
	{
		case SwitchDriver::SWITCH_TURNED_ON: // The button has been pressed
		{
			fizzyMint.light[RED_LIGHT].turnOn();
			fizzyMint.light[GREEN_LIGHT].turnOn(200);
			fizzyMint.light[BLUE_LIGHT].turnOn(150);
//			fizzyMint.buzzer.playPeriod_us(NOTE_C5_US, 1000);

//			fizzyMint.tunePlayer.playTune(starWarsNotes, STAR_WARS_NOTE_COUNT, 60);
		}
		break;

		case PiezoDriver::KNOCK_DETECTED: // A knock has been detected - eventInfo says how hard.
		{
			alarmGen.start(10, &fizzyMint.buzzer);
			fizzyMint.light[RED_LIGHT].turnOn();
			fizzyMint.light[GREEN_LIGHT].turnOn();
			fizzyMint.light[BLUE_LIGHT].turnOn();
		}
		break;

		case SigGen::SIG_GEN_FINISHED: // A Signal Generator has finished 
		{
			fizzyMint.buzzer.stop();
			fizzyMint.light[RED_LIGHT].turnOff();
			fizzyMint.light[GREEN_LIGHT].turnOff();
			fizzyMint.light[BLUE_LIGHT].turnOff();
		}

		case SwitchDriver::SWITCH_TURNED_OFF: // The switch has been released
		case SwitchDriver::SWITCH_HELD_ON:   // The button has been held down for a second
		case SwitchDriver::SWITCH_STILL_HELD_ON: // Held down for another second - eventInfo counts number of seconds
		case SigGen::SIG_GEN_STARTED: // A Signal Generator has been started
		case SigGen::SIG_GEN_STOPPED: // A Signal Generator has been stopped
		case TimerTickMngt::TIMER_EXPIRED: // A timer has expired - eventInfo from timerId
		case TimerTickMngt::TIMER_TICK_UPDATE: // Generated every time fizzyMint is updated - could be every 500us (micro seconds) e.g. less than a millisecond
		case TimerTickMngt::TIMER_TICK_100MS: // Generated ten times a second
		case TimerTickMngt::TIMER_TICK_10S: // Generated once every ten seconds
		case TimerTickMngt::TIMER_FREQ_GEN: // Generated a certain number of times a seconds
		case TimerTickMngt::TIMER_TICK_S: // Generated every second
		case TunePlayer::TUNE_NEXT_NOTE: // Tune Player started next note
		case TunePlayer::TUNE_STOPPED: // The Tune has been stopped before it has ended
		case TunePlayer::TUNE_ENDED: // The Tune has ended
		break;
	}
}


