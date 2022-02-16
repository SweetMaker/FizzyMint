
/*******************************************************************************
RayGun.cpp - Takes a FizzyMint and turns it into a Gun .

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

#include "RayGun.h"
#include "SigLib.h"
#include "SigDebug.h"


using namespace SweetMaker;

FizzyMint * fizzyMint;

RayGun::RayGun()
{

}

void RayGun::setup(FizzyMint * _fizzyMint)
{
	Serial.println("RayGun::setup");
	fizzyMint = _fizzyMint;
	fizzyMint->configEventHandlerCallback(&control);
	control.configEventHandler(&model);
	model.configEventHandler(&view);
	view.configModel(&model);
	
	lastUpdateTime_ms = millis();

	model.handleEvent(SYSTEST_START, 0, 0);
	model.handleEvent(TURNED_ON, 0, 0);
}

void RayGun::update()
{
	unsigned long thisTime_ms = millis();
	unsigned long elapsedTime_ms = thisTime_ms - lastUpdateTime_ms;

	model.update(elapsedTime_ms);
	view.update(elapsedTime_ms);

	lastUpdateTime_ms = thisTime_ms;
}

RayGun::Control::Control()
{
	rayGunOn = true;
	model = NULL;
}

void RayGun::Control::configEventHandler(IEventHandler * _model)
{
	model = _model;
}

void RayGun::Control::handleEvent(uint16_t eventType, uint8_t sourceRef, uint16_t eventInfo)
{
	if (eventType == SwitchDriver::SWITCH_TURNED_ON)
		eventType = TRIGGER_PRESSED;

	if (eventType == SwitchDriver::SWITCH_STILL_HELD_ON) {
		if (rayGunOn) {
			eventType = TURNED_ON;
			rayGunOn = false;
		}
		else {
			eventType = TURNED_OFF;
			rayGunOn = true;
		}
	}
	
	model->handleEvent(eventType, sourceRef, eventInfo);
}

RayGun::Model::Model()
{
	view = NULL;
	state = RAYGUN_STATE::Idle;
	cap.configGenerator(&gen);
	laz.configCapacitor(&cap);
}

void RayGun::Model::configEventHandler(IEventHandler * _view)
{
	view = _view;
}

void RayGun::Model::update(uint16_t elapsedTime_ms)
{
	laz.update(elapsedTime_ms);
}

void RayGun::Model::handleEvent(uint16_t eventType, uint8_t sourceRef, uint16_t eventInfo)
{
	debugHelper(eventType, sourceRef, eventInfo);

	if (eventType == TimerTickMngt::TIMER_TICK_100MS)
	{
		gen.processTick_100ms();
		cap.processTick_100ms();
		laz.processTick_100ms();
		view->handleEvent(eventType, sourceRef, eventInfo);
	}
	else
	{
		delay(10);
	}

	switch (state)
	{
		case Idle:
		{
			switch (eventType)
			{
				case SYSTEST_START:
				view->handleEvent(SYSTEST_START, 0, 0);
				timer.countDown_ms = 2000;
				timer.eventInfo = 0;
				fizzyMint->timerMngt.startTimer(&timer);
				Serial.println("STATE: SystemTest");
				state = SystemTest;
				break;
			}
		}
		break;
		
		case SystemTest:
		{
			switch (eventType)
			{
				case TimerTickMngt::TIMER_EXPIRED:
				Serial.println("STATE: Standby");
				view->handleEvent(ENTER_STANDBY, 0, 0);
				state = Standby;
				break;

			}
		}
		break;

		case Standby:
		{
			switch (eventType)
			{
				case TURNED_ON:
				Serial.println("STATE: LeavingStandby");
				view->handleEvent(LEAVE_STANDBY, 0, 0);
				timer.countDown_ms = 3000;
				timer.eventInfo = 0;
				fizzyMint->timerMngt.startTimer(&timer);
				state = LeavingStandby;
				break;
			}
		}
		break;

		case LeavingStandby:
		{
			switch (eventType)
			{
				case TimerTickMngt::TIMER_EXPIRED:
				Serial.println("STATE: Priming");
				view->handleEvent(ENTER_PRIMING, 0, 0);
				state = Priming;
				cap.turnOn();
				break;
			}
		}
		break;

		case Priming:
		{
			switch (eventType)
			{
				case TimerTickMngt::TIMER_TICK_100MS:
				{
					if (cap.isCharged() && !laz.isTurnedOn())
					{
						view->handleEvent(CAP_CHARGED, 0, 0);
						laz.turnOn();
						view->handleEvent(LAZ_TURNED_ON, 0, 0);
					}

					if (laz.isPrimed())
					{
						Serial.println("STATE: ReadyToFire");
						view->handleEvent(READY_TO_FIRE, 0, 0);
						state = ReadyToFire;
					}
				}
				break;
			}
		}
		break;

		case ReadyToFire:
		{
			switch (eventType)
			{
				case TimerTickMngt::TIMER_TICK_100MS:
				{
					if (cap.isLowInEnergy())
						gen.setRequestPower(gen.ratedPowerOutput_W);
					else if (cap.isFullyCharged())
						gen.setRequestPower(0);
				}
				break;

				case RayGun::TRIGGER_PRESSED:
				{
					laz.fire();
					Serial.println("STATE: FIRING");
					view->handleEvent(FIRING, 0, 0);
					state = Firing;
				}
				break;

				case RayGun::TURNED_OFF:
				{
					Serial.println("STATE: STANDBY");
					state = Standby;
					cap.turnOff();
					laz.turnOff();
				}
				break;
			}
		}
		break;

		case Firing:
		{
			switch (eventType)
			{
				case TimerTickMngt::TIMER_TICK_100MS:
				{
					if (!laz.isFiring())
					{
						Serial.println("STATE: READY TO FIRE");
						state = ReadyToFire;
					}
				}
				break;
			}
		}
		break;
	}

}


void RayGun::Model::debugHelper(uint16_t eventType, uint8_t src, uint16_t info)
{
	if ((eventType != TimerTickMngt::TIMER_TICK_100MS) &&
		(eventType != TimerTickMngt::TIMER_TICK_S) &&
		(eventType != TimerTickMngt::TIMER_TICK_UPDATE))
	{
		Serial.print("E: ");
		Serial.println(eventType);
	}

	if (eventType == TimerTickMngt::TIMER_TICK_10S)
	{
		Serial.print("State: ");
		Serial.print(state);
		Serial.print(" Gen: ");
		Serial.print(gen.getRequestedPower(), HEX);
		Serial.print(" ");
		Serial.print(gen.getCurrentPower(), HEX);
		Serial.print(" Gt: ");
		Serial.print(gen.getTemp_mC() / 1000);
		Serial.print(" Cap: ");
		Serial.print(cap.getEnergyLevel(), HEX);
		Serial.print(" Lt ");
		Serial.println(laz.getTemp_mC() / 1000);
	}

	if (eventType == SwitchDriver::SWITCH_TURNED_ON)
	{
	}

	if (Serial.available())
	{
		char cmd = Serial.read();
		switch (cmd)
		{
			case 'o':
			cap.jumpStart();
			laz.jumpStart();
			break;
			case 's':
			fizzyMint->buzzer.stop();
			break;
			case 'k':
			break;
			case 'l':
			break;
		}
	}

}



RayGun::Model::Generator::Generator()
{
	requestedPower_W = 0;
	currentPower_W = 0;
	temperature_mC = ambientTemperature_mC;
}

void RayGun::Model::Generator::processTick_100ms()
{
	if (currentPower_W < requestedPower_W)
		currentPower_W += maxRateOfChangeOfPower_W/10;

	if (currentPower_W > requestedPower_W)
		currentPower_W -= maxRateOfChangeOfPower_W/10;

	temperature_mC += (currentPower_W / heatCapacity_CJ);
	temperature_mC -= (temperature_mC - ambientTemperature_mC ) >> 8;
}

void RayGun::Model::Generator::setRequestPower(uint32_t power_watts)
{
	requestedPower_W = power_watts;
}

uint32_t SweetMaker::RayGun::Model::Generator::getRequestedPower()
{
	return requestedPower_W;
}

uint32_t RayGun::Model::Generator::getCurrentPower()
{
	return currentPower_W;
}

uint8_t SweetMaker::RayGun::Model::Generator::getFanLevel()
{
	return temperature_mC>>14;
}

uint32_t SweetMaker::RayGun::Model::Generator::getTemp_mC()
{
	return temperature_mC;
}

RayGun::Model::Capacitor::Capacitor()
{
	gen = NULL;
	energyLevel_J = 0;
	isTurnedOn = false;
}

void SweetMaker::RayGun::Model::Capacitor::configGenerator(Generator * g)
{
	gen = g;
}

void SweetMaker::RayGun::Model::Capacitor::processTick_100ms()
{
	if (!isTurnedOn)
		return;

	energyLevel_J += gen->getCurrentPower() / 10;

	if (isFullyCharged())
		gen->setRequestPower(0);

	if (isLowInEnergy())
		gen->setRequestPower(gen->ratedPowerOutput_W);
}

uint32_t SweetMaker::RayGun::Model::Capacitor::getEnergyLevel()
{
	return energyLevel_J;
}

void RayGun::Model::Capacitor::putEnergy(uint32_t energy)
{
	energyLevel_J += energy;
}

uint32_t RayGun::Model::Capacitor::drawEnergy(uint32_t request_J)
{
	if (energyLevel_J > request_J)
	{
		energyLevel_J -= request_J;
		return request_J;
	}
	else
	{
		request_J = energyLevel_J;
		energyLevel_J = 0;
		return request_J;
	}
}


bool RayGun::Model::Capacitor::isFullyCharged()
{
	if (energyLevel_J >= targetEngyLevel_J)
		return true;
	return false;
}

void SweetMaker::RayGun::Model::Capacitor::turnOn()
{
	isTurnedOn = true;
}

void SweetMaker::RayGun::Model::Capacitor::turnOff()
{
	isTurnedOn = false;
	energyLevel_J = 0;
}

void SweetMaker::RayGun::Model::Capacitor::jumpStart()
{
	energyLevel_J = targetEngyLevel_J;
	isTurnedOn = true;
}

bool RayGun::Model::Capacitor::isLowInEnergy()
{
	if (energyLevel_J <= taregtLvlHysteresis_J)
		return true;
	return false;
}

bool RayGun::Model::Capacitor::isCharged()
{
	if (energyLevel_J >= readyEnergyLevel_J)
		return true;
	return false;
}


RayGun::Model::Lazer::Lazer()
{
	static SigGen::SAMPLE lazBoltEnergyPulse_mW[] = { 0,200,255,255,200,150,100,50 };
	turnedOn = false;
	primed = false;
	firing = false;
	temperature_mC = ambientTemperature_mC;
	drainRate = 0;
	boltPulse_mW.configSamples(lazBoltEnergyPulse_mW, NUM_SAM(lazBoltEnergyPulse_mW), 500);
	boltPulse_mW.configOptions(0);
	boltPulse_mW.configOutput(NULL);
}

void RayGun::Model::Lazer::configEventHandler(IEventHandler * handler)
{
	boltPulse_mW.configEventHandler(handler, LAZ_OSC);
}

void RayGun::Model::Lazer::configCapacitor(Capacitor * c)
{
	cap = c;
}

void RayGun::Model::Lazer::processTick_100ms()
{
	if (turnedOn)
	{
		uint32_t energy = 0;
		if (primed)
			energy = idleDrainRate_W / 10;
		else
		{
			energy = primeDrainRate_W / 10;
			if (temperature_mC > primedTemp_mC)
				primed = true;
		}
		temperature_mC += cap->drawEnergy(energy) / heatCapacity_CJ;
	}
	
	temperature_mC -= (temperature_mC - ambientTemperature_mC) >> 8;
}

void RayGun::Model::Lazer::update(uint16_t elapsedTime_ms)
{
	if (firing)
	{
		int32_t boltValue;
		uint32_t energyPulse;
		boltPulse_mW.update(elapsedTime_ms);
		boltValue = boltPulse_mW.readValue();
		energyPulse = boltValue * elapsedTime_ms * 300;
		energyPulse = cap->drawEnergy(energyPulse);

		if (!boltPulse_mW.isRunning())
			firing = false;
	}
}

void RayGun::Model::Lazer::turnOn()
{
	turnedOn = true;
}

void SweetMaker::RayGun::Model::Lazer::turnOff()
{
	turnedOn = false;
	primed = false;
}
	

bool RayGun::Model::Lazer::isTurnedOn()
{
	return turnedOn;
}

bool RayGun::Model::Lazer::isPrimed()
{
	return primed;
}

void RayGun::Model::Lazer::fire()
{
	firing = true;
	boltPulse_mW.start(1);
}

bool SweetMaker::RayGun::Model::Lazer::isFiring()
{
	return firing;
}

uint32_t SweetMaker::RayGun::Model::Lazer::getTemp_mC()
{
	return temperature_mC;
}

uint32_t SweetMaker::RayGun::Model::Lazer::getBoltPulse_mW()
{
	return boltPulse_mW.readValue();
}

void SweetMaker::RayGun::Model::Lazer::jumpStart()
{
	temperature_mC = primedTemp_mC;
	primed = true;
	turnedOn = true;

}

/******************************************************************** 
*********************************************************************
*********************************************************************
*/


RayGun::View::View()
{
	fizzyMint = NULL;
	model = NULL;
}

void RayGun::View::configModel(Model * mod)
{
	model = mod;
}

void SweetMaker::RayGun::View::handleEvent(uint16_t eventType, uint8_t sourceRef, uint16_t eventInfo)
{
			switch (eventType)
			{
				case TimerTickMngt::TIMER_TICK_100MS:
				{
					processTick_100ms();
				}
				break;

				case SYSTEST_START:
				{
					configCatherineWheel();
				}
				break;

				case ENTER_STANDBY:
				{
					configGreenBlip();
				}
				break;

				case LEAVE_STANDBY:
				{
					configSpeedyBlip();
				}
				break;

				case ENTER_PRIMING:
				{
					configPriming();
				}
				break;


				case FIRING:
				{
					configFiring();
				}
				break;

				case FIRING_DONE:
				{
					fizzyMint->buzzer.setVolume(1);
				}



			}
}

void RayGun::View::processTick_100ms()
{

	switch (model->state)
	{
		case Firing:
		{
			int32_t boltPower = model->laz.getBoltPulse_mW();
			SigScaler * scaler = &sigScaler[4];
			scaler->configLinearScale(boltPower * -2, NOTE_C6_US);
		}
		// Fall through

		case Priming:
		case ReadyToFire:
		{
			SigGen * lazOscillator = &sigGen[0];
			SigGen * genFanOscillator = &sigGen[1];
			SigScaler * lazHueScaler = &sigScaler[0];
			SigScaler * genFanScaler = &sigScaler[1];
			SigScaler * lazValueScaler = &sigScaler[2];

			StaticGen * lazStatic = &staticGen[0];
			StaticGen * capStatic = &staticGen[1];

    		uint32_t oscillatorPeriod_ms = 3600 - ((uint32_t)model->gen.getFanLevel() * 800);
			genFanOscillator->configPeriod_ms(oscillatorPeriod_ms);

			int32_t nrg = 40 + (model->cap.getEnergyLevel() >> 15);
			genFanScaler->configLinearScale(128, (int16_t)nrg);

			capStatic->configPeriod_ms(model->gen.getCurrentPower() >> 10);
			capStatic->configDuty_256(model->gen.getCurrentPower() >> 11);

			lazValueScaler->configLinearScale(model->laz.getTemp_mC() >> 9, 0);

		}
		break;
	}

}

void RayGun::View::update(uint16_t elapsedTime_ms)
{
	switch (model->state)
	{

		case Firing:
		case Priming:
		case ReadyToFire:
		{

			for (int i = 0; i < 3; i++) {
				sigGen[i].update(elapsedTime_ms);
			}

			for (int i = 0; i < 3; i++)
				staticGen[i].update(elapsedTime_ms);

			lazCconv.saturationInput->writeValue(255);

			if (model->state == Firing)
			{
				uint32_t pulseStrength = model->laz.getBoltPulse_mW();
				lazCconv.valueInput->writeValue((int32_t)pulseStrength);
				lazCconv.hueInput->writeValue(200);

				SigGen * buzSigGen = &sigGen[3];
				buzSigGen->update(elapsedTime_ms);
			}
			else
			{
				ISigInput * buz = (ISigInput*)&sigScaler[3];
				SigGen * genFanOscillator = &sigGen[1];

				uint32_t energyLevel = model->cap.getEnergyLevel();
				energyLevel = energyLevel >> 8;
				energyLevel += (genFanOscillator->readValue() * 2);
				buz->writeValue(energyLevel);
			}

			lazCconv.update();

		}
		break;

		case SystemTest:
		case Standby:
		case LeavingStandby:
//		default:
		for (int i = 0; i < 4; i++) {
			sigGen[i].update(elapsedTime_ms);
		}
		break;
	}
}

void SweetMaker::RayGun::View::configCatherineWheel()
{
	static SigGen::SAMPLE buzzerSig[] = { NOTE_C5_US,NOTE_C5_US, 0, NOTE_G5_US,NOTE_G5_US, 0,  NOTE_D5_US,NOTE_D5_US, 0,  NOTE_D5_US,NOTE_D5_US, 0, 0, NOTE_C7_US, 0,NOTE_C7_US, 0,NOTE_C7_US, 0,NOTE_C7_US, 0,NOTE_C7_US, 0 };
	static SigGen::SAMPLE ledFlash[] = {0, 255 };

	SigGen * buzGen;
	Serial.println("View::configCatherineWheel");
	sigScaler[0].configOutput(&fizzyMint->redLight);
	sigScaler[1].configOutput(&fizzyMint->greenLight);
	sigScaler[2].configOutput(&fizzyMint->blueLight);
	for (int i = 0; i < 3; i++) {
		sigScaler[i].configLinearScale(128, 0);
		sigGen[i].configSamples(ledFlash, NUM_SAM(ledFlash), 500 * (i+ 1));
		sigGen[i].configOutput(&sigScaler[i]);
		sigGen[i].start();
	}
	buzGen = &sigGen[3];
	buzGen->configSamples(buzzerSig, NUM_SAM(buzzerSig), 2000);
	buzGen->configOptions(SigGen::INTERPOLATE_OFF);
	buzGen->configOutput(&fizzyMint->buzzer);
	buzGen->start(1);
	fizzyMint->buzzer.setVolume(1);
}

void SweetMaker::RayGun::View::configGreenBlip()
{
	static SigGen::SAMPLE blip[] = { 255,0,0,0,0,0,0,0, 0,0,0,0,0,0,0 };
	sigScaler[0].configOutput(&fizzyMint->redLight);
	sigScaler[1].configOutput(&fizzyMint->greenLight);
	sigScaler[2].configOutput(&fizzyMint->blueLight);
	for (int i = 0; i < 3; i++) {
		sigScaler[i].configLinearScale(128, 0);
		sigGen[i].configSamples(blip, NUM_SAM(blip), 4000);
		sigGen[i].configOutput(&sigScaler[i]);
		sigGen[i].stop();
	}
	fizzyMint->redLight.turnOff();
	fizzyMint->blueLight.turnOff();
	sigGen[1].start();

	static SigGen::SAMPLE buzzerSig[] = { NOTE_C5_US,0,NOTE_G5_US,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	SigGen * buzGen = &sigGen[3];
	buzGen->configSamples(buzzerSig, NUM_SAM(buzzerSig), 4000);
	buzGen->configOptions(SigGen::INTERPOLATE_OFF);
	buzGen->configOutput(&fizzyMint->buzzer);
	buzGen->start(0);
	buzGen->update(100);
	sigGen[1].update(100);

	fizzyMint->buzzer.setVolume(1);
}


void SweetMaker::RayGun::View::configSpeedyBlip()
{
	static SigGen::SAMPLE blip[] = { 255,0,0,0,0,0,0,0 };
	sigScaler[0].configOutput(&fizzyMint->redLight);
	sigScaler[1].configOutput(&fizzyMint->greenLight);
	sigScaler[2].configOutput(&fizzyMint->blueLight);
	for (int i = 0; i < 3; i++) {
		sigScaler[i].configLinearScale(255, 0);
		sigGen[i].configSamples(blip, NUM_SAM(blip), 400);
		sigGen[i].configOutput(&sigScaler[i]);
		sigGen[i].stop();
	}
	fizzyMint->redLight.turnOn();
	fizzyMint->blueLight.turnOn();
	sigGen[1].start(3);

	static SigGen::SAMPLE buzzerSig[] = { NOTE_C5_US, NOTE_D5_US, NOTE_G5_US };
	SigGen * buzGen = &sigGen[3];
	buzGen->configSamples(buzzerSig, NUM_SAM(buzzerSig), 200);
	buzGen->configOptions(SigGen::INTERPOLATE_OFF);
	buzGen->configOutput(&fizzyMint->buzzer);
	buzGen->start(5);
	fizzyMint->buzzer.setVolume(1);

}

void SweetMaker::RayGun::View::configPriming()
{
	SigGen * lazOscillator = &sigGen[0];
	SigGen * genFanOscillator = &sigGen[1];

	SigScaler * lazHueScaler = &sigScaler[0];
	SigScaler * genFanScaler = &sigScaler[1];
	SigScaler * lazValueScaler = &sigScaler[2];
	SigScaler * buzValueScaler = &sigScaler[3];

	StaticGen * lazStatic = &staticGen[0];
	StaticGen * capStatic = &staticGen[1];
	StaticGen * buzStatic = &staticGen[2];

	SigStopper * genStopper = &stoppers[0];
	SigStopper * buzStopper = &stoppers[1];

	lazOscillator->configSamples(sineWave255, NUM_SAM(sineWave255), 3000);
	lazOscillator->configEventHandler(this, SIG_GEN_REF::LAZ_OSC);
	lazOscillator->configOutput(lazHueScaler);
	lazOscillator->start();

	lazHueScaler->configLinearScale(255, 0);
	lazHueScaler->configOutput(lazCconv.hueInput);

	lazStatic->configPeriod_ms(4);
	lazStatic->configDuty_256(200);
	lazStatic->configOutput(lazValueScaler);

	lazValueScaler->configOutput(lazCconv.valueInput);
	lazValueScaler->configLinearScale(20, 60, 30, 128);

	lazCconv.configBlueOutput(&fizzyMint->blueLight);
	lazCconv.configRedOutput(&fizzyMint->redLight);
	lazCconv.configGreenOutput(NULL);
	lazCconv.saturationInput->writeValue(220);

	genFanOscillator->configSamples(sineWave255, NUM_SAM(sineWave255), 100);
	genFanOscillator->configEventHandler(this, SIG_GEN_REF::GEN_FAN_REF);
	genFanOscillator->configOutput(genFanScaler);
	genFanOscillator->start();

	genFanScaler->configLinearScale(100, 0);
	genFanScaler->configOutput(genStopper);

	genStopper->configOutput(&fizzyMint->greenLight);

	capStatic->configDuty_256(128);
	capStatic->configPeriod_ms(5);
	capStatic->configOutput(genStopper->stopCtrl);

	buzValueScaler->configLinearScale(0, 0x3fffff >> 8, NOTE_A2_US, NOTE_GS6_US);
	buzValueScaler->configOutput(buzStopper);
	buzStatic->configDuty_256(50);
	buzStatic->configPeriod_ms(50);
	buzStatic->configOutput(buzStopper->stopCtrl);
	buzStopper->configOutput(&fizzyMint->buzzer);
	fizzyMint->buzzer.setVolume(1);
}

void SweetMaker::RayGun::View::configFiring()
{
	SigGen * buzGen = &sigGen[3];
	SigScaler * fireScaler = &sigScaler[4];
	SigDebug * debug;

	debug = new SigDebug();
	debug->configOutput(fireScaler);
	debug->configTrace(SigDebug::SerialOut);

	buzGen->configSamples(fireSig, NUM_SAM(fireSig), 150);
	buzGen->configOptions(SigGen::INTERPOLATE_OFF);
	buzGen->configOptions(0);
	buzGen->configOutput(debug);
	buzGen->start(0);
	fizzyMint->buzzer.setVolume(10);


	fireScaler->configLinearScale(0, NOTE_C6_US);
	debug = new SigDebug();
	fireScaler->configOutput(debug);

	debug->configOutput(&fizzyMint->buzzer);
	debug->configTrace(SigDebug::SerialOut);
}

