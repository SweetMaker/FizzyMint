/*******************************************************************************
RecorderPractice.ino - Takes a FizzyMint and creates a recorder practice tool.

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
#include <FizzyMint.h>
FizzyMint fizzyMint;


void restartTune();

uint16_t bpm = 200;

uint8_t songNum = 0;
bool newNote = false;
bool switchPressed = false;
bool songNumChanged = false;
bool practiceFinished = false;
bool songFinished = false;

static const SigGen::SAMPLE blip[] PROGMEM = { 64,255,230,205,180,155,130,105,80,55,30,0 };
SigGen redSig(blip, NUM_SAM(blip), 700, 0);
SigGen greenSig(blip, NUM_SAM(blip), 700, 0);
SigGen blueSig(blip, NUM_SAM(blip), 700, 0);

uint8_t practiceCount_minutes = 0;
Timer practiceTimer;

NOTE scarecrow[] = {
	{ NOTE_B5_US, 4 },
	{ NOTE_C6_US, 2 },
	{ NOTE_B5_US, 4 },
	{ NOTE_A5_US, 2 },
	{ NOTE_G5_US, 4 },
	{ NOTE_E5_US, 2 },
	{ NOTE_D5_US, 4 },
	{ NOTE_REST_US, 2 },

	{ NOTE_A5_US, 2 },
	{ NOTE_B5_US, 2 },
	{ NOTE_C6_US, 2 },
	{ NOTE_B5_US, 4 },
	{ NOTE_G5_US, 2 },
	{ NOTE_A5_US, 4 },
	{ NOTE_REST_US, 2 },
	{ NOTE_B5_US, 4 },
	{ NOTE_REST_US, 2 },

	{ NOTE_G5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_A5_US, 2 },
	{ NOTE_G5_US, 4 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 4 },
	{ NOTE_G5_US, 2 },
	{ NOTE_B5_US, 4 },
	{ NOTE_G5_US, 2 },
	{ NOTE_D5_US, 4 },
	{ NOTE_D5_US, 2 },
	{ NOTE_F5_US, 4 },
	{ NOTE_E5_US, 2 },
	{ NOTE_D5_US, 4 },
	{ NOTE_REST_US, 2 },
	{ NOTE_G5_US, 4 },
	{ NOTE_REST_US, 2 },
};

NOTE dearLiza[] = {
	{ NOTE_G5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_B5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },

	{ NOTE_G5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_B5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_A5_US, 2 },
	{ NOTE_G5_US, 4 },

	{ NOTE_A5_US, 2 },
	{ NOTE_B5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },

	{ NOTE_A5_US, 2 },
	{ NOTE_B5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_D5_US, 2 },
	{ NOTE_E5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_A5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ 0, 2 }
};

NOTE tadpoles[] = {
	{ NOTE_C6_US, 2 },
	{ NOTE_C6_US, 1 },
	{ NOTE_C6_US, 1 },
	{ NOTE_A5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_A5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_C6_US, 1 },
	{ NOTE_C6_US, 1 },
	{ NOTE_G5_US, 4 },

	{ NOTE_E5_US, 2 },
	{ NOTE_E5_US, 1 },
	{ NOTE_E5_US, 1 },
	{ NOTE_D5_US, 1 },
	{ NOTE_D5_US, 1 },
	{ NOTE_D5_US, 2 },
	{ NOTE_B5_US, 1 },
	{ NOTE_B5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_G5_US, 4 },

	{ NOTE_A5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_C6_US, 1 },
	{ NOTE_C6_US, 1 },
	{ NOTE_B5_US, 2 },
	{ NOTE_G5_US, 2 },
	{ NOTE_A5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_A5_US, 1 },
	{ NOTE_G5_US, 2 },
	{ NOTE_G5_US, 1 },
	{ NOTE_E5_US, 1 },

	{ NOTE_D5_US, 1 },
	{ NOTE_D5_US, 1 },
	{ NOTE_D5_US, 1 },
	{ NOTE_D5_US, 1 },
	{ NOTE_E5_US, 1 },
	{ NOTE_G5_US, 1 },
	{ NOTE_G5_US, 1 },
	{ NOTE_E5_US, 1 },
	{ NOTE_A5_US, 2 },
	{ NOTE_B5_US, 2 },
	{ NOTE_C6_US, 4 }
};

void myEventHandler(uint16_t eventId, uint8_t src, uint16_t eventInfo)
{
	switch (eventId)
	{
		case TunePlayer::TUNE_NEXT_NOTE:
		newNote = true;
		break;

		case TunePlayer::TUNE_ENDED:
		songFinished = true;
		break;

		case SwitchDriver::SWITCH_TURNED_ON:
		switchPressed = true;
		Serial.println("Switch Turned On");
		break;

		case SwitchDriver::SWITCH_HELD_ON:
		case SwitchDriver::SWITCH_STILL_HELD_ON:
		Serial.print("SWITCH_HELD_ON");
			songNum++;
			if (songNum == 3)
				songNum = 0;
			songNumChanged = true;

		Serial.println(songNum);
		greenSig.start(1, fizzyMint.light + GREEN_LIGHT);
		redSig.start(1, fizzyMint.light + RED_LIGHT);
		blueSig.start(1, fizzyMint.light + BLUE_LIGHT);
		break;

		case TimerTickMngt::TIMER_EXPIRED:
		Serial.println("TimerExpired");
		practiceCount_minutes++;
		if (practiceCount_minutes == 5)
		{
			practiceFinished = true;
			practiceCount_minutes = 0;
		}
		practiceTimer.startTimer(60000, 0xffff);;

		break;

	}

}

void setup()
{
	Serial.begin(112500);
	Serial.println("Recoder Practice");
	fizzyMint.configEventHandlerCallback(myEventHandler);
	fizzyMint.tunePlayer.configReverb(5, 150);

	practiceTimer.startTimer(60000, 0xffff);
	redSig.start(0, fizzyMint.light + RED_LIGHT);
}

void loop()
{
	static uint8_t ledSeq = 0;

	if (switchPressed == true) {
		practiceFinished = false;
		restartTune();
		switchPressed = false;
	}

	if (songNumChanged == true) {
		songNumChanged = false;
		restartTune();
	}

	if (newNote == true) {
		switch (ledSeq++) {
			case 0:	greenSig.start(1, fizzyMint.light + GREEN_LIGHT);	break;
			case 1:	redSig.start(1, fizzyMint.light + RED_LIGHT); break;
			case 2: blueSig.start(1, fizzyMint.light + BLUE_LIGHT); ledSeq = 0; break;
		}
		newNote = false;
	}

	if (practiceFinished == true) {
		fizzyMint.tunePlayer.stop();
		playFinishedEffect();
	}

	if (songFinished == true) {
		songFinished = false;
		redSig.start(0, fizzyMint.light + RED_LIGHT);
	}

	fizzyMint.update();
}


void restartTune() {
	fizzyMint.tunePlayer.stop();
	switch (songNum) {
		case 0:
		fizzyMint.tunePlayer.playTune(scarecrow, NUM_NOTE(scarecrow), bpm);
		break;

		case 1:
		fizzyMint.tunePlayer.playTune(tadpoles, NUM_NOTE(tadpoles), bpm);
		break;

		case 2:
		fizzyMint.tunePlayer.playTune(dearLiza, NUM_NOTE(dearLiza), bpm);
		break;
	}
}


void playFinishedEffect()
{
	SigGen::SAMPLE descendingNote[2] = { NOTE_G7_US, NOTE_G6_US };
	SigGen piezoSig(descendingNote, NUM_SAM(descendingNote), 500, SigGen::DONT_USE_PROGMEM);
	piezoSig.start(1, &fizzyMint.buzzer);

	SigGen::SAMPLE lightsDown[] = { 255,50 };
	SigGen lightsDownSig[3];

	for (int i = 0; i < 3; i++)
		lightsDownSig[i].configSamples(lightsDown, NUM_SAM(lightsDown), 1000, SigGen::DONT_USE_PROGMEM);

	for (int i = 0; i < 6; i++) {
		uint8_t lightNum = random(0, 3);
		lightsDownSig[lightNum].start(1, &fizzyMint.light[lightNum]);
		piezoSig.start(1);

		while (piezoSig.isRunning())
			fizzyMint.update();

		descendingNote[0] = (descendingNote[0] >> 1) * 3;
		descendingNote[1] = (descendingNote[1] >> 1) * 3;
	}

	fizzyMint.buzzer.stop();
}