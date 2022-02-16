/*******************************************************************************
Target.cpp - Takes a FizzyMint and creates a Target which detects hits.

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

#include <Arduino.h>
#include "target.h"
#include "SigConnector.h"
#include "StaticGen.h"
#include "SigConnector.h"
#include "SigStopper.h"
#include "PerfMon.h"


using namespace SweetMaker;

Target::Target(FizzyMint * fizzy)
{
	fizzyMint = fizzy;
	fizzyMint->configEventHandlerCallback(this);
}

void Target::setup()
{
	Serial.begin(115200);
	Serial.println(F("SweetMaker::FizzyMint Nerf Gun Target Example Sketch"));
	/*
	* Initialise three sine wave signal generators to use for flashing lights
	* time period is set to 300ms
	*/
	for (int i = 0; i < 3; i++) {
		sineWave[i].configSamples(sineWave255, NUM_SAM(sineWave255), 300, 0);
		sineWave[i].configOutput(&fizzyMint->light[i]);
	}
}

/* 
 * waitForNewGame: flashes lights periodically waiting for switchPressedEvent
 */
void Target::waitForNewGame()
{
	// Starts by giving single beep and flash 
	fizzyMint->buzzer.playPeriod_us(NOTE_C5_US, 300);
	flashLights();
	fizzyMint->updateDelay(600);

	 // Now periodically flash lights until switch pressed
	Timer idleTimer;
	switchPressedEvent = false;
	while (switchPressedEvent == false) {
		if (!idleTimer.isRunning()) {
			flashLightsStaggered();
			idleTimer.startTimer(2000, 0);
		}
		fizzyMint->update();
	}

	// Now flash and beep twice
	for (int i = 0; i < 2; i++) {
		flashLights(1);
		fizzyMint->buzzer.playPeriod_us(NOTE_C7_US, 150);
		fizzyMint->updateDelay(200);
	}
}

/*
 * playStartTune: Plays a simple staggered ascending scale while flashing
 *                lights for each new note.
 */
void Target::playStartTune()
{
	NOTE simpleMelody[] = {
		{ NOTE_C3_US,1 },
		{ NOTE_D3_US,1 },
		{ NOTE_E3_US,2 },

		{ NOTE_D3_US,1 },
		{ NOTE_E3_US,1 },
		{ NOTE_F3_US,2 },

		{ NOTE_E3_US,1 },
		{ NOTE_F3_US,1 },
		{ NOTE_G3_US,2 },

		{ NOTE_F3_US,1 },
		{ NOTE_G3_US,1 },
		{ NOTE_A3_US,2 },

		{ NOTE_G3_US,1 },
		{ NOTE_A3_US,1 },
		{ NOTE_B3_US,2 },

		{ NOTE_A3_US,1 },
		{ NOTE_B3_US,1 },
		{ NOTE_C4_US,2 }
	};

	// Start playing tune
	fizzyMint->tunePlayer.playTune(simpleMelody, NUM_NOTE(simpleMelody), 1200);

	newNoteEvent = true; // flash at start as well
	while (fizzyMint->tunePlayer.isRunning())
	{
		if (newNoteEvent == true) {
			// For each new note flash lights
			newNoteEvent = false; 
			flashLights();
		}
		fizzyMint->update();
	}
}

/*
 * startCountDown: Does a 5 second count followed by a Ring to start
 */
void Target::startCountDown()
{
	uint16_t countdownNote_us[5] = {
		NOTE_A4_US,
		NOTE_C5_US,
		NOTE_E5_US,
		NOTE_G5_US,
		NOTE_B5_US
	};

	// Countdown
	for (int i = 0; i < 5; i++) {
		fizzyMint->buzzer.playPeriod_us(countdownNote_us[i], 300);
		flashLights();
		fizzyMint->updateDelay(800);
	}

	// Sound Siren Alarm and flash lights 
	static const SigGen::SAMPLE alarm[] PROGMEM = { NOTE_G7_US, NOTE_B7_US, NOTE_D7_US };
	SigGen alarmGen(alarm, NUM_SAM(alarm), 90, SigGen::INTERPOLATE_OFF);


	for (int i = 0; i < 3; i++) {
		alarmGen.start(6, &fizzyMint->buzzer);
		flashLights(6, 90);
		while (alarmGen.isRunning())
			fizzyMint->update();

		fizzyMint->buzzer.stop();
		fizzyMint->updateDelay(300);
	}
}


/*
 * gameOn: loop waiting for the following events
 *
 *      piezoHitEvent: The target has been hit, increment score and react
 *                     by flashing lights and alarming
 *
 *      switchPressed: The bullseye has been hit, double increment score and 
 *                     react by flashing lights and alarming
 *
 *      switchHeldDown: This is a user reset event requesting to end the game
 *
 *      heartbeatTimeExpiry: React by briefly flashing lights and beeping
 *
 * When a score of 5 has been reached we exit this routine
 *                      
 */
void Target::gameOn()
{
	uint8_t score = 0;

	Timer heartbeatTimer(5000, 0);

	piezoHitEvent = false;
	switchPressedEvent = false;
	switchHeldEvent = false;

	Serial.println("Starting to keep score");
	while (score < 5) {

		if (!heartbeatTimer.isRunning()) 
		{
			Serial.println(score);
			heartbeatTimer.startTimer(5000, 0);
			fizzyMint->buzzer.playPeriod_us(NOTE_C7_US, 100);
			flashLightsStaggered();
		}

		if (piezoHitEvent == true)
		{
			score++;
			flashLights(5);
			playHitNoise();
			fizzyMint->updateDelay(300);
			piezoHitEvent = false;
		}

		if (switchPressedEvent == true)
		{
			score++;
			switchPressedEvent = false;
		}

		fizzyMint->update();
	}
}

/*
* winnersFanfare: this configures a firework like effect where each light periodically 
*                 starts to fire as a rocket with a falling note and then explodes with
*                 a static crackle noise and ligths which fade out.
*
*                 This is achieved with a complex sequence of signal generators, static
*                 generators and mixers.
*/
void Target::winnersFanfare()
{
	// define signal smaples we will use
	static const SigGen::SAMPLE fwSam[] PROGMEM = { 0,40,45,50,55,60,65,70,75,80,100,255,255,255,255,255,255,255,255,255 };
	static const SigGen::SAMPLE samStaticDuty[] PROGMEM = { 20,20,20,20,20,20,20,20,20,20,20,20,50,80,110,140,170,200,230,255 };
	static const SigGen::SAMPLE whizz[] PROGMEM = { NOTE_G7_US, NOTE_G5_US,NOTE_G4_US, NOTE_G3_US };
	static const SigGen::SAMPLE crackle[] PROGMEM = { NOTE_G6_US, NOTE_G4_US,NOTE_G5_US, NOTE_G3_US };

	// declare the various objects we will use
	SigGen sigGen[3];
	SigGen sigGenStaticDuty[3];
	StaticGen staticGen[3];
	SigStopper stopper[3];
	SigGen whizzSig(whizz, NUM_SAM(whizz), 2000, 0);
	whizzSig.configEventRef(10);
	SigGen crackleSig(crackle, NUM_SAM(crackle), 200, SigGen::INTERPOLATE_OFF);

	// initialise and configure objects 
	for (int i = 0; i < 3; i++) {
		sigGen[i].configSamples(fwSam, NUM_SAM(fwSam), 4000, 0);
		sigGen[i].configOutput(&stopper[i]);
		stopper[i].configOutput(&fizzyMint->light[i]);
		staticGen[i].configOutput(stopper[i].stopCtrl);
		staticGen[i].configPeriod_ms(20);
		staticGen[i].configDuty_256(230);

		sigGenStaticDuty[i].configSamples(samStaticDuty, NUM_SAM(samStaticDuty), 4000, 0);
	}

	Timer timer[3];  // used for controlling when next rocket will fire
	Timer finishTimer(15000, 0xffff); // used to bring fanfare to a close

	// select random start times for first round of rockets
	for (int i = 0; i < 3; i++) {
		timer[i].startTimer(random(0, 2000), 0xffff);
	}

	fizzyMint->updateDelay(1000);

	/*
	* Enter main loop - this loops around checking rocket start timers
	* and then triggering rockets as necessary. When the whizz sound stops
	* it automatically calls the 'crackle' sound effect for the rocket post
	* explosion.
	*/
	whizzSigFinished = false;
	while (finishTimer.isRunning()) {
		for (int i = 0; i < 3; i++) {
			staticGen[i].configDuty_256((uint8_t)sigGenStaticDuty[i].readValue());

			if (!timer[i].isRunning()) {
				sigGen[i].start(1);
				sigGenStaticDuty[i].start(1);
				timer[i].startTimer(random(3000, 5000), 0xffff);
				whizzSig.start(1, &fizzyMint->buzzer);
				crackleSig.stop();
				fizzyMint->update();
				 
			}
		}

		if (whizzSigFinished == true) {
			crackleSig.start(10, &fizzyMint->buzzer);
			whizzSigFinished = false;
		}

		fizzyMint->update();
	}
}


void Target::loop()
{
	waitForNewGame();

	playStartTune();

	// Add slight delay between start tune and countdown
	fizzyMint->updateDelay(1000);

	startCountDown();

	gameOn();

	winnersFanfare();
}

/*
 * handleEvent: this is our fizzyMint callback function. It simply sets boolean
 *              flags to record events have happened which the target can check 
 *              and respond to.
 */
void Target::handleEvent(uint16_t eventId, uint8_t sourceReference, uint16_t eventInfo)
{
//	Serial.println(eventId);
	switch (eventId) {

		case SwitchDriver::SWITCH_TURNED_ON:
		switchPressedEvent = true;
		break;

		case SwitchDriver::SWITCH_HELD_ON:
		{
			switchHeldEvent = true;

			// This causes the code to jump to program address location '0'
			// This results in it restarting
			void(*resetFunc)(void) = 0;
			resetFunc();
		}
		break;

		case PiezoDriver::KNOCK_DETECTED:
		Serial.println("knock detected");
		piezoHitEvent = true;
		break;

		case SigGen::SIG_GEN_FINISHED:
		if (eventInfo == 10)
			whizzSigFinished = true;
		break;

		case TunePlayer::TUNE_NEXT_NOTE:
		newNoteEvent = true;
		break;
	}
}

/*
 * flashLights: simple helper function to flash lights 'stagger'
 */
void Target::flashLights()
{
	flashLights(1);
}

void Target::flashLights(uint16_t numFlash)
{
	for (int i = 0; i < 3; i++) {
		sineWave[i].start(numFlash);
	}
}

void Target::flashLights(uint16_t numFlash, uint16_t period_ms)
{
	for (int i = 0; i < 3; i++) {
		sineWave[i].configPeriod_ms(period_ms);
		sineWave[i].start(numFlash);
	}
}

void Target::flashLightsStaggered()
{
	for (int i = 0; i < 3; i++) {
		sineWave[i].start(1, i * 200);
	}
}

void Target::playHitNoise()
{
	static const SigGen::SAMPLE hitNoiseSignal[] PROGMEM = { NOTE_C2_US,NOTE_C3_US, NOTE_C4_US, NOTE_C5_US, NOTE_C6_US, NOTE_C5_US, NOTE_C6_US, NOTE_C5_US, NOTE_C6_US, NOTE_C6_US };
	buzzerSig.configSamples(hitNoiseSignal, NUM_SAM(hitNoiseSignal), 300, 0);
	buzzerSig.configOutput(&fizzyMint->buzzer);
	buzzerSig.start(3);
}