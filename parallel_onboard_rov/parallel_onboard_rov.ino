//onboard- parallel
//finished, but does not work
//some code syntax is documented in other arduino programs



//these are compile instructions
//when any of these is 0, the corresponding part of the document is not compiled
//that is, the compiler on the computer just skips over it- the final program doesn't have that part
//switch these on and off for debugging purposes, when you are only testing one part of the code
#define COM 1 //run communication code
#define DEPTH 0 //run depth sensor code
#define MOTORS 0 //run motor control code

#define rst 2//remote status pin
//pins 3-5 are the remote data pins, read by direct port manipulation- see syntax document
#define ls1 7//local clock, bit 1
#define ls2 8//local clock, bit 2
#define lda 12//local data- for sending depth value

#define mfl 10//motor pin: front left positive
#define mfln 11//motor pin: front left negative
#define mu 6//motor pin: vertical positive
#define mun 9//motor pin: vertical negative

#include <Wire.h>//for communicating with the onboard slave arduino- see syntax document

bool rstate=0;//store remote clock state

uint8_t depth;//hold the depth sensor value

char j;//loop index for communication

int8_t axes[4];//this holds all the axes for transformation
int8_t forth, side, turn, vr;//store all the axis values
int8_t fl;//store front left motor value

//toggle(), waitcom(), readcom(), and sendcom() are all part of the parallel communication protocol
//see document header for details

//signal to topside for next data value
void toggle(){
	digitalWrite(ls2, j & 2);//write whatever the clock says for the second bit
	digitalWrite(ls1, j & 1);//write whatever the clock says for the first bit
}

//wait for topside while it copies down the next axis to the remote data pins
void waitcom(){
	while(digitalRead(rst) == rstate){//waits until the remote clock toggles
	}

	rstate = digitalRead(rst);//reset the remote clock variable, so it expects what the remote clock pin is not
}

int8_t readcom(){
	return PIND & 0b00111000 << 2;//grab pins 3 through 5 all at once
}

void sendcom(){
	digitalWrite(lda, bitRead(depth, j));
}

void transform_axes(){//TODO
	for(char i=0; i<4; i++){
		if(axes[i] >> 7){//if the sign bit is on- that is, if the value is positive
			axes[i]^= 1<<7;//switch off the sign bit- this will ensure the signed int is interpreted correctly
		}else{
			axes[i]-= 1<<7;//translate the 'negative' values into the actual negative zone of the signed int range
		}
	}
}

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
	pinMode(ls1, OUTPUT);//first local clock bit
	pinMode(ls2, OUTPUT);//second local clock bit
	pinMode(rst, INPUT);//remote status bit
	pinMode(lda, OUTPUT);//local data bit
	
	digitalWrite(ls1, LOW);//set up the local clock bits, in case they were set to high in a previous program
	digitalWrite(ls2, LOW);

	pinMode(mfl, OUTPUT);//front left motor- positive
	pinMode(mfln, OUTPUT);//front left motor- negative
	pinMode(mu, OUTPUT);//vertical motors- positive
	pinMode(mun, OUTPUT);//vertical motors- negative
	
//here is another compiler instruction!
//if MOTORS was set to zero, the compiler skips to the next '#endif'
#if MOTORS
	Wire.begin();//start communication with onboard slave
#endif

	depth=0b00001011;//numbers starting with 0b are binary values

	Serial.begin(115200);//this is for debugging- remove when functional
	
}

void loop(){
//sensors///////////////////////////////////////////////////

#if DEPTH
	depth=(uint8_t)analogRead(3);//depth sensor is connected to analog pin 3: type cast to uint8_t- see syntax document
	depth-=99;//air pressure is 100, so we take that off- although leave 1 on for rounding up
	depth>>=2;//divide by 4 to get to number of feet of water
#endif
//com///////////////////////////////////////////////////////
#if COM
	Serial.println("com");

//start the com loop
	for(j= 3 ; j>=0; j--){//onboard calls for 4 axes, one at a time
	Serial.print("j=");
	Serial.println((int)j);//type cast j from char to int

//only send data if onboard is reading the depth sensor
#if DEPTH
		sendcom();//send depth bit number j
	Serial.println("sent!");
#endif

		toggle();//change the clock values
	Serial.println("toggled");

		waitcom();//wait for topside to give the go-ahead before reading
	Serial.println("got something!");

		axes[j]=readcom();//condense bits from remote data pins into axes[j]
	Serial.println(readcom());
	}


#endif
//i2c///////////////////////////////////////////////////////
#if MOTORS
	Wire.beginTransmission(8);
	Wire.write(axes[0]);
	Wire.write(axes[1]);
	Wire.write(axes[2]);
	Wire.endTransmission(8);
#endif
//motors////////////////////////////////////////////////////
#if MOTORS
	//front left motor
	fl=forth+side+turn;
	split_write(mfl, mfln, fl);

	//vertical motors
	split_write(mu, mun, vr);
#endif
}

