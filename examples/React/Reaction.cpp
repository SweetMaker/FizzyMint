
/*******************************************************************************
Reaction.cpp - Takes a FizzyMint and creates a reaction game.

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

/*
* We will be using the SweetMaker Software Development Kit and a fizzyMint so
* start by including the following files to import class definitions
*/
#include <Arduino.h>
#include "FizzyMint.h"
#include "SigConnector.h"
#include "SigGen.h"
#include "StaticGen.h"
#include "SigMixer.h"
#include "SigScaler.h"
#include "PerfMon.h"

#include "./React.h"

using namespace SweetMaker;

FizzyMint *fizzyMint;

unsigned long playerTimes_ms[2];

bool switchPressedFlag;
bool switchHeldOnFlag;
bool switchReleasedFlag;
bool timerExpired;
bool knockDetected;
bool tuneEnded;
bool nextNote;
bool whizzSigFinished;

NOTE inTheHallOfTheMountainKingFirstSection[] = {
	// First Section
	{ NOTE_D3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_G3_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_A3_US, 4 },{ NOTE_GS3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_GS3_US, 4 },{ NOTE_G3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_G3_US, 4 },
	{ NOTE_D3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_G3_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_D4_US, 2 },{ NOTE_C4_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_C4_US, 4 },{ 0, 1 }
};

NOTE inTheHallOfTheMountainKingSecondSection[] = {
	//Second Repeat
	{ NOTE_A3_US, 2 },{ NOTE_B3_US, 2 },{ NOTE_CS4_US, 2 },{ NOTE_D4_US, 2 },{ NOTE_E4_US, 2 },{ NOTE_C4_US, 2 },{ NOTE_E4_US, 4 },{ NOTE_F4_US, 2 },{ NOTE_CS4_US, 2 },{ NOTE_F4_US, 4 },{ NOTE_E4_US, 2 },{ NOTE_C4_US, 2 },{ NOTE_E4_US, 4 },
	{ NOTE_A3_US, 2 },{ NOTE_B3_US, 2 },{ NOTE_CS4_US, 2 },{ NOTE_D4_US, 2 },{ NOTE_E4_US, 2 },{ NOTE_C4_US, 2 },{ NOTE_E4_US, 4 },{ NOTE_F4_US, 2 },{ NOTE_CS4_US, 2 },{ NOTE_F4_US, 4 },{ NOTE_E4_US, 8 }
};

NOTE inTheHallOfTheMountainKingLastSection[] = {
	// Last Section 
	{ NOTE_D3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_G3_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_A3_US, 4 },{ NOTE_GS3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_GS3_US, 4 },{ NOTE_G3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_G3_US, 4 },
	{ NOTE_D3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_G3_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_D4_US, 2 },{ NOTE_C4_US, 2 },{ NOTE_A3_US, 2 },{ NOTE_F3_US, 2 },{ NOTE_E3_US, 2 },{ NOTE_D3_US, 4 }
};

const SigGen::SAMPLE blip[] PROGMEM = { 64,255,230,205,180,155,130,105,80,55,30,0 };
const SigGen::SAMPLE bellAlarm[] PROGMEM = { NOTE_G5_US, NOTE_B5_US, NOTE_D5_US };

#define PLAYER_TIME_NULL (0xffff)
#define PLAYER_TIME_FAIL (0)

void waitForSwitchHeldOnEvent();
void playTuneUntilSwitchPressed(void);
void playBellRingingEffect(void);
void playHeadsDownEffect(void);
void waitForPlayerToPressSwitch(uint8_t player);
int playRisingTension(void);
void playErrorEffect();
unsigned long timePlayerReaction(void);
void loopReactBack(void);
void loopAnnounceRoundWinner();
void loopCelebrateWinner();
int calculateRoundWinner();


void reactHandleEvent(uint16_t eventId, uint8_t sourceInstance, uint16_t eventInfo);
void update();
void updateDelay(uint16_t delay_ms);

#define PLAYER1 (0)
#define PLAYER2 (1)

void reactSetup()
{
	Serial.begin(115200);
	Serial.println(F("** React **"));
	Serial.println((uint16_t)reactSetup);
	Serial.println((uint16_t)reactHandleEvent);

	fizzyMint = new FizzyMint();
	fizzyMint->configEventHandlerCallback(reactHandleEvent);

	randomSeed(0);

	fizzyMint->light[BLUE_LIGHT].turnOff();
	fizzyMint->light[RED_LIGHT].turnOff();
	fizzyMint->light[GREEN_LIGHT].turnOff();
}

void reactLoop()
{
	waitForSwitchHeldOnEvent();

	playTuneUntilSwitchPressed();

	updateDelay(1000);

	playerTimes_ms[0] = PLAYER_TIME_NULL;
	playerTimes_ms[1] = PLAYER_TIME_NULL;

		/*
		 * Announce Start
		 */
	playBellRingingEffect();
	updateDelay(100);

	for (int playerNum = 0; playerNum < 2; playerNum++)
	{
		//Serial.println(playerNum);
		playHeadsDownEffect();

		updateDelay(100);
		//Serial.println(F("Wait For Switch to be Pressed"));
		waitForPlayerToPressSwitch(playerNum);
		if (playRisingTension() == -1)
		{
				/* Player has moved too soon - they lose */
				//Serial.println(F("Sorry - you moved too soon"));
			playErrorEffect();
			playerTimes_ms[playerNum] = PLAYER_TIME_FAIL;
			updateDelay(1000);
			break;
		}
		else
		{
			playerTimes_ms[playerNum] = timePlayerReaction();
			//Serial.println(playerTimes_ms[playerNum]);
			loopReactBack();
		}
		updateDelay(300);
	}

		//Serial.print(F("Round winner is player: "));
		//Serial.println(calculateRoundWinner());
	loopAnnounceRoundWinner();
	updateDelay(500);

	//Serial.print(F("Scores are: "));
	//Serial.print(playerScores[0]);
	//Serial.print(F(" : "));
	//Serial.println(playerScores[1]);

	loopCelebrateWinner();

	updateDelay(500);
}

void update()
{
	fizzyMint->update();
}

void updateDelay(uint16_t delay_ms)
{
	unsigned long targetTime = millis() + delay_ms;
	while (millis() < targetTime)
		update();
}

/*
* This is the eventHandler Callback
*/
void reactHandleEvent(uint16_t eventId, uint8_t sourceRef, uint16_t eventInfo)
{
	switch (eventId)
	{
		case SigGen::SIG_GEN_FINISHED:
		whizzSigFinished = true;
		break;

		case SwitchDriver::SWITCH_TURNED_ON:
		switchPressedFlag = true;
		break;

		case SwitchDriver::SWITCH_HELD_ON:
		switchHeldOnFlag = true;
		break;

		case SwitchDriver::SWITCH_TURNED_OFF:
		//Serial.println(F("Switch Turned Off"));
		switchReleasedFlag = true; 
		break;

		case TimerTickMngt::TIMER_EXPIRED:
		//Serial.print("Timer Expired");
		//Serial.println(eventInfo);
		if (eventInfo == 1)
			timerExpired = true;
		break;

		case PiezoDriver::KNOCK_DETECTED:
		knockDetected = true;
		break;

		case TunePlayer::TUNE_ENDED:
		tuneEnded = true;
		break;

		case TunePlayer::TUNE_NEXT_NOTE:
		nextNote = true;
		break;

		case TimerTickMngt::TIMER_TICK_S:
		break;

		case TimerTickMngt::TIMER_TICK_10S:
		PerfMon::getPerfMon()->intervalStop();
		PerfMon::getPerfMon()->print();
		PerfMon::getPerfMon()->reset();
		PerfMon::getPerfMon()->intervalStart();
		break;
	}
}


void waitForSwitchHeldOnEvent(void)
{
	Timer flashTimer(5000, 0);
	SigGen blipGen(blip, NUM_SAM(blip), 100, 0);
	blipGen.configOutput(fizzyMint->light + BLUE_LIGHT);
	
	switchHeldOnFlag = false;
	while (switchHeldOnFlag == false)
	{
		if (!flashTimer.isRunning())
		{
			blipGen.start(2);
			flashTimer.startTimer(5000,0);
		}
		fizzyMint->update();
	}
	blipGen.stop();
	fizzyMint->light[BLUE_LIGHT].turnOff();
}


void playTuneUntilSwitchPressed(void)
{
	uint8_t ledSeq, tuneSeq;
	Timer timer;
	SigGen sigGen[3];
	for (int i = 0; i < 3; i++) {
		sigGen[i].configSamples(blip, NUM_SAM(blip), 700, 0);
		sigGen[i].configOutput(&fizzyMint->light[i]);
	}

	tuneSeq = 0;
	ledSeq = 1;

	sigGen[RED_LIGHT].start(1);
	
	fizzyMint->tunePlayer.configReverb(32, 100);

	fizzyMint->tunePlayer.playTune(
		inTheHallOfTheMountainKingFirstSection,
		NUM_NOTE(inTheHallOfTheMountainKingFirstSection),
		100 * 8);

	/*
	* Waiting to play
	*/
	switchPressedFlag = false;
	tuneEnded = false;
	timerExpired = false;
	nextNote = true;

	while (switchPressedFlag == false)
	{
		if (tuneEnded == true)
		{
			tuneEnded = false;
			switch (tuneSeq++) {
				case 0:
				fizzyMint->tunePlayer.playTune(
					inTheHallOfTheMountainKingFirstSection,
					NUM_NOTE(inTheHallOfTheMountainKingFirstSection),
					100 * 8);
				break;
				case 1:
				case 2:
				fizzyMint->tunePlayer.playTune(
					inTheHallOfTheMountainKingSecondSection,
					NUM_NOTE(inTheHallOfTheMountainKingSecondSection),
					100 * 8);
				break;
				case 3:
				fizzyMint->tunePlayer.playTune(
					inTheHallOfTheMountainKingLastSection,
					NUM_NOTE(inTheHallOfTheMountainKingLastSection),
					100 * 8);
				break;
				case 4:
				tuneSeq = 0;
				timer.startTimer(4000, 1);
				timerExpired = false;
				break;
			}
		}

		if (timerExpired == true)
		{
			timerExpired = false;
			fizzyMint->tunePlayer.playTune(
				inTheHallOfTheMountainKingFirstSection,
				NUM_NOTE(inTheHallOfTheMountainKingFirstSection),
				100 * 8);
		}

		if (nextNote == true)
		{
			nextNote = false;
			switch (ledSeq++) {
				case 0:	sigGen[GREEN_LIGHT].start(1);	break;
				case 1:	sigGen[RED_LIGHT].start(1); break;
				case 2: sigGen[BLUE_LIGHT].start(1); break;
				case 3: sigGen[RED_LIGHT].start(1); ledSeq = 0; break;
			}
		}

		update();
	}

	NOTE switchPressBeep[] = {
		{ NOTE_F5_US, 2 },
		{ NOTE_F5_US, 2 }
	};
	fizzyMint->tunePlayer.playTune(switchPressBeep, NUM_NOTE(switchPressBeep), 100 * 8);
	nextNote = true;

	while (fizzyMint->tunePlayer.isRunning() == true)
	{
		update();
		if (nextNote == true)
		{
			nextNote = false;
			for (int i = 0; i < 3; i++)
				sigGen[i].start(1);
		}
	}

	fizzyMint->tunePlayer.playTune(
		inTheHallOfTheMountainKingLastSection,
		NUM_NOTE(inTheHallOfTheMountainKingLastSection),
		400 * 8);
	tuneEnded = false;
	nextNote = true;
	ledSeq = 1;

	while (tuneEnded == false)
	{
		update();
		if (nextNote == true)
		{
			nextNote = false;
			switch (ledSeq++) {
				case 0:	sigGen[GREEN_LIGHT].start(1);	break;
				case 1:	sigGen[RED_LIGHT].start(1); break;
				case 2: sigGen[BLUE_LIGHT].start(1); break;
				case 3: sigGen[RED_LIGHT].start(1); ledSeq = 0; break;
			}
		}
	}
	return;
}


void playBellRingingEffect()
{
	SigGen piezoSig(bellAlarm, NUM_SAM(bellAlarm), 90, SigGen::INTERPOLATE_OFF);

 	SigGen blipSig[3];

	for (int i = 0; i < 3; i++) {
		blipSig[i].configSamples(blip, NUM_SAM(blip), 180, 0);
		blipSig[i].configOutput(&fizzyMint->light[i]);
	}


	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < 3; j++)
			blipSig[j].start(1);

		piezoSig.start(3, &fizzyMint->buzzer);
		while (piezoSig.isRunning() == true)
			update();

		updateDelay(50);
	}

	fizzyMint->buzzer.stop();
}


void playHeadsDownEffect()
{

	SigGen::SAMPLE descendingNote[2] = { NOTE_G7_US, NOTE_G6_US };
	SigGen piezoSig(descendingNote, NUM_SAM(descendingNote), 400, SigGen::DONT_USE_PROGMEM);
	piezoSig.start(1, &fizzyMint->buzzer);

	static const SigGen::SAMPLE lightsDown[] PROGMEM = { 255,50 };
	SigGen lightsDownSig[3];

	for (int i = 0; i < 3; i++)
		lightsDownSig[i].configSamples(lightsDown, NUM_SAM(lightsDown), 800, 0);

	for (int i = 0; i < 6; i++) {
		uint8_t lightNum = random(0, 3);
		lightsDownSig[lightNum].start(1, &fizzyMint->light[lightNum]);
		piezoSig.start(1);

		while (piezoSig.isRunning())
			update();

		descendingNote[0] = (descendingNote[0] >> 1) * 3;
		descendingNote[1] = (descendingNote[1] >> 1) * 3;
	}

	fizzyMint->buzzer.stop();
}

void waitForPlayerToPressSwitch(uint8_t playerNum)
{
	/*
	* Flash towards player
	*/
	SigGen blipSig[3];
	SigScaler blipScalers[3];
	StaticGen staticGen[3];
	SigScaler staticScaler[3];
	SigMixer mixer[3];

	for (int i = 0; i < 3; i++) {
		blipSig[i].configSamples(blip, NUM_SAM(blip), 500 + i * 500, 0);
		blipSig[i].configOutput(&blipScalers[i]);
		blipScalers[i].configLinearScale(64 * (i + 2), 0);
		blipScalers[i].configOutput(&mixer[i]);

		staticGen[i].configDuty_256(200);
		staticGen[i].configPeriod_ms(1);
		staticGen[i].configOutput(&staticScaler[i]);
		staticScaler[i].configLinearScale(40, -20);
		staticScaler[i].configOutput(&mixer[i]);
	}

	if (playerNum == PLAYER1) {
		mixer[0].configOutput(&fizzyMint->light[BLUE_LIGHT]);
		mixer[1].configOutput(&fizzyMint->light[RED_LIGHT]);
		mixer[2].configOutput(&fizzyMint->light[GREEN_LIGHT]);
	}
	else {
		mixer[2].configOutput(&fizzyMint->light[BLUE_LIGHT]);
		mixer[1].configOutput(&fizzyMint->light[RED_LIGHT]);
		mixer[0].configOutput(&fizzyMint->light[GREEN_LIGHT]);
	}

	switchPressedFlag = false;
	static const SigGen::SAMPLE piezoSig[] PROGMEM = { NOTE_C7_US, NOTE_C7_US ,NOTE_C7_US ,NOTE_C7_US, 0, 0 };
	SigGen piezoSigGen(piezoSig, NUM_SAM(piezoSig), 250, SigGen::INTERPOLATE_OFF);
	piezoSigGen.configOutput(&fizzyMint->buzzer);
	while(switchPressedFlag == false)	{
		blipSig[0].start(1);
		blipSig[1].start(1, 100);
		blipSig[2].start(1, 200);
		piezoSigGen.start(2, 1500);

		if (!piezoSigGen.isRunning())
			fizzyMint->buzzer.stop();

		Timer t1(3000, 0xffff);
		while (t1.isRunning())
		{
			if (switchPressedFlag)
				break;
			update();
		}
		if (fizzyMint->microSwitch.getState() == SwitchDriver::STATE_ON)
			break;
	};
	piezoSigGen.stop();
	fizzyMint->buzzer.stop();

	for (int i = 0; i < 3; i++)
	{
		blipSig[i].configPeriod_ms(200);
		blipSig[i].start(2);
	}
	piezoSigGen.configPeriod_ms(200);
	piezoSigGen.start(2, 1500);
	updateDelay(500);
}


int playRisingTension()
{
	uint16_t blipPeriod = 100;

	SigGen blipSig[3];
	SigScaler blipScalers[3];
	StaticGen staticGen[3];
	SigScaler staticScaler[3];
	SigMixer mixer[3];

	for (int i = 0; i < 3; i++) {
		blipSig[i].configSamples(blip, NUM_SAM(blip), blipPeriod, 0);
		blipSig[i].configOutput(&blipScalers[i]);
		blipScalers[i].configLinearScale(255, 0);
		blipScalers[i].configOutput(&mixer[i]);

		staticGen[i].configDuty_256(200);
		staticGen[i].configPeriod_ms(1);
		staticGen[i].configOutput(&staticScaler[i]);
		staticScaler[i].configLinearScale(40, -20);
		staticScaler[i].configOutput(&mixer[i]);
		mixer[i].configOutput(&fizzyMint->light[i]);
	}

	SigGen::SAMPLE tensionNote[] = { 0, 0 };
	SigGen piezoSig(tensionNote, NUM_SAM(tensionNote), 200, 0);
	piezoSig.configOptions(SigGen::DONT_USE_PROGMEM);

	fizzyMint->buzzer.stop();

	tensionNote[0] = NOTE_A2_US;

   	Timer tensionTimer(random(3000, 7000), 1);
	Timer tensionUpdateTimer(1000, 2);

	switchReleasedFlag = false;
	uint8_t blipNum = 1;

	while (tensionTimer.isRunning() && (switchReleasedFlag == false))
	{
		if (!tensionUpdateTimer.isRunning())
		{
			tensionUpdateTimer.startTimer(1000, 2);
			tensionNote[0] = (tensionNote[0] * 6) >> 3;
			piezoSig.start(2, &fizzyMint->buzzer);

			blipSig[0].start(blipNum);
			blipSig[1].start(blipNum, 20);
			blipSig[2].start(blipNum);
			blipNum++;
		}
		update();
	}

	if (switchReleasedFlag == true)
		return (-1);
	
	return (0);
}

unsigned long timePlayerReaction(void)
{
	static const SigGen::SAMPLE fadeOut[] PROGMEM = { 255,150,130,110,90,70,50,30,10,0 };
	SigGen fadeSig[3];
	for (int i = 0; i < 3; i++) {
		fadeSig[i].configSamples(fadeOut, NUM_SAM(fadeOut), 300, 0);
		fadeSig[i].start(1, fizzyMint->light + i);
	}

	fizzyMint->buzzer.playPeriod_us(NOTE_C5_US, 50);

	unsigned long startTime_ms = millis();

	switchPressedFlag = false;
	while (switchPressedFlag == false)
		update();

	return (millis() - startTime_ms);
}

void loopReactBack(void)
{
	SigGen blipSig[3];
	SigScaler blipScalers[3];

	for (int i = 0; i < 3; i++) {
		blipSig[i].configSamples(blip, NUM_SAM(blip), 100, 0);
		blipSig[i].configOutput(&blipScalers[i]);
		blipScalers[i].configLinearScale(255, 0);
		blipScalers[i].configOutput(fizzyMint->light + i);
		blipSig[i].start(1);
	}

	fizzyMint->buzzer.playPeriod_us(NOTE_D3_US, 0);

	for (int i = 0; i <5; i++)
	{
		while (blipSig[0].isRunning())
			update();

		for (int j = 0; j < 3; j++) {
			blipScalers[j].configLinearScale(255 - (i*40), 0);
			blipSig[j].start(1);
		}
	}

	fizzyMint->tunePlayer.playTune(
		inTheHallOfTheMountainKingFirstSection,	7, 100 * 8);
	nextNote = true;
	while (fizzyMint->tunePlayer.isRunning()) {
		update();
		if (nextNote == true) {
			blipSig[random(0, 3) % 3].start();
			nextNote = false;
		}
	}
}


int calculateRoundWinner()
{
	if (playerTimes_ms[PLAYER1] == PLAYER_TIME_FAIL)
		return PLAYER2;

	if (playerTimes_ms[PLAYER2] == PLAYER_TIME_FAIL)
		return PLAYER1;

	if(playerTimes_ms[PLAYER1] > playerTimes_ms[PLAYER2])
		return PLAYER2;
	return PLAYER1;
}

void loopAnnounceRoundWinner()
{
	static const SigGen::SAMPLE red[] PROGMEM = { 0,255,0,255 };
	static const SigGen::SAMPLE green[] PROGMEM = { 255,0,0,0 };
	static const SigGen::SAMPLE blue[] PROGMEM = { 0,0,0,255 };
	NOTE welcomeTune[] = {
		{ NOTE_A4_US, 1 },
		{ NOTE_B4_US, 1 },
		{ NOTE_C5_US, 1 },
		{ NOTE_B4_US, 1 },
		{ NOTE_C5_US, 1 },
		{ NOTE_D5_US, 1 },
		{ NOTE_C5_US, 1 },
		{ NOTE_D5_US, 1 },
		{ NOTE_E5_US, 1 },
		{ NOTE_D5_US, 1 },
		{ NOTE_E5_US, 1 },
		{ NOTE_F5_US, 1 },
		{ NOTE_E5_US, 1 },
		{ NOTE_F5_US, 1 },
		{ NOTE_G5_US, 4 },
		{ 0, 2 }

	};

	SigGen redSig(red, NUM_SAM(red), 333, 0);
	SigGen greenSig(green, NUM_SAM(green), 333, 0);
	SigGen blueSig(blue, NUM_SAM(blue), 333, 0);

	redSig.start(4, fizzyMint->light + RED_LIGHT);
	greenSig.start(4, fizzyMint->light + GREEN_LIGHT);
	blueSig.start(4, fizzyMint->light + BLUE_LIGHT);

	fizzyMint->tunePlayer.playTune(welcomeTune, NUM_NOTE(welcomeTune), 800);
	tuneEnded = false;
	while (tuneEnded == false)
		update();

	greenSig.stop();
	blueSig.stop();
	redSig.stop();

	uint8_t roundWinner = calculateRoundWinner();

	static const SigGen::SAMPLE bellAlarm[] PROGMEM = { NOTE_G5_US, NOTE_B5_US, NOTE_D5_US };
	SigGen piezoSig(bellAlarm, NUM_SAM(bellAlarm), 90, SigGen::INTERPOLATE_OFF);
	piezoSig.start(0, &fizzyMint->buzzer);
	SigGen blipSig(blip, sizeof(blip), 200, 0);

	for (int i = 0; i<3; i++)
		fizzyMint->light[i].turnOff();

	if (roundWinner == PLAYER1)
	{
		blipSig.start(15, fizzyMint->light + GREEN_LIGHT);
	}
	else
	{
		blipSig.start(15, fizzyMint->light + BLUE_LIGHT);
	}

	while (blipSig.isRunning())
		update();

	for(int i=0;i<3;i++)
    	fizzyMint->light[i].turnOff();
	
	fizzyMint->buzzer.stop();
}


void loopCelebrateWinner()
{
	static const SigGen::SAMPLE fwSam[] PROGMEM = { 0,40,45,50,55,60,65,70,75,80,100,255,255,255,255,255,255,255,255,255 };
	static const SigGen::SAMPLE samStaticDuty[] PROGMEM = { 20,20,20,20,20,20,20,20,20,20,20,20,50,80,110,140,170,200,230,255 };
	static const SigGen::SAMPLE whizz[] PROGMEM = { NOTE_G7_US, NOTE_G5_US,NOTE_G4_US, NOTE_G3_US };
	static const SigGen::SAMPLE crackle[] PROGMEM = { NOTE_G6_US, NOTE_G4_US,NOTE_G5_US, NOTE_G3_US };
	SigGen sigGen[3];
	SigGen sigGenStaticDuty[3];
	StaticGen staticGen[3];
	SigStopper stopper[3];
	SigGen whizzSig(whizz, NUM_SAM(whizz), 2000, 0);
	whizzSig.configEventRef(10);
	SigGen crackleSig(crackle, NUM_SAM(crackle), 200, SigGen::INTERPOLATE_OFF);

	for (int i = 0; i < 3; i++) {
		sigGen[i].configSamples(fwSam, NUM_SAM(fwSam), 4000, 0);
		sigGen[i].configOutput(&stopper[i]);
		stopper[i].configOutput(fizzyMint->light + i);
		staticGen[i].configOutput(stopper[i].stopCtrl);
		staticGen[i].configPeriod_ms(20);
		staticGen[i].configDuty_256(230);

		sigGenStaticDuty[i].configSamples(samStaticDuty, NUM_SAM(samStaticDuty), 4000, 0);
	}

	Timer timer[3];
	Timer finishTimer(15000, 0xffff);
	for (int i = 0; i < 3; i++) {
		timer[i].startTimer(random(0, 2000), 0xffff);
	}

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
			}
		}

		if (whizzSigFinished == true) {
			crackleSig.start(10, &fizzyMint->buzzer);
			whizzSigFinished = false;
		}
		
		update();
	}

	for (int i = 0; i < 3; i++)
		fizzyMint->light[i].turnOff();
}

void playErrorEffect()
{
	NOTE fart[] = {
		{ NOTE_B3_US, 2 },
		{ NOTE_B2_US, 4 }
	};
	SigGen redSig(blip, NUM_SAM(blip), 200, 0);
	SigGen greenSig(blip, NUM_SAM(blip), 200, 0);
	SigGen blueSig(blip, NUM_SAM(blip), 200, 0);

	redSig.start(10, fizzyMint->light + RED_LIGHT);
	greenSig.start(10, fizzyMint->light + GREEN_LIGHT);
	blueSig.start(10, fizzyMint->light + BLUE_LIGHT);

	fizzyMint->tunePlayer.configReverb(120, 100);
	fizzyMint->tunePlayer.playTune(fart, NUM_NOTE(fart), 300);

	while (fizzyMint->tunePlayer.isRunning())
		update();
}


