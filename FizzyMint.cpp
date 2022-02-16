/*******************************************************************************
    FizzyMint.cpp: Class for controlling a FizzyMint Device

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

#include "FizzyMint.h"
#include "EventMngr.h"
#include "Updater.h"
#include "PerfMon.h"

#define HARDWARE_VERSION_1 (1)
#define HARDWARE_VERSION_2 (2)
#define HARDWARE_VERSION (HARDWARE_VERSION_2)


#define RED_LED_PIN_V1 (9)
#define BLUE_LED_PIN_V1 (10)
#define GREEN_LED_PIN_V1 (5)
#define GREEN_GND_V1 (6)
#define RED_GND_V1 (8)
#define BLUE_GND_V1 (11)

#define SWITCH_PIN_V1 (3)
#define SWITCH_GND_V1 (4)

#define PIEZO_PIN_V1 (A3)
#define PIEZO_GND_V1 (A2)

#define RED_LED_PIN_V2 (6)
#define BLUE_LED_PIN_V2 (11)
#define GREEN_LED_PIN_V2 (5)
#define GREEN_GND_V2 (4)
#define RED_GND_V2 (7)
#define BLUE_GND_V2 (12)

#define SWITCH_PIN_V2 (2)
#define SWITCH_GND_V2 (3)

#define PIEZO_PIN_V2 (A3)
#define PIEZO_GND_V2 (A2)

static IToneGen * tg;
/*
 * ::FizzyMint - Instantiation
 *
 * Sets IO pins correctly. Sets variables to default values.
 */
FizzyMint::FizzyMint()
{
	IToneGen * toneGen;
	/*
	* Ground Connection Pins
	*/
	uint8_t redLedGnd;
	uint8_t greenLedGnd;
	uint8_t blueLedGnd;
	uint8_t switchGnd;
	uint8_t piezoGnd;

	/*
	 * Signal Pins
	 */
	uint8_t redLedPin;
	uint8_t greenLedPin;
	uint8_t blueLedPin;
	uint8_t switchPin;
	uint8_t piezoPin;

	userEventHandlerCallback = NULL;
	userEventHandlerObject = NULL;

	/*
	 * Depending on the Hardware version different pins are used
	 */
    hardwareVersion = determineHardwareVersion();
    
	switch(hardwareVersion)
	{
		case HARDWARE_VERSION_1 :
		{
			redLedPin = RED_LED_PIN_V1;
			redLedGnd = RED_GND_V1;
			greenLedPin = GREEN_LED_PIN_V1;
			greenLedGnd = GREEN_GND_V1;
			blueLedPin = BLUE_LED_PIN_V1;
			blueLedGnd = BLUE_GND_V1;
			switchPin = SWITCH_PIN_V1;
			switchGnd = SWITCH_GND_V1;
			piezoPin = PIEZO_PIN_V1;
			piezoGnd = PIEZO_GND_V1;
			ToneGenT0	* tGen = new ToneGenT0();
			tGen->setup(piezoPin);
			toneGen = tGen;
		}
		break;

		case HARDWARE_VERSION_2 :
		{
			redLedPin = RED_LED_PIN_V2;
			redLedGnd = RED_GND_V2;
			greenLedPin = GREEN_LED_PIN_V2;
			greenLedGnd = GREEN_GND_V2;
			blueLedPin = BLUE_LED_PIN_V2;
			blueLedGnd = BLUE_GND_V2;
			switchPin = SWITCH_PIN_V2;
			switchGnd = SWITCH_GND_V2;
			piezoPin = PIEZO_PIN_V2;
			piezoGnd = PIEZO_GND_V2;
			FizzyMint::ToneGen	* tGen = new FizzyMint::ToneGen();
			tGen->setup();
			toneGen = tGen;
		}
		break;
	}

	/*
	 * Red Light
	 */
	pinMode(redLedGnd, OUTPUT);
	digitalWrite(redLedGnd, LOW);
	light[red].configPin(redLedPin);

	/*
	* Blue Light
	*/
	pinMode(blueLedGnd, OUTPUT);
	digitalWrite(blueLedGnd, LOW);
	light[blue].configPin(blueLedPin);

	/*
	* Green Light
	*/
	pinMode(greenLedGnd, OUTPUT);
	digitalWrite(greenLedGnd, LOW);
	light[green].configPin(greenLedPin);

	/*
	* Micro Switch
	*/
	pinMode(switchGnd, OUTPUT);
	digitalWrite(switchGnd, LOW);
	microSwitch.config(switchPin, HIGH);
	microSwitch.configEventHandler(this);

	/*
	* Piezo Element
	*/
	pinMode(piezoGnd, OUTPUT);
	digitalWrite(piezoGnd, LOW);
	buzzer.configToneGen(toneGen);
	buzzer.configListeningPin(piezoPin);
	buzzer.configEventHandler(this, 0);
	tg = toneGen;

	tunePlayer.configToneGen(&buzzer);
	tunePlayer.configEventHandler(this);
	
	/*
	 * Event Manager
	 */

	 EventMngr::getMngr()->configCallBack(this);
	 
	 /*
	  * Read Eeprom Config
	  */
	  {
	      EEPROM_DATA eeprom_data;
	      if(this->getEepromData(&eeprom_data) == 0)
	      {
	          this->colourBalance.red = eeprom_data.config.redLightScale;
	          this->colourBalance.green = eeprom_data.config.greenLightScale;
	          this->colourBalance.blue = eeprom_data.config.blueLightScale;
	      }
	      else
	      {
	          this->colourBalance.red = 255;
	          this->colourBalance.green = 255;
	          this->colourBalance.blue = 255;
	      }
	  }

	this->lastUpdateTime_ms = millis();
	TimerTickMngt::getTimerMngt()->update(0);
}

/*
 * ::configEventHandlerCallback - This callback is the FizzyMint Users way of
 *                             receiving all event indictions.
 */

void FizzyMint::configEventHandlerCallback(IEventHandler * callbackObject)
{
	userEventHandlerObject = callbackObject;
}

void FizzyMint::configEventHandlerCallback(IEventHandler::EventHandler callback)
{
	userEventHandlerCallback = callback;
}


void FizzyMint::configLightBalance(boolean on)
{
    if(on == true)
    {
	this->light[red].configLevelling(colourBalance.red);
	this->light[green].configLevelling(colourBalance.green);
	this->light[blue].configLevelling(colourBalance.blue);
    }
    else
    {
	this->light[red].configLevelling(255);
	this->light[green].configLevelling(255);
	this->light[blue].configLevelling(255);
    }
}

void FizzyMint::configLightBalance(uint8_t red, uint8_t green, uint8_t blue)
{
	this->light[red].configLevelling(red);
	this->light[green].configLevelling(green);
	this->light[blue].configLevelling(blue);
}


/*
 * ::update - checks for knocks, updates LEDs and tunes
 */
void FizzyMint::update()
{
	unsigned long thisTime_ms = millis();
	unsigned long elapsedTime_ms = thisTime_ms - lastUpdateTime_ms;

	PerfMon::getPerfMon()->intervalStop();
	PerfMon::getPerfMon()->intervalStart();

	AutoUpdateMngr::getUpdater()->update(elapsedTime_ms);

	lastUpdateTime_ms = thisTime_ms;
}

/*
 * This is the eventHandler Callback
 */
void FizzyMint::handleEvent(uint16_t eventId, uint8_t sourceRef, uint16_t eventInfo)
{
#ifdef SWEET_MAKER_DEBUG
	Serial.print("FMhe: ");
	Serial.print(sourceRef);
	Serial.print(" ");
	Serial.print(eventId);
	Serial.print(" ");
	Serial.println(eventInfo);
#endif
	if(userEventHandlerCallback)
		userEventHandlerCallback(eventId, sourceRef, eventInfo);

	if(userEventHandlerObject)
		userEventHandlerObject->handleEvent(eventId, sourceRef, eventInfo);
}

/*
 * ::delay - allows synchronous delays while continuing to service FizzyMint
 */
void FizzyMint::updateDelay(uint16_t duration_ms)
{
	unsigned long finishTime_ms = millis() + duration_ms;

	while (millis() < finishTime_ms)
		this->update();
}

void FizzyMint::helloWorld()
{

}

int FizzyMint::getEepromData(EEPROM_DATA * data)
{
#define FMED_PO_SCALE_RED_LIGHT		(0)
#define FMED_PS_SCALE_RED_LIGHT			(1)
#define FMED_PO_SCALE_GREEN_LIGHT	(1)
#define FMED_PS_SCALE_GREEN_LIGHT		(1)
#define FMED_PO_SCALE_BLUE_LIGHT	(2)
#define FMED_PS_SCALE_BLUE_LIGHT		(1)
#define FMED_PS_RESERVED1           (3)
#define FMED_PO_RESERVED1               (1)
#define FMED_PO_HARDWARE_VERSION	(4)
#define FMED_PS_HARDWARE_VERSION		(1)
#define FMED_PO_SOFTWARE_VERSION	(5)
#define FMED_PS_SOFTWARE_VERSION		(1)
#define FMED_PO_SERIAL_NUM			(6)
#define FMED_PS_SERIAL_NUM				(4)
#define FMED_PS_RESERVED2           (10)
#define FMED_PO_RESERVED2               (4)
#define FMED_PO_CRC_VALUE			(14)
#define FMED_PS_CRC_VALUE				(2)

	uint16_t storedCrc;
	uint16_t calculatedCrc;
	EepromUtility::EepromReader eepromReader(eeprom_data_offset, eeprom_data_length);
	/* Calculate and check CRC */
	calculatedCrc = EepromUtility::calculateCrc(FizzyMint::eeprom_data_offset, FizzyMint::eeprom_data_length -2);

	data->config.redLightScale = eepromReader.readU8();
	data->config.greenLightScale = eepromReader.readU8();
	data->config.blueLightScale = eepromReader.readU8();
	eepromReader.readU8();
	data->hardwareVersion = eepromReader.readU8();
	data->softwareVersion = eepromReader.readU8();
	data->serialNumber = eepromReader.readU32();
	eepromReader.readU32();
	storedCrc = eepromReader.readU16();

	if (storedCrc != calculatedCrc)
		return -1;
	return 0;
}


uint8_t FizzyMint::determineHardwareVersion(void)
{
	uint8_t version;
  
    // Set both pins low
	pinMode(BLUE_GND_V1, OUTPUT);
	digitalWrite(BLUE_GND_V1, LOW);
	
	pinMode(BLUE_LED_PIN_V1, OUTPUT);
	digitalWrite(BLUE_LED_PIN_V1, LOW);

	pinMode(BLUE_GND_V2, OUTPUT);
	digitalWrite(BLUE_GND_V2, LOW);

    // now set Blue Gnd to INPUT
	pinMode(BLUE_GND_V1, INPUT);

    // now set BlueLed+ HIGH
    digitalWrite(BLUE_LED_PIN_V1, HIGH);

	if(digitalRead(BLUE_GND_V1) == HIGH)
		version = HARDWARE_VERSION_1;
	else
		version = HARDWARE_VERSION_2;
		
	pinMode(BLUE_GND_V1, INPUT);
	pinMode(BLUE_LED_PIN_V1, INPUT);
	
	return version;
}

FizzyMint::ToneGen::ToneGen()
{
}

void FizzyMint::ToneGen::setup()
{
	volume = 0xff;
	AvrTimer1::setWaveformGenerationMode(
	    AvrTimer1::PWM_PHASE_AND_FREQUENCY_CORRECT_ICR,
	    AvrTimer1::PIN_DISCONNECTED,
	    AvrTimer1::PIN_DISCONNECTED
	);
}



/*
 * This configures the timer prescaler and TOP values to achieve the
 * requested frequency. The timer then automatically starts and will
 * toggle OC1A and OC1B
 *
 * The Master clock frequency is F_CPU and expected to be 16M or 8M
 * At 16M the lowest frequecy without prescaling is 16000000 / 0xffff = 244.14
 * With the use of PRESCALING_8 this allows us to reach 30.5Hz which allows
 * support for NOTE_B0 is 31Hz.
 *
 */
void FizzyMint::ToneGen::playFrequency_hz(uint16_t frequency, uint16_t duration_ms)
{
	uint32_t period_us;
	if(frequency == 0)
	{
		this->stop();
		return;
	}
	period_us = 1000000 / frequency;
	playPeriod_us(period_us, duration_ms);
}

void FizzyMint::ToneGen::playPeriod_us(uint32_t period_us, uint16_t duration_ms)
{
	uint32_t ocra;
	/*
	 * If period is 0 or less than NOTE_B8_US (127)
	 * Then don't play anything
	 */
	if(period_us < NOTE_B8_US)
	{
		AvrTimer1::setInterruptMaskRegister(0);
		AvrTimer1::setClockSelectMode(AvrTimer1::NO_CLOCK_SOURCE);
		return;
	}
	  
	
	AvrTimer1::setWaveformGenerationMode(
	    AvrTimer1::PWM_PHASE_AND_FREQUENCY_CORRECT_ICR,
	    AvrTimer1::PIN_DISCONNECTED,
	    AvrTimer1::PIN_DISCONNECTED
	);

	DDRC  = 0x0c;

	AvrTimer1::setPeriod_us(AvrTimer1::PWM_PHASE_AND_FREQUENCY_CORRECT_ICR, period_us);
	AvrTimer1::setInterruptMaskRegister(AvrTimer1::OUTPUT_COMPARE_A_MATCH_INTERRUPT_ENABLE);
	
	// set the duty cycle so as to control the volume;
	if (volume == 1) {
		ocra = 150;
	}
	else {
		ocra = AvrTimer1::getInputCaptureRegister() >> 1;
	}
	AvrTimer1::setOutputCompareRegisterA(ocra);
}

void FizzyMint::ToneGen::writeValue(int32_t period_us)
{
	playPeriod_us(period_us, 0);
}

void FizzyMint::ToneGen::stop(void)
{
	pinMode(PIEZO_GND_V2, OUTPUT);
	digitalWrite(PIEZO_GND_V2, LOW);
	pinMode(PIEZO_PIN_V2, INPUT_PULLUP);
	

	AvrTimer1::setClockSelectMode(AvrTimer1::NO_CLOCK_SOURCE);
	AvrTimer1::setInterruptMaskRegister(0);
}

void FizzyMint::ToneGen::setVolume(uint8_t new_volume)
{
	volume = new_volume;
}

#ifdef _FIZZY_MINT_
ISR(TIMER1_COMPA_vect)
{
	PORTC == 0x04 ? PORTC = 0x08 : PORTC = 0x04;
}
#endif




