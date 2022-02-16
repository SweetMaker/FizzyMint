
/*******************************************************************************
RayGun.ino - Takes a FizzyMint and turns it into a Gun .

Copyright(C) 2016  Howard James May

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
#include "RayGun.h"

using namespace SweetMaker;

FizzyMint myFizzyMint;
RayGun rayGun;

void setup()
{
	Serial.begin(115200);
	Serial.print("RayGun Size: "); Serial.println(sizeof(RayGun));
	Serial.println("Starting Setup");
	rayGun.setup(&myFizzyMint);
	Serial.println("Finishing Setup");
}

void loop()
{
	myFizzyMint.update();
	rayGun.update();
}

