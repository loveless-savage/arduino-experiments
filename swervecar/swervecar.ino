#include <Arduino.h>
#include "swervecar.h"

#define REMOTE 1 //whether or not to use the IR sensor code
#define BLUETOOTH 0 //...the bluetooth-controlled drive code
#define LINE 0 //...the line-following system

#define p_enc 3 // sensor watching wheels
#define p_bump 11 // sensor for front bumper
#define p_ir 4 // ir remote input

#define mwv 5
#define mwp 7
#define mwn 8
#define msv 6
#define msp 9
#define msn 10

const int turnRate=100;//how fast to turn the pivot motor?

volatile int milsTurning=0;//how long does the pivot take to turn?
volatile int catchCount=0;//capture turn duration first catch only

volatile ternary pivotPos = IDLE;// where are the wheels right now?
volatile bool isCaught = false;//has catchPivot happened yet?


void catchPivot(){
	analogWrite(msv, 0);//freeze turning motor

	if(catchCount==0){
		milsTurning = millis();//start counting milliseconds until next turn
	}else if(catchCount==1){//only do this after turning again
		milsTurning = millis() - milsTurning;//calculate time since turn began
		milsTurning /= 2;
		return;
	}
	catchCount++;

	switch(pivotPos){
	case IDLE://something is very wrong! stop and let the user fix the wheel
		while(1);
	case LEFT://turn right instead
		digitalWrite(msp,LOW);
		digitalWrite(msn,HIGH);
		analogWrite(msv,turnRate);
		break;
	case RIGHT:
		digitalWrite(msn,LOW);
		digitalWrite(msp,HIGH);
		analogWrite(msv,turnRate);
		break;
	}

}


#if REMOTE

#include <IRremote.h>

#define IR_FORTH	0x00ff18e7       //code from IR controller "▲" button
#define IR_BACK		0x00FF4AB5       //code from IR controller "▼" button
#define IR_RIGHT	0x00FF5AA5       //code from IR controller ">" button
#define IR_LEFT		0x00FF10EF       //code from IR controller "<" button
#define IR_OK		0x00FF38C7       //code from IR controller "OK" button
#define IR_0		0x00FF9867       //code from IR controller "0" button
#define IR_1		0x00FFA25D       //code from IR controller "1" button
#define IR_2		0x00FF629D       //code from IR controller "2" button
#define IR_3		0x00FFE21D       //code from IR controller "3" button
#define IR_4		0x00FF22DD       //code from IR controller "4" button
#define IR_5		0x00FF02FD       //code from IR controller "5" button
#define IR_6		0x00FFC23D       //code from IR controller "6" button
#define IR_7		0x00FFE01F       //code from IR controller "7" button
#define IR_8		0x00FFA857       //code from IR controller "8" button
#define IR_9		0x00FF906F       //code from IR controller "9" button
#define IR_STAR		0x00FF6897       //code from IR controller "*" button
#define IR_POUND	0x00FFB04F       //code from IR controller "#" button

IRrecv IR(p_ir);  //IR sensor object
decode_results IRresults;   

int timeOut=0;//allow for movement to sustain for some time
bool wasPressed=false;//to see if a button was just released

ternary userInstructions = IDLE;//what does the remote say?
ternary lastInstructions = IDLE;//what happened last time?

#endif

//general motor movement////////////////////////////////////
int rvel;//store right wheel velocity for transforming
int lvel;//store left wheel velocity for transforming

#if REMOTE
//ir sensor functions///////////////////////////////////////
void ir_stuff(){
	lastInstructions = userInstructions;
	if(IR.decode(&IRresults)){//if something on the remote is being pressed
		wasPressed=true;//setup for when button is released
		switch(IRresults.value){
		case IR_FORTH:
			userInstructions=IDLE;
			writeWheels(255);
			break;
		case IR_RIGHT:
			userInstructions=RIGHT;
			break;
		case IR_LEFT:
			userInstructions=LEFT;
			break;
		case IR_BACK:
			userInstructions=IDLE;
			writeWheels(-255);
			break;
		default:
			break;
		}
		Serial.println(IRresults.value,HEX);

		IRresults.value = 0;
		IR.resume();
	//if no buttons are pressed on the remote
	}else if(wasPressed){
		wasPressed=false;//don't call this whenever no buttons are pressed
		timeOut=millis();//what time is it?
	}else if(millis()-timeOut>=250){/**/
		userInstructions=IDLE;
		writeWheels(0);
	}
}
#endif

void writeWheels(int vel){
	if(vel<0){//if velocity is negative
		digitalWrite(mwp,LOW);//positive pin off
		digitalWrite(mwn,HIGH);//negative pin on
		rvel=-vel;//rvel is positive
	}else{
		digitalWrite(mwn,LOW);//negative pin off
		digitalWrite(mwp,HIGH);//positive pin on
		rvel=vel;//rvel is positive
	}

	//constrain motor speed to 255, the max PWM output
	if(rvel>255){
		rvel=255;
	}

	analogWrite(mwv,rvel);//write the PWM signal
}

void writePivot(ternary dir, ternary lastdir){

	switch(dir){//if turning right, register negative speed
	case RIGHT:
		digitalWrite(msp,LOW);//positive pin off
		digitalWrite(msn,HIGH);//negative pin on
	case LEFT:
		digitalWrite(msn,LOW);//negative pin off
		digitalWrite(msp,HIGH);//positive pin on
	case IDLE:
		switch(lastdir){
		case LEFT:
			digitalWrite(msp,LOW);//positive pin off
			digitalWrite(msn,HIGH);//negative pin on
			break;
		case RIGHT:
			digitalWrite(msn,LOW);//negative pin off
			digitalWrite(msp,HIGH);//positive pin on
			break;
		case IDLE:
			break;
		}
	}

	analogWrite(msv,turnRate);
}


void setup(){
	Serial.begin(9600);

	pinMode(mwv,OUTPUT);
	pinMode(mwp,OUTPUT);
	pinMode(mwn,OUTPUT);
	pinMode(msv,OUTPUT);
	pinMode(msp,OUTPUT);
	pinMode(msn,OUTPUT);

	// manage turning safeguards
	attachInterrupt(p_enc-2, catchPivot, FALLING);

	writePivot(LEFT,IDLE);//start turning to one side
	//at this point, the interrupt signal will take care of the correction system

#if REMOTE
	pinMode(p_ir,INPUT);
	digitalWrite(p_ir,HIGH);
	IR.enableIRIn();
#endif

	delay(2000);
}

void loop(){
#if REMOTE
	ir_stuff();
#endif
	
	//manage turning reaction
	if(userInstructions!=lastInstructions){
		switch(lastInstructions){
		case IDLE://starting from center-aligned
			break;
		}
	}
}

