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

/*
* We will be using the SweetMaker Software Development Kit and a fizzyMint so
* start by including the following files to import class definitions
*/

#include "FizzyMint.h"
#include "Bulb.h"
#include "SigLib.h"
#include "PerfMon.h"


using namespace SweetMaker;

void handleEvent(uint16_t eventId, uint8_t sourceInstance, uint16_t eventInfo);
void update(uint16_t elapsedTime_ms);

FizzyMint fizzyMint;
Bulb bulb;

void setup()
{
	Serial.begin(115200);
	Serial.println("Setup: Enter");

	fizzyMint.configEventHandlerCallback(handleEvent);
	fizzyMint.configLightBalance(true);

	bulb.setup(&fizzyMint);

	randomSeed(analogRead(0));
	Serial.println("Setup: Exit");
}


void loop()
{
	fizzyMint.update();
}


/*
* This is the eventHandler Callback
*/
void handleEvent(uint16_t eventId, uint8_t sourceRef, uint16_t eventInfo)
{
	switch (eventId)
	{
		case SwitchDriver::SWITCH_TURNED_ON:
		Serial.println(F("Switch Turned On"));
		break;

		case TimerTickMngt::TIMER_TICK_10S:
		PerfMon::getPerfMon()->intervalStop();
		PerfMon::getPerfMon()->print();
		PerfMon::getPerfMon()->reset();
		PerfMon::getPerfMon()->intervalStart();
		break;
	}

}


