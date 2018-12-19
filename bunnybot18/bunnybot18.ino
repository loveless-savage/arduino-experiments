/* arduino code for the 2018 bunnybot color sorters
 * each of three arduinos has an adafruit color sensor and a servo attached.
 * the arduinos are programmed to operate as follows:
	> detect the color of an incoming ball
	> if the ball is of the desired color, the servo retracts a paddle and allows it to drop into a hopper
	> if the ball is the wrong color, the servo deflects it off the bot
	> the servo does not move until it needs to react to an opposite colored ball
	  -this means it will only flip-flop once to channel 3 red balls in a row.
 * the arduinos work autonomously and need no driver input. They are not connected to the RoboRIO,
   and draw power independently from the battery
 * all three arduinos are connected to a toggle switch which determines which color- red or blue-
   is rejected that round, with no need to restart the arduinos
*/


// red bumped = 0, blue bumped = 1
#define WHICH_COLOR 0

// which arduino? each one has slightly different tuning
#define ARDUINO_NUM 0

#if ARDUINO_NUM == 0 // ****************************** arduino 0, furthest from hopper

// minimum color intensity
#define RED_CUTOFF 1000
#define BLUE_CUTOFF 1000
// where will the servo sweep to? deg
#define BUMP_POS 125
// where will the servo sit when idle? deg
#define IDLE_POS 180
// how long will the flippy-floppy thing stay engaged? ms
#define DELAY 2000
// how long exposure time? index value, see abt. 20 lines down
#define EXP_IDX 2

#elif ARDUINO_NUM == 1 // ****************************** arduino 1, in the center

#define RED_CUTOFF 1500
#define BLUE_CUTOFF 1500

#define BUMP_POS 125
#define IDLE_POS 180

#define DELAY 2000

#define EXP_IDX 2

#else // ****************************** arduino 2, nearest to hopper

#define RED_CUTOFF 1500
#define BLUE_CUTOFF 1500

#define BUMP_POS 125
#define IDLE_POS 180

#define DELAY 2000

#define EXP_IDX 2

#endif // ******************************


#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h> 


// Color Sensor
/*
	Connect SCL		to analog 5
	Connect SDA		to analog 4
	Connect VDD		to 3.3V DC
	Connect GND		to common ground
*/
// easier to switch between integration times using an array than typing "TCS34725_INTEGRATIONTIME_101MS" every time
const tcs34725IntegrationTime_t TCS_Itgn_t[] {
	TCS34725_INTEGRATIONTIME_2_4MS, //    0: 2.4ms
	TCS34725_INTEGRATIONTIME_24MS,  //    1:  24ms
	TCS34725_INTEGRATIONTIME_50MS,  //    2:  50ms
	TCS34725_INTEGRATIONTIME_101MS, //    3: 101ms
	TCS34725_INTEGRATIONTIME_154MS, //    4: 154ms
	TCS34725_INTEGRATIONTIME_700MS  //    5: 700ms
};
// declare color sensor with specific int time and gain values
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS_Itgn_t[EXP_IDX], TCS34725_GAIN_1X);


// Servo declaration
#define servo_pin 15 // pin 15 = A2
Servo myservo;

// is the servo paddle pointing in or out?
bool isBumping = false;


// toggle switch pin
#define toggle_b 16 // pin 16 = A3
// seekingRed stores state of toggle switch
bool seekingRed;


// color values; swapped if toggle is activated
uint16_t bumpVal = 0;
uint16_t idleVal = 0;

// booleans interpreting color sensor's data:
// is there a ball? and if so, is it red or not?
bool isBall, isRed;


// flip-flop thing- deprecated
/*
void bump(){
	// the servo goes from 0 degrees to 180 degrees
	// in steps of 1 degree

	// which direction should the servo go? reverse motion if:
	//  ->isRed==false
	//  ->toggle_dir==false
	// though not if both are false, hence the xor (^)
	myservo.write(BUMP_POS);

	// put the servo back once the ball gets bumped
	delay(DELAY/2);  
	myservo.write(IDLE_POS);
	delay(DELAY/2);
}
*/


void setup(void) {
	Serial.begin(9600);

	// initialize color sensor
	if (tcs.begin()) {
		Serial.println("Found sensor");
	} else {
		Serial.println("No TCS34725 found ... check your connections");
		while (1);
	}

	// initialize servo
	myservo.attach(servo_pin);
	myservo.write(IDLE_POS);

	// toggle pin
	pinMode(toggle_b, INPUT);
}


void loop(void) {
	// collection variables for the color sensor data
	uint16_t r, g, b, c;

	// nab data from color sensor- we only need r and b
	tcs.getRawData(&r, &g, &b, &c);//&r is a reference to r's location in memory
	// other color sensor data options- deprecated
		//colorTemp = tcs.calculateColorTemperature(r, g, b);
		//lux = tcs.calculateLux(r, g, b);

	// reduce color sensor data to booleans
	isBall = true;
	// is the ball red?
	if (r>RED_CUTOFF && r>b ) {
		isRed = true;
	// ...blue?
	} else if (b>BLUE_CUTOFF) {
		Serial.print("\tblue\t");
		isRed = false;
	// ...not there?
	} else {
		Serial.print("\t\t");
		isBall = false;
	}

	// listen for toggle switch
	seekingRed = digitalRead(toggle_b);
	// collect data into appropriate locations- deprecated
/*
	if(digitalRead(toggle_b)){
		bumpVal = b;
		idleVal = r;
	}else{
		bumpVal = r;
		idleVal = b;
	}
*/

	// diagnostic printout- explained by following comment
	if(isBall){
		Serial.print(isRed?"ball YES- 1\t":"ball YES- 0\t");
		Serial.print(isBumping?"Bumping = 1\t":"Bumping = 0\t");

		Serial.print((isRed ^ isBumping)?"^ 1\t":"^ 0\t");
		Serial.print(WHICH_COLOR?"WHICH = 1\t":"WHICH = 0\t");

		Serial.print((isRed ^ isBumping)==/*seekingRed*/WHICH_COLOR?"FLOPPING\t":"static\t");

	}else{
		Serial.print("ball NO    \t\t\t\t");
	}
	Serial.print(isBumping?"Bumping = 1\t":"Bumping = 0\t");

	/* here be the action, where the servo decides whether or not to flipflop
	 * isBall --> only act if a ball is detected at all
	 * (isRed ^ isBumping) --> if ball color is different from the servo's current state
	 * ==seekingRed --> if switch is toggled, opposite ball color matches servo's state
	 * ******currently using WHICH_COLOR instead, while switch is being attached
	*/

	if(isBall && (isRed ^ isBumping)==/*seekingRed*/WHICH_COLOR ){
		// write either idle position or bumping position to the servo, according to isBumping
		myservo.write(isBumping? BUMP_POS:IDLE_POS );
		// servo is toggled, so toggle stored state too
		isBumping != isBumping;
	}

	Serial.println("");

/*
	// bump the ball if we want to keep it- deprecated
	if( bumpVal > COLOR_CUTOFF ){
		bump();
	}
*/
}
