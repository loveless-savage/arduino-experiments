/*onboard
 *sensors
 *com
 *i2c
 *motors
*/
#define cal 3
#define res 4
#define lck 7
#define rck 5
#define lda 12
#define rda 8

#define mfl 6
#define mfln 9
#define mu 10
#define mun 11

#include <Wire.h>
#include <LIS3MDL.h>

LIS3MDL compass;

bool state=0;
uint16_t sensordata, motordata, caliper;

int8_t forth, side, turn;
int8_t fl, vr;

void* calib;//stores label

void split_write(int p, int pn, int8_t value){
        if(value<0){
                analogWrite(p, 0);
                analogWrite(pn, -value);
        }else{
                analogWrite(pn, 0);
                analogWrite(p, value);
        }
};

void toggle(){
	state = digitalRead(rck);//expects what the remote clock was not
	digitalWrite(lck, !digitalRead(lck) );//flips the clock pin
	Serial.println("toggle");
	delay(500);
}

void waitcom(){
	Serial.println("waiting");
	Serial.print("remote clock:\t");
	Serial.println(state);
	Serial.print("local clock:\t");
	Serial.println(digitalRead(rck));
	while(digitalRead(rck) == state){//waits until the remote clock changes
	}
	Serial.println("got a call!");
	delay(500);
}

void readcom(void* calib, char i){
	if(digitalRead(cal)!=bitRead(caliper, i)){//if, by chance, the boards get out of sync
		digitalWrite(res, HIGH);
		toggle();
		Serial.println("AAAAAAHHHHHH!!!! out of sync!");
		goto *calib;
	}

	Serial.println("readcom");
	if(digitalRead(rda)){
		bitSet(motordata, i);//if topside sent a 1
	}else{
		bitClear(motordata, i);//if topside sent a 0
	}
	Serial.println("");
	delay(500);
}

void sendcom(char i){
	digitalWrite(lda, bitRead(sensordata, i));
	Serial.println("sendcom");
	delay(500);
}

void setup(){
	pinMode(cal, INPUT);
	pinMode(res, OUTPUT);
	pinMode(lck, OUTPUT);
	pinMode(rck, INPUT);
	pinMode(lda, OUTPUT);
	pinMode(rda, INPUT);
	
	digitalWrite(lck, LOW);

	pinMode(mfl, OUTPUT);
	pinMode(mfln, OUTPUT);
	pinMode(mu, OUTPUT);
	pinMode(mun, OUTPUT);
	
/*
	compass.init();
	compass.enableDefault();
	delay(1);
*/
	sensordata=0b0101001100001111;
	caliper=0b1011001111000010;
	Serial.begin(115200);
	
}
//this is onboard
void loop(){
	//label- this is a flag for the goto statements, if the boards get out of sync
beginloop:
	calib=&&beginloop;
	Serial.println("labellabellabel");
//sensors///////////////////////////////////////////////////
	/*sensordata has two bytes in it, and the data itself has two halves as well
	 *sensordata = XXXXXXXX XXXXXXXX ;
	 *compass value-^^^         ^^^-depth value
	*\/

	compass.read();
	if((unsigned int)compass.m.y<=(unsigned int)compass.m.x){
		sensordata=(uint16_t)(180.0+atan2(compass.m.y,compass.m.x)*180/M_PI);
	}else{
		sensordata=(uint16_t)(270.0-atan2(compass.m.x,compass.m.y)*180/M_PI);
	}
	//make space for depth
	sensordata=sensordata << 7;
	
	//depth- add onto sensordata without erasing the compass value
	//bitmasks: 0xff00 and 0x00ff help us ensure the bits can only be written in certain areas
	//they allow bits to be written to the areas shown below:
	//            XXXXXXXX --------        -------- XXXXXXXX
	sensordata=( 0xff00 & sensordata )|( 0x00ff & analogRead(3) );
*/
//com///////////////////////////////////////////////////////
	for(char i= 0x0f ; i>=0; i--){
	Serial.print("bit# ");
	Serial.println(i,HEX);
		waitcom();
		readcom(calib, i);
		sendcom(i);
		toggle();
	}

	//debug print
	Serial.print("\tmotordata= ");
        Serial.println(motordata,BIN);
//i2c///////////////////////////////////////////////////////
/*
	}
	//timer=micros()-timer;

	//Serial.println(timer);
	
	//debug print
	Serial.print("motordata= ");
        Serial.println(motordata,BIN);
#endif
//i2c///////////////////////////////////////////////////////
#if SENSORS & MOTORS
	Wire.beginTransmission(8);
	Wire.write(motordata);
	Wire.endTransmission(8);
#endif
//motors////////////////////////////////////////////////////
