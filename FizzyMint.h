/*******************************************************************************
FizzyMint.h Class for controlling a FizzyMint Device

	Copyright(C) 2015-2016  Howard James May

	This file is part of the SweetMaker SDK

	The SweetMaker SDK is free software: you can redistribute it and / or
	modify it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	The SweetMaker SDK is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see <http://www.gnu.org/licenses/>.

	Contact me at sweet.maker@outlook.com
********************************************************************************
Release     Date                        Change Description
--------|-------------|--------------------------------------------------------|
   1      24-Feb-2016   Initial release
*******************************************************************************/


#ifndef __FIZZY_MINT_H__
#define __FIZZY_MINT_H__


#include "Arduino.h"
#include "SwitchDriver.h"
#include "SigGen.h"
#include "LedDriver.h"
#include "PiezoDriver.h"
#include "TunePlayer.h"
#include "TimerMngt.h"
#include "ToneGenT1.h"
#include "ToneGenT0.h"
#include "EepromUtility.h"
#include "Colour.h"

using namespace SweetMaker;

/*
 * Enable this for Serial Debug
 */
//#define SWEET_MAKER_DEBUG

class FizzyMint: public IEventHandler
{

	/*
	 * These class methods provide the public interface to FizzyMint which you
	 * use in your program.
	 */
public:
	/*
	 * Use this to create and setup FizzyMint once at startup
	 */
	FizzyMint();

     #define RED_LIGHT 0
	 #define GREEN_LIGHT 1
	 #define BLUE_LIGHT 2
	
	/*
	* Fizzy Mint uses a part of the EEPROM memory located at the end of the address space.
	*/
	static const uint16_t eeprom_data_length = 16;
	static const uint16_t eeprom_data_offset = 0x400 - eeprom_data_length;

	typedef struct eepromData{
		uint8_t hardwareVersion;
		uint8_t softwareVersion;
		uint32_t serialNumber;
		struct {
			uint8_t redLightScale;
			uint8_t greenLightScale;
			uint8_t blueLightScale;
		}config;
	}EEPROM_DATA;	

	void configEventHandlerCallback(EventHandler callbackFunction);
	void configEventHandlerCallback(IEventHandler * callbackObject);
	void configLightBalance(boolean);
	void configLightBalance(uint8_t red, uint8_t green, uint8_t blue);

	/*
	 * Call one of these frequently to allow FizzyMint to work properly
	 */
	void update();
	void updateDelay(uint16_t duration_ms); // wait for 'duration' milliseconds.
	int getEepromData(EEPROM_DATA * data);
	void helloWorld();

	/*
	* Attributes of FizzyMint
	*/
	typedef enum {
		red = 0,
		green = 1,
		blue = 2
	}LightNum;

	LedDriver    light[3];
	PiezoDriver  buzzer;
	TunePlayer   tunePlayer;
	SwitchDriver microSwitch;

private:
	/*
	 * These class methods are only for use by FizzyMint itself
	 */
	uint8_t hardwareVersion;
	unsigned long lastUpdateTime_ms;

	EventHandler userEventHandlerCallback;
	IEventHandler * userEventHandlerObject;
	void handleEvent(uint16_t eventId, uint8_t sourceInst, uint16_t eventInfo);
	uint8_t determineHardwareVersion(void);
	
	ColourRGB colourBalance;
	
	/*
	 * Fizzy Mint has it's own ToneGen class dedicated to it's pinout
	 */
	class ToneGen : public IToneGen {
	public:
	    ToneGen();
	    void setup();
	    
	    void playPeriod_us(uint32_t period_us, uint16_t duration_ms);
		void playFrequency_hz(uint16_t frequency_hz, uint16_t duration_ms);
		void writeValue(int32_t period_us);
		void setVolume(uint8_t volume);
		void stop(void);
		
		private:
		uint8_t volume;
		uint16_t Cv;
	};

};

#endif


