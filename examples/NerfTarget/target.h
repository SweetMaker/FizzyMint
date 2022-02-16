/*******************************************************************************
Target.h - Takes a FizzyMint and creates a Target.

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
//typedef int FizzyMint;

#ifndef __TARGET_H__
#define __TARGET_H__

namespace SweetMaker {

	class Target : IEventHandler {
	public:
		Target(FizzyMint * fizzy);
		void setup(void);
		void loop();

	private:
		FizzyMint *fizzyMint;

		SigGen sineWave[3]; // used for driving lights
		SigGen buzzerSig; // used for driving buzzer

		// Inherited via IEventHandler
		virtual void handleEvent(uint16_t eventId, uint8_t sourceReference, uint16_t eventInfo) override;

		void waitForNewGame();
		void playStartTune();
		void startCountDown();
		void gameOn();
		void winnersFanfare();

		void flashLights();
		void flashLights(uint16_t numFlash);
		void flashLights(uint16_t numFlash, uint16_t period_ms);
		void flashLightsStaggered();
		void playHitNoise();

		bool switchPressedEvent;
		bool switchHeldEvent;
		bool piezoHitEvent;
		bool whizzSigFinished;
		bool newNoteEvent;

	};
}

#endif