
/*******************************************************************************
React.ino - Takes a FizzyMint and creates a reaction game.

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
#include "React.h"
#include "Arduino.h"

void setup()
{
	reactSetup();
}

void loop()
{
	reactLoop();
}
