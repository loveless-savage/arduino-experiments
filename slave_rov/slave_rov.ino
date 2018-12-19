/*slave (onboard)
 *i2c
 *motors
*/
#define mfr 3
#define mfrn 5
#define mbr 6
#define mbrn 9
#define mbl 10
#define mbln 11

#include <Wire.h>

int8_t forth, side, turn;
int8_t fr, br, bl;

void split_write(int p, int pn, int8_t value){
	if(value<0){
		analogWrite(p, 0);
		analogWrite(pn, -value);
	}else{
		analogWrite(pn, 0);
		analogWrite(p, value);
	}
};

void setup(){
	pinMode(mfr, OUTPUT);
	pinMode(mfrn, OUTPUT);
	pinMode(mbr, OUTPUT);
	pinMode(mbrn, OUTPUT);
	pinMode(mbl, OUTPUT);
	pinMode(mbln, OUTPUT);

	Wire.begin(8);
	Wire.onReceive(updateMotors);
}

void loop(){
	delay(10);
}

void updateMotors(int howMany){
	//i2c
	//first byte sent has forward and sideways motion within it
	side  =Wire.read();
	forth =side  & 0xf0;
	side  =side  << 4;
	//second byte sent has vertical and angular motion; we only need second half
	turn  =Wire.read() & 0xf0;
	
	//edit motors
	//front right
	fr= forth-side -(turn/4);
	//back  right
	br=-forth-side +(turn/4);
	//back  left
	bl=-forth+side -(turn/4);
	
	//write to whichever pin of the pair going to a certain h-bridge has correct polarity
	split_write(mfr, mfrn, fr);
	split_write(mbr, mbrn, br);
	split_write(mbl, mbln, bl);
}
