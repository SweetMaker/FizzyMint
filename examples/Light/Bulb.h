/*******************************************************************************
Bulb.h - Takes a FizzyMint and creates various light effects.

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

#ifndef __BULB_H__
#define __BULB_H__

#include "FizzyMint.h"
#include "Updater.h"
#include "Colour.h"
#include "SigStopper.h"
#include "StaticGen.h"

namespace SweetMaker {
	class Bulb : private AutoUpdate
	{
	public:
		Bulb();
		void setup(FizzyMint * _fizzy);


	private:
		FizzyMint * fizzy;

		// Inherited via Updatable
		void update(uint16_t elapsedTime_ms);
		HsvToRgbSigConv hsvColour;

		SigGen hueSig;
		SigGen hueSpeedSig;

		SigGen satSig;
		SigScaler satSigScaler;

		SigGen valSig;
		SigScaler valSigScaler;
		SigStopper valSigStopper;
		StaticGen valStatic;

		uint16_t avgJumpPeriod_ms;
		uint16_t avgStaticOnPeriod_ms;

	};
}

#endif