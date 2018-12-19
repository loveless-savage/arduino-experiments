#include <Arduino.h>

#define REMOTE 1 //whether or not to use the IR sensor code
#define BLUETOOTH 0 //...the bluetooth-controlled drive code
#define LINE 0 //...the line-following system

#define sl 3
#define sr 11
#define ir 4

#define mrv 5
#define mrp 7
#define mrn 8
#define mlv 6
#define mlp 9
#define mln 10

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

IRrecv IR(ir);  //IR sensor object
decode_results IRresults;   

int timeOut=0;//allow for movement to sustain for some time
bool wasPressed=false;//to see if a button was just released

#endif

//general motor movement////////////////////////////////////
int rvel;//store right wheel velocity for transforming
int lvel;//store left wheel velocity for transforming

void writeR(int vel){
	if(vel<0){//if velocity is negative
		digitalWrite(mrp,LOW);//positive pin off
		digitalWrite(mrn,HIGH);//negative pin on
		rvel=-vel;//rvel is positive
	}else{
		digitalWrite(mrn,LOW);//negative pin off
		digitalWrite(mrp,HIGH);//positive pin on
		rvel=vel;//rvel is positive
	}

	//constrain motor speed to 255, the max PWM output
	if(rvel>255){
		rvel=255;
	}

	analogWrite(mrv,rvel);//write the PWM signal
}

void writeL(int vel){
	if(vel<0){//if velocity is negative
		digitalWrite(mlp,LOW);//positive pin off
		digitalWrite(mln,HIGH);//negative pin on
		lvel=-vel;//lvel is positive
	}else{
		digitalWrite(mln,LOW);//negative pin off
		digitalWrite(mlp,HIGH);//positive pin on
		lvel=vel;//lvel is positive
	}

	//constrain motor speed to 255, the max PWM output
	if(lvel>255){
		lvel=255;
	}

	analogWrite(mlv,lvel);//write the PWM signal
}

#if REMOTE
//ir sensor functions///////////////////////////////////////
void ir_stuff()
{
	if(IR.decode(&IRresults)){
		wasPressed=true;//setup for when button is released
		switch(IRresults.value){
		case IR_FORTH:
			writeR(255);
			writeL(250);
			break;
		case IR_RIGHT:
			writeR(-120);
			writeL(120);
			break;
		case IR_LEFT:
			writeR(120);
			writeL(-120);
			break;
		case IR_BACK:
			writeR(-255);
			writeL(-250);
			break;
		default:
			break;
		}
		Serial.println(IRresults.value,HEX);

		IRresults.value = 0;
		IR.resume();
	}else if(wasPressed){
		wasPressed=false;//don't call this whenever no buttons are pressed
		timeOut=millis();//what time is it?
	}else if(millis()-timeOut>=250){/**/
		writeL(0);
		writeR(0);
	}
}
#endif


void setup(){
	Serial.begin(115200);

	pinMode(mrv,OUTPUT);
	pinMode(mrp,OUTPUT);
	pinMode(mrn,OUTPUT);
	pinMode(mlv,OUTPUT);
	pinMode(mlp,OUTPUT);
	pinMode(mln,OUTPUT);

#if REMOTE
	pinMode(ir,INPUT);
	digitalWrite(ir,HIGH);
	IR.enableIRIn();
#endif
}

void loop(){
#if REMOTE
	ir_stuff();
#endif
}
