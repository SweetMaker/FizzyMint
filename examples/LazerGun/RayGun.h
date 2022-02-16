/*******************************************************************************
RayGun.h - Takes a FizzyMint and turns it into a Gun .

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

#ifndef __RAYGUN_H__
#define __RAYGUN_H__


#include "SweetMaker.h"
#include "FizzyMint.h"

extern FizzyMint * fizzyMint;

namespace SweetMaker {
	/*
	* We will be using the SweetMaker Software Development Kit and a fizzyMint so
	* start by including the following files to import class definitions
	*/

	class RayGun {
	public:
		RayGun();
		void setup(FizzyMint * fizzyMint);
		void update();

	private:

		typedef enum {
			LAZ_OSC,
			GEN_FAN_REF
		}SIG_GEN_REF;

		typedef enum {
			Idle,
			SystemTest,
			Standby,
			LeavingStandby,
			Priming,
			ReadyToFire,
			Firing,
			TurningOff
		}RAYGUN_STATE;

		typedef enum {
			TRIGGER_PRESSED = IEventHandler::USER + 0,
			TURNED_ON = IEventHandler::USER + 1,
			TURNED_OFF = IEventHandler:: USER + 2,
			SYSTEST_START = IEventHandler::USER + 3,
			ENTER_STANDBY = IEventHandler::USER + 4,
			LEAVE_STANDBY = IEventHandler::USER + 5,
			ENTER_PRIMING = IEventHandler::USER + 6,
			READY_TO_FIRE = IEventHandler::USER + 7,
			FIRING = IEventHandler::USER + 8,
			CAP_CHARGED = IEventHandler::USER + 9,
			CAP_LOW = IEventHandler::USER + 10,
			CAP_FULL_CHARGED = IEventHandler::USER + 11,
			LAZ_TURNED_ON = IEventHandler::USER + 12,
			FIRING_DONE = IEventHandler::USER + 13,

		}RAYGUN_EVENTS;

		class Control : public IEventHandler {
		public:
			Control();
			void configEventHandler(IEventHandler * model);
			void handleEvent(uint16_t eventType, uint8_t sourceRef, uint16_t eventInfo);
		private:
			boolean rayGunOn;
			IEventHandler * model;
		};

		class Model : public IEventHandler {

		private:
			IEventHandler * view;
			static const uint32_t ambientTemperature_mC = 20000;
			Timer timer;

			class Generator {
			public:
				Generator();
				void processTick_100ms();
				void setRequestPower(uint32_t power_watts);
				uint32_t getRequestedPower();
				uint32_t getCurrentPower();
				uint8_t getFanLevel();
				uint32_t getTemp_mC();
				static const uint32_t ratedPowerOutput_W = 0xffff;

			private:
				static const uint32_t maxRateOfChangeOfPower_W = 10000;
				static const uint32_t heatCapacity_CJ = 1000;
				uint32_t requestedPower_W;
				uint32_t currentPower_W;
				uint32_t temperature_mC;
			};

			class Capacitor {
			public:
				Capacitor();
				void configGenerator(Generator * g);
				void processTick_100ms();
				uint32_t getEnergyLevel();
				uint32_t drawEnergy(uint32_t energy_J);
				void putEnergy(uint32_t energy_J);
				bool isCharged();
				bool isLowInEnergy();
				bool isFullyCharged();
				void turnOn();
				void turnOff();

				void jumpStart();

			private:
				static const uint32_t targetEngyLevel_J = 0x3fffff;
				static const uint32_t taregtLvlHysteresis_J = 0x380000;
				static const uint32_t readyEnergyLevel_J = 0x08ffff;
				uint32_t energyLevel_J;
				Generator * gen;
				bool isTurnedOn;
			};

			class Lazer {
			public:
				Lazer();
				void processTick_100ms();
				void configEventHandler(IEventHandler * handler);
				void configCapacitor(Capacitor * cap);
				void update(uint16_t elapsedTime_ms);
				void turnOn();
				void turnOff();
				bool isTurnedOn();
				bool isPrimed();
				void fire();
				bool isFiring();
				uint32_t getTemp_mC();
				uint32_t getBoltPulse_mW();

				void jumpStart();

			private:
				static const uint32_t primedTemp_mC = 40000;
				static const uint32_t primeDrainRate_W = 20000;
				static const uint32_t idleDrainRate_W = 8000;
				static const uint32_t heatCapacity_CJ = 20;
				bool turnedOn;
				bool primed;
				bool firing;

				uint32_t temperature_mC;
				uint16_t drainRate;

				Capacitor * cap;
				SigGen boltPulse_mW;
			};

		public:
			Model();
			void configEventHandler(IEventHandler * view);
			void handleEvent(uint16_t eventType, uint8_t sourceRef, uint16_t eventInfo);
			void update(uint16_t elapsedTime_ms);
			RAYGUN_STATE state;

			Generator gen;
			Capacitor cap;
			Lazer laz;

			void debugHelper(uint16_t eventType, uint8_t src, uint16_t info);

		};

		class View : public IEventHandler{
		public:
			View();
			void configModel(Model *mod);

			void handleEvent(uint16_t eventType, uint8_t sourceRef, uint16_t eventInfo);
			void processTick_100ms();
			void update(uint16_t elapsedTime_ms);
		private:
			Model * model;

			SigGen sigGen[4];
			SigScaler sigScaler[5];
			StaticGen staticGen[3];
        	SigStopper stoppers[2];
			HsvToRgbSigConv lazCconv;

			void configCatherineWheel();
			void configGreenBlip();
			void configSpeedyBlip();
			void configPriming();
			void configFiring();

			SigGen::SAMPLE fireSig[2] = { 0,  255 };

		};


		/*
		 * Private Members of RayGun
		 */
		Control control;
		Model model;
		View view;

		unsigned long lastUpdateTime_ms;
	

	};
}

#endif