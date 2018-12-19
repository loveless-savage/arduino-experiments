/*topside
 *controller
 *com
 *serial
 */

#define CONT 1
#define COM 1

#define cal 3
#define res 4
#define lck 5
#define rck 7
#define lda 8
#define rda 12

bool clstate=0;//clock state
bool restate=0;//reset pin state
uint16_t sensordata, motordata, caliper;
uint8_t joya, joyv, joyx, joyy;
int8_t joyt;

void* calib;//stores label

//for compressing controller information
int8_t transf_axis(int inputaxis){
	joyt=analogRead(inputaxis) >> 2;//read whichever potentiometer and shrink it to 8 bits
	joyt^=1<<7;//this flips the sign bit: in effect, this translates zero to the center potentiometer state
	if(joyt<112){
		joyt+=16;//position zero is now to either side of the center point, not just above it
	}
	joyt&=0b11100000;//reduce the resolution of the measurement, so only the three least precise bits are kept
	return joyt;
}

void toggle(){
	clstate = digitalRead(rck);//expects what the remote clock was not
	digitalWrite(lck, !digitalRead(lck) );//flips the pin
}

void waitcom(void* calib){
	while(clstate==digitalRead(rck)){//waits until the remote clock toggles

		if(restate!=digitalRead(res)){//if, by chance, the boards get out of sync and the reset pin toggles
			Serial.println("AAAAAAHHHHHH!!!! recalibrate!");
			toggle();
			restate=!restate;
			goto *calib;
		}
	}
}

void readcom(char i){
	if(digitalRead(rda)){
		bitSet(sensordata, i);//if onboard sent a 1
	}else{
		bitClear(sensordata, i);//if onboard sent a 0
	}
}

void sendcom(char i){
	digitalWrite(lda, bitRead(motordata, i));//actual data
	digitalWrite(cal, bitRead(caliper, i ));//caliper byte
}

void setup(){
	pinMode(cal, OUTPUT);
	pinMode(res, INPUT);
	pinMode(lck, OUTPUT);
	pinMode(rck, INPUT);
	pinMode(lda, OUTPUT);
	pinMode(rda, INPUT);
	
	digitalWrite(res, LOW);
	digitalWrite(lck, LOW);
	
	Serial.begin(115200);
	caliper=0b1011001111000010;//this helps the boards sync while in the COM sequence
}

void loop(){
	//label- this is a flag for the goto statements, if the boards get out of sync
beginloop:
	calib=&&beginloop;
//controller////////////////////////////////////////////////
#if CONT
	//Serial.println("I hear your instructions, master");
	//grab all the potentiometer values and transform them
	joya=transf_axis(2);
	joyv=transf_axis(3);
	joyx=transf_axis(0);
	joyy=transf_axis(1);
	
	//stuff all the numbers into one variable- bitshifts yeah!
	motordata= joya << 8 | joyv << 4 | joyx | joyy >> 4;
	//topside will send motordata down to onboard, which will pull it apart and interpret it
	Serial.print(joya);
	Serial.print("\t");
	Serial.print(joyv);
	Serial.print("\t");
	Serial.print(joyx);
	Serial.print("\t");
	Serial.println(joyy);
#endif
//com///////////////////////////////////////////////////////
#if COM
	Serial.println("loop");
	for(char i= 0x0f ; i>=0; i--){
		sendcom(i);
		toggle();
		waitcom(calib);
		readcom(i);
	}
	Serial.println("com finished");
#endif
//serial////////////////////////////////////////////////////
/*
	Serial.print("compass value = ");
	Serial.println(sensordata >> 7);
	Serial.print("depth = ");
	Serial.println(sensordata & 0xff);
	Serial.println("");
*/
}

