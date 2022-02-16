/*
 * Welcome to the SweetMaker FizzyMint getting started guide. 
 *
 * SweetMaker is a software library for Arduino to help make programming
 * quicker, easier and more fun. It has solved a lot of tricky Arduino 
 * programming problems for you so you don't have to. And 'FizzyMint' is 
 * a simple Arduino circuit which is great for adding Bling to your Junk
 * Model, Tinkering and Toy Making. This sketch uses a FizzyMint and the
 * SweetMaker::FizzyMint code for driving it. If you don't have a FizzyMint
 * then go to www.sweetmaker.tech for details of how to make one :)
 *
 * This guide takes you through how to use the software by example. Work from 
 * top to bottom following the instructions in each Exercise. It should take 
 * you about 30 minutes form Novice to Master of the FizzyMint.
 * 
 * Before you start you should have the following installed:
 * 1) The Arduino IDE from https://www.arduino.cc
 * 2) The SweetMaker Library from https://github.com/SweetMaker/SweetMaker
 * 3) A FizzyMint connected to your USB com port
 *
 * See http://www.sweetmaker.tech for more details
 *
 * Now continue down to Exercise 1
 *
 */

/*
* EXERCISE (1): Compile and Upload to FizzyMint
*
* In this exercise we are simply going to tell the Arduino IDE to compile,
* link and upload the program to FizzyMint. We won't change any of the code; we
* are just checking everything is working properly before we move forward.
*
* Step 1) Check your FizzyMint is connected to a USB port on your computer.
* Step 2) On the Arduino IDE (IDE for short) check menu 'Tools->port' is set to 
*         the correct USB port. Check 'Tools->board' is set to Arduino Nano.
*         Check 'Tools->Processor' is set to ATmega328.
* Step 3) On the Arduino IDE Select the 'tick' menu button to Verify the code 
*         compiles and links. This should be successful and the green banner 
*         towards the bottom of the screen should say 'Done compiling'.
* Step 4) On the Arduino IDE Select the 'right arrow' menu button to Upload
*         the code to your FizzyMint. Once complete the Green Banner should
*         read 'Done Uploading'.
*
* Once successfully uploaded the FizzyMint red light should start flashing 
* at a rate of once per second. Congratulations, you can now upload code to 
* the FizzyMint.
*/

/*
 * EXERCISE (2): FizzyMint Setup 
 *
 * This exercise shows you the code we need to setup FizzyMint for use. This is
 * the same in most FizzyMint sketches. All Arduino Sketches, including 
 * SweetMaker FizzyMint sketches, MUST have two special functions 'setup', 
 * which is run once when the program starts; and 'loop' which is then run again
 * and again forever. In this exercise we are going to take a look at the setup 
 * function and how we configure FizzyMint. There are a number of important 
 * concepts of a 'C++' program which we will encounter as we start, but which 
 * aren't so  important to understand to progress through this tutorial.
 * So don't worry if you don't understand everything, but do ask if you get stuck.
 * (Did I mention that the programming language we are using is called 'C++' 
 * pronounced See Plus Plus).
 *
 * Work down through the file until the end of this exercise following the 
 * explanations and instructions as you go.
 */

/*
 * #include is an instruction to the 'compiler' telling it to put the content
 * of the file named FizzyMint.h here. This file has got code needed for using
 * FizzyMint. We also include a file called 'TuneLib.h", some of our code will
 * need this later.
 */
#include <EEPROM.h>
#include <FizzyMint.h>
#include "TuneLib.h"

/*
 * 'FizzyMint' is a C++ class defined in the file FizzyMint.h
 * The line below creats an instance of FizzyMint which we are calling 
 * fizzyMint. C++ is case sensitive. We can then control our real life
 * FizzyMint using fizzyMint ... feeling dizzy? Sit tight when we use it
 * it will become more clear.
 */
FizzyMint fizzyMint;

/*
 * This is called a 'forward declaration'. Its telling the compiler that there is
 * a function called 'myEventHandler' which we are going to use but which is 
 * further on in the file. The compiler needs to be told this or else it will
 * complain to you that 'myEventHandler' is undefined when we use it a few lines
 * later.
 */
void myEventHandler(uint16_t eventId, uint8_t src, uint16_t eventInfo);

/*
* Well done we've made it to the setup function. Remember this is run once when
* the program starts up. Our setup function starts a 'serial' connection to the 
* host computer and tells fizzyMint what function to call when various 'events'
* it knows about occurs.
*/
void setup()
{
	/* Start Serial at a speed (Baud rate) of 112500 Bytes per second */
	Serial.begin(112500);
	Serial.println("Welcome to the SweetMaker::FizzyMint Tutorial");

	/*
	* In the IDE menu select 'tools->Serial Monitor' Check in the bottom right
	* hand corner of the Serial Monitor window that the baud is set to 112500.
	* ... what do you see?
	*
	* Note: Make sure you keep the Serial Monitor connected when using your
	* FizzyMint, some seem to sporadically reset when connected to the USB
	* port if the Serial Monitor is not connected! I don't know why.
	*/

	/* Configure callback function for handling events */
	fizzyMint.configEventHandlerCallback(myEventHandler);

	/*
	 * We can do other things in setup as well if we want. For instance we could 
	 * turn another light on. Lets tell fizzyMint to turn the blue light on.
	 * fizzyMint has three lights, a red a green and a blue one. we can turn them
	 * on and off and even set the brightness. Try turning the Blue light on by 
	 * uncommenting the following line (delete the '//') and pressing the 'Upload' 
	 * button (the button with the arrow pointing right).
	 */
	 fizzyMint.light[BLUE_LIGHT].turnOn();
}

/*
 * This is the loop function. All it does is call the fizzyMint update function.
 * This function makes fizzyMint do all kinds of things behind the scenes. In 
 * this tutorial we aren't going to do anything else in the loop() function.
 * Instead we are going to put our code in 'myEventHandler'
 */
void loop()
{
	fizzyMint.update();
}


/*
 * EXERCISE (3): SweetMaker Event Handling
 *
 * SweetMaker::FizzyMint will detect lots of different events and tell us when 
 * they happen through our event handler function. When fizzyMint detects an 
 * event such as the button being pressed, it will call this function to tell us. 
 * The function has three 'parameters':
 *   eventId - the event which has occured, supported events are shown below
 *   ref - used to indicate where the event was generated
 *   eventInfo - optional event specific information
 *
 * In this exercise we are going to use the SWITCH_TURNED_ON event to trigger 
 * some action. Find the SWITCH_TURNED_ON event at the top of the function.
 */

void myEventHandler(uint16_t eventId, uint8_t eventRef, uint16_t eventInfo)
{
	switch (eventId)
	{
		/*
		* Step 1: FizzyMint can generate musical notes using its buzzer
		* uncomment the line below to make FizzyMint play a note when the button
		* is pressed.
		*/
		case SwitchDriver::SWITCH_TURNED_ON: // The button has been pressed
		  fizzyMint.buzzer.playPeriod_us(NOTE_C5_US, 1000); // Play Note C5 for 1000ms (one second);

		/*
		 * Step 2: Now add your own code to turn the green light on as well 
		 * as play a note. 
		 */
      fizzyMint.light[GREEN_LIGHT].turnOn();
		break;

		case SwitchDriver::SWITCH_TURNED_OFF: // The switch has been released
		/*
		* Step 3: Now add code to turn the green light off
		*/
      fizzyMint.light[GREEN_LIGHT].turnOff();
      break;

		/*
		 * EXERCISE (4): Controlling Light brightness and different notes
		 *
		 * In this exercise we will introduce two more SwitchDriver events and
		 * when they occur use them to change the brightness of the Lights and 
		 * the note being played.
		 */
		case SwitchDriver::SWITCH_HELD_ON:   // The button has been held down for a second
		/*
		 * Step 1: When the switch is held for a second turn all the lights off
		 */
		fizzyMint.light[RED_LIGHT].turnOff();
		// fizzyMint.light[GREEN_LIGHT].turnOff();
		// <add code for blue light here>

		break;

		case SwitchDriver::SWITCH_STILL_HELD_ON: // Held down for another second - eventInfo counts number of seconds
		{
		/*
		 * Step 2: If the Switch is held on even longer, gradually increase the
		 * brightness and pitch of the note. This event is generated once every second
		 * that the switch continues to be held down; eventInfo is increased by 1 each 
		 * time
		 */
			if (eventInfo < 16)
			{
				uint8_t brightness = eventInfo * 16;
				uint32_t note = map(eventInfo, 0, 16, NOTE_A2_US, NOTE_G7_US); // useful Arduino helper function
				/*
				 * To control the brightness when we call 'turnOn()' we tell it how bright
				 * we want it to be from a min of 0 to a max of 255. turnOn(128) would be mid
				 * brightness.Try multiplying eventInfo by 20 to increase the brightness each
				 * time.
				 */
				// <Insert your Step2: Code Here>
			}
		}
		break;


		/*
		 * EXERCISE (5): Signal Generators
		 *
		 * One of the most powerful features of the SweetMaker Library is the
		 * Signal Generator. This allows you to make different 'waveforms' like
		 * heartbeats, or sine waves or triangle waves. You can control how many
		 * milliseconds it takes to play the wave and how many times it plays.
		 * The output from a signal generator can be used as an input to the 
		 * lights or the buzzer.
		 *
		 * In this example we create signal generators to make the lights go 
		 * blip and to make the buzzer note slide up and down. We also use a new
		 * event ... the KNOCK_DETECTED event. The buzzer, when it isn't buzzing
		 * detects knocks! 
		 */
		case PiezoDriver::KNOCK_DETECTED: // A knock has been detected - eventInfo says how hard.
		{
		/*
		* The following line defines a wave we are calling 'blip', it quickly 
		* rises in value from 64 to 255 then gradually falls to zero making a 
		* kind of blip wave.
		*/
			static const SigGen::SAMPLE blip[] PROGMEM = { 64,255,230,205,180,155,130,105,80,55,30,0 };

			/*
			* The following makes three signal generators, one for each of the leds.
			* When we make a SigGen we tell it the samples we want to use, the number
			* of samples, the period of time we want the signal to be played over, and
			* there are a few options we might use. 
			*/
			static SigGen redSig(blip, NUM_SAM(blip), 700, 0);
			//static SigGen greenSig(blip, NUM_SAM(blip), 700, 0);
			// < add another SigGen called blueSig >

			/*
			* We now need to tell the SigGens where to send their signals! The following
			* line configures the output of the redSig to go to the red light. We can do
			* this because the FizzyMint lights have been designed as a SweetMaker::SigInput
			*/
			redSig.configOutput(&fizzyMint.light[RED_LIGHT]);
			/*
			 * Step 1: Configure the green and blue SigGen outputs to connect to the green
			 * and blue lights.
			 */
			//<put your code here. Hint: copy, paste, modify the above line>

			/*
			 * Step 2: Start the signal generators and make them run once. Or twice if
			 * you fancy it!
			 */
			redSig.start(1);
			// <do the same for blue and green>

			/*
			 * Step 3: Go back and change the time period for each of the 
			 * signal generators to something different. Maybe change the Blip wave
			 * sample values as well.
			 */

			/*
 			 * Step 4: Create a new Signal Generator to go from NOTE_C3_US to NOTE_C6_US
			 * configure the output to go to the buzzer, and then start it. I've started
			 * you off, modify and complete the lines
			 */
			// static const SigGen::SAMPLE risingNoteSamples[] PROGMEM = { <add notes here> };
			// static SigGen risingNoteSig(risingNoteSamples, NUM_SAM(risingNoteSamples), 2000, 0);
			// configure output to be <&fizzyMint.buzzer>
			// start the SigGen

			/*
			 * Did that work well? Is there an Issue?
			 * Uncomment and take note of the next line then scroll down till you get to
			 * the next exercise.
			 */
			// risingNoteSig.configEventRef(1);
		}
		break; 


		/*
		 * EXERCISE (6) : EventRef / Stop that buzzer
		 *
		 * I expect that EXERCISE (5) left the buzzer ringing. We need to 
		 * stop that and to do that we look out for the SIG_GEN_FINISHED
		 * event. But ... we've got four SigGens running, three for the Lights and
		 * one for the buzzer. All of them stopping at different times. How do we 
		 * know which one is the risingNoteSig? Use the EventRef at the end of 
		 * EXERCISE (5) we set the eventRef to a value of 1. Lets use that 
		 * knowledge now.
		 */

		/*
		 * Step 1: stop that buzzer. Uncomment the instruction to stop the buzzer
		 */
		case SigGen::SIG_GEN_FINISHED: // A Signal Generator has finished 
		{
			if (eventRef == 1) {
			//	fizzyMint.buzzer.stop();

			/*
			 * EXERCISE (7) : Timers.
			 *
			 * SweetMaker has support for timers. You can start a timer and then get
			 * an event when the timer expires. In this exercise we are going to start
			 * a timer when processing the risingNoteSig SIG_GEN_FINISHED event.
			 */
			/* Step 1: Start a timer and set the time to 5000 ms */
				static Timer myTimer;
				//myTimer.startTimer(<set time in ms>,<eventRef>);
			}
		}
		break;

		case SigGen::SIG_GEN_STARTED: // A Signal Generator has been started
		case SigGen::SIG_GEN_STOPPED: // A Signal Generator has been stopped
		break;

		/*
		 * Step 2: play a tune when timer expires.
		 *
		 * We now capture the TIMER_EXPIRED event and use this to start the
		 * TunePlayer. When the timer expires lets play the Star Wars Main Theme,
		 * one of the tunes included in the "TuneLib.h" file. Look in this file to 
		 * see how to write your own tune.
		 */
		case TimerTickMngt::TIMER_EXPIRED: // A timer has expired - eventInfo from timerId
		/*
		 * playTune(<notes>, <number of notes>, <tempo>) tempo is in notes per minute.
		 */
		// fizzyMint.tunePlayer.playTune(starWarsNotes, NUM_NOTE(starWarsNotes), STARWARS_TEMPO);
		break;

		/*
		 * There are a few other Timer Events which you can use in your programs
		 */
		case TimerTickMngt::TIMER_TICK_UPDATE: // Generated every time fizzyMint is updated - could be every 500us (micro seconds) e.g. less than a millisecond
		case TimerTickMngt::TIMER_TICK_100MS: // Generated ten times a second
		case TimerTickMngt::TIMER_TICK_10S: // Generated once every ten seconds
		case TimerTickMngt::TIMER_FREQ_GEN: // Generated a certain number of times a seconds
		break;

		/*
		 * This code has been here from the start of the tutorial. It is what has been
		 * making the red light flash
		 */
		case TimerTickMngt::TIMER_TICK_S: // Generated every second
		{
			static uint8_t ledOn = 0;
			if (ledOn == 0) {
				fizzyMint.light[RED_LIGHT].turnOn();
				ledOn = 1;
			}
			else {
				fizzyMint.light[RED_LIGHT].turnOff();
				ledOn = 0;
			}
		}
		break;

		/*
		* EXERCISE (8) : Tune Events 
		*/
		case TunePlayer::TUNE_NEXT_NOTE: // Tune Player started next note
		{
		/*
		 * This event is generated by the TunePlayer for each new note in the tune.
		 * The eventInfo increases with each new Note. When this event occurs we are  
		 * going to change the lights.
		 */
			switch (eventInfo % 3) // this finds the remainder when we divide by 3
			{
				case 0:
				/*
				 * Set the lights on the 1st, 4th, 7th ... notes
				 */
				fizzyMint.light[RED_LIGHT].turnOn(); // Set Red Light On
				fizzyMint.light[GREEN_LIGHT].turnOn(128); // Green Light Half Brightness
				fizzyMint.light[BLUE_LIGHT].turnOff(); // Blue Light Off
				break;

				case 1:
				/*
				 * Step 1: uncomment these lines to set the lights on the 2nd, 5th, 8th ... note
				 */
				//fizzyMint.light[GREEN_LIGHT].turnOn();
				//fizzyMint.light[BLUE_LIGHT].turnOn(128);
				//fizzyMint.light[RED_LIGHT].turnOff();
				break;

				case 2:
				/*
				 * Step 2: add code to change the lights on the 3rd, 6th, 9th ... notes
				 */
				break;
			}
		}
		break;

		case TunePlayer::TUNE_STOPPED: // The Tune has been stopped before it has ended
		case TunePlayer::TUNE_ENDED: // The Tune has ended
		/*
		 * Step 3: Turn all the lights off at the End of the tune
		 */
		// < insert code here>
		break;
	}
}


/*
* *** CONGRATULATIONS ***
*
* You have reached the end of the tutorial, you have learnt how to control
* FizzyMint lights, respond to events, play tunes, make notes, and create
* your own Signal Generators. Awesome!!
*
* This is just the begginging, FizzyMint, SweetMaker and Arduino allow you
* to do more, the only limit is your imagination. Think of a project and make 
* something. Use the other example FizzyMint Sketches for inspiration; load them 
* onto your FizzyMint and see what they do. Look at the code and see how they 
* work for yourself.
*
*/


/*******************************************************************************
Tutorial.ino - Introduction to SweetMaker and FizzyMint

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
