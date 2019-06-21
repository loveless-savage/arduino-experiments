/*onboard
 *sensors
 *com
 *i2c
 *motors
*/

#define COM 1
#define SENSORS 0
#define COMPASS 0
#define MOTORS 0

#define cal 3
#define res 4
#define lck 7
#define rck 5
#define lda 12
#define rda 8

#define mfl 10
#define mfln 11
#define mu 6
#define mun 9

#include <Wire.h>
#include <LIS3MDL.h>

LIS3MDL compass;

bool clstate=0;//clock state
bool restate=1;//reset pin state
uint16_t sensordata, motordata, caliper;

uint8_t depth;//this is for transforming the depth sensor

int8_t forth, side, turn;
int8_t fl, vr;

void* calib;//stores label

unsigned long timer;

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
	delay(10);
	clstate = digitalRead(rck);//expects what the remote clock was not
	digitalWrite(lck, !digitalRead(lck) );//flips the clock pin
}

void waitcom(){
	while(digitalRead(rck) == clstate){//waits until the remote clock toggles
	}
}

void readcom(void* calib, char i){
	if(digitalRead(cal)!=bitRead(caliper, i)){//if, by chance, the boards get out of sync
		digitalWrite(res, restate);
		toggle();
		Serial.println("AAAAAAHHHHHH!!!! out of sync!");
		restate=!restate;
		goto *calib;
	}

	if(digitalRead(rda)){
		bitSet(motordata, i);//if topside sent a 1
	}else{
		bitClear(motordata, i);//if topside sent a 0
	}
}

void sendcom(char i){
	digitalWrite(lda, bitRead(sensordata, i));
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
	
#if SENSORS
	Wire.begin();
	compass.init();
	compass.enableDefault();
	delay(1);
#endif
	sensordata=0b0101001100001111;
	caliper=0b1011001111000010;
	Serial.begin(115200);
	
}
//this is onboard
void loop(){
	//label- this is a flag for the goto statements, if the boards get out of sync
beginloop:
	calib=&&beginloop;
//sensors///////////////////////////////////////////////////
	sensordata=0;
#if COMPASS
	/*sensordata has two bytes in it, and the data itself has two halves as well
	 *sensordata = XXXXXXXX XXXXXXXX ;
	 *compass value-^^^         ^^^-depth value
	*/
	compass.read();
	if((unsigned int)compass.m.y<=(unsigned int)compass.m.x){
		sensordata=(uint16_t)(180.0+atan2(compass.m.y,compass.m.x)*180/M_PI);
	}else{
		sensordata=(uint16_t)(270.0-atan2(compass.m.x,compass.m.y)*180/M_PI);
	}
	//make space for depth
	sensordata=sensordata >> 1;
#endif
#if SENSORS
	//depth value
	depth=(uint8_t)analogRead(3);
	depth-=99;//air pressure is 100, so we take that off- although leave some on for rounding up
	depth>>=2;//divide by 4
	
	//add depth value onto sensordata without erasing the compass value
	//bitmasks: 0xff00 and 0x00ff help us ensure the bits can only be written in certain areas
	//they allow bits to be written to the areas shown below:
	//            XXXXXXXX --------        -------- XXXXXXXX
	sensordata=( 0xff00 & depth )|( 0x00ff & sensordata );
#endif
//com///////////////////////////////////////////////////////
#if COM
	Serial.print("com");
	//timer=micros();
	for(char i= 0x0f ; i>=0; i--){
		waitcom();
		readcom(calib, i);
		sendcom(i);
		toggle();
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
#if MOTORS
	//first, to extract the info from motordata
	forth = (motordata >>8) & 0xf0;
	side  = (motordata >>8) & 0x0f;
	side  = side << 4;
	vr    = motordata & 0xf0;
	turn  = motordata & 0x0f;
	turn  = turn  << 2;

	//front left motor
	fl=forth+side+turn;
	split_write(mfl, mfln, fl);

	//vertical motors
	split_write(mu, mun, vr);
#endif
}

