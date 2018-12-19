// red bumped = 0, blue bumped = 1
#define WHICH_COLOR 0

// which arduino? each one has slightly different tuning
#define ARDUINO_NUM 0

#if ARDUINO_NUM == 0 // ****************************** arduino 0, furthest from hopper

// minimum color intensity
#define RED_CUTOFF 1500
#define BLUE_CUTOFF 1500
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
// Initialise with specific int time and gain values
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS_Itgn_t[EXP_IDX], TCS34725_GAIN_1X);


// Servo Setup
#define servo_pin 15 // pin 15 = A2
Servo myservo;

// is the servo in or out?
bool isBumping = false;


// toggle switch
#define toggle_b 16 // pin 16 = A3
bool seekingRed;


// color values; primary collection
uint16_t r,b;
// color values; swapped if toggle is activated
uint16_t bumpVal = 0;
uint16_t idleVal = 0;
bool isBall, isRed;

// flip-flop thing- old system
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

	// color sensor
	if (tcs.begin()) {
		Serial.println("Found sensor");
	} else {
		Serial.println("No TCS34725 found ... check your connections");
		while (1);
	}

	// servo
	myservo.attach(servo_pin);
	myservo.write(IDLE_POS);

	// toggle
	pinMode(toggle_b, INPUT);
}


// Now we're ready to get readings!
void loop(void) {
	//uint16_t r, g, b, c, colorTemp, lux;

	// nab data from color sensor- we only need r and b
	tcs.getRawData(&r, NULL, &b, NULL);//&r is a reference to r's location in memory
		//colorTemp = tcs.calculateColorTemperature(r, g, b);
		//lux = tcs.calculateLux(r, g, b);

	// reduce color sensor data to booleans
	isBall = true;
	// red
	if (r>RED_CUTOFF && r>b ) {
		Serial.println("\t\tred");
		isRed = true;
	// blue
	} else if (b>BLUE_CUTOFF) {
		Serial.println("\t\tblue");
		isRed = false;
	// neither
	} else {
		Serial.println("no ball");
		isBall = false;
	}

	// toggle
	seekingRed = digitalRead(toggle_b);
/*
	if(digitalRead(toggle_b)){
		bumpVal = b;
		idleVal = r;
	}else{
		bumpVal = r;
		idleVal = b;
	}
*/


	// here be the action
	// isBall --> only act if a ball is detected at all
	// (isRed ^ isBumping) --> if ball color is different from the servo's current state
	// ==seekingRed --> if switch is toggled, opposite ball color matches servo's state
	// ******currently using WHICH_COLOR instead, while switch is being attached
	if(isBall && (isRed ^ isBumping)==/*seekingRed*/WHICH_COLOR ){
		// write either idle position or bumping position to the servo, according to isBumping
		myservo.write(isBumping? BUMP_POS:IDLE_POS );
		// servo is toggled
		isBumping != isBumping;
	}


/*
	// bump the ball if we want to keep it
	if( bumpVal > COLOR_CUTOFF ){
		bump();
	}
*/
}
