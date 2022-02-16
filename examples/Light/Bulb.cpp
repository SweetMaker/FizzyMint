/*******************************************************************************
Bulb.cpp - Takes a FizzyMint and creates various light effects.

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

#include "Bulb.h"
#include "SigLib.h"

using namespace SweetMaker;

Bulb::Bulb()
{
}

void Bulb::setup(FizzyMint * _fizzy)
{
	fizzy = _fizzy;

	hsvColour.configRedOutput(&fizzy->light[RED_LIGHT]);
	hsvColour.configGreenOutput(&fizzy->light[GREEN_LIGHT]);
	hsvColour.configBlueOutput(&fizzy->light[BLUE_LIGHT]);


	/* Hue Configuration */
	hueSig.configSamples(triangleWave255, NUM_SAM(triangleWave255), 5000, 0);
	hueSig.start(0, hsvColour.hueInput);

	const static SigGen::SAMPLE hueSpeedSam[] PROGMEM = { 5000,5000,1000,3000,8000 };
	hueSpeedSig.configSamples(hueSpeedSam, NUM_SAM(hueSpeedSam), 60000, 0);
	hueSpeedSig.start();

	/* Saturation Configuration */
	satSig.configSamples(sineWave255, NUM_SAM(sineWave255), 20000, 0);
	satSig.start(0, &satSigScaler);
	satSigScaler.configOutput(hsvColour.saturationInput);
	satSigScaler.configLinearScale(128, 64);

	/* Value Configuration */
	valSig.configSamples(breathingWave, NUM_SAM(breathingWave), 7000, 0);
	valSig.start(0, &valSigScaler);
	valSigScaler.configOutput(&valSigStopper);
	valSigScaler.configLinearScale(128, 128);
	valSigStopper.configOutput(hsvColour.valueInput);
	valStatic.configOutput(valSigStopper.stopCtrl);
	valStatic.configDuty_256(20);
	valStatic.configPeriod_ms(1);

	avgJumpPeriod_ms = 5000;
	avgStaticOnPeriod_ms = 10000;
}

void Bulb::update(uint16_t elapsedTime_ms)
{
	hsvColour.update();

	if (random(0, avgJumpPeriod_ms) < elapsedTime_ms)
	{
		/* Jump */
		hueSig.update(2500);
		hueSpeedSig.update(20000);
		satSig.update(3000);

		/* And turn static off*/
		valStatic.configDuty_256(0);
	}

	if (random(0, avgStaticOnPeriod_ms) < elapsedTime_ms)
	{
		valStatic.configDuty_256(20);
	}

	hueSig.configPeriod_ms(hueSpeedSig.readValue());
}
