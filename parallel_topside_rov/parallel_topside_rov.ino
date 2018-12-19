//topside- parallel


#define CONT 1
#define COM 1

//pin 4 is the local status pin
#define lst 4
//pins 5, 6 and 7 are the local data pins
//pins 8 and 9 together make the onboard clock (2 bits, 0-3)
//pin 10 is the onboard data variable
#define rda 10

uint8_t axes[4];//all the controller data
int joyt;//temp variable for transforming controller data

char j;//stores which bit the onboard wants

uint8_t depth;//grab the depth readout from onboard

//for compressing controller information
void grab_axes(){
	for(char i=0; i<4; i++){//loop through all potentiometers
		joyt=analogRead(i);//read each potentiometer
		joyt>>=7;//shrink reading to 3 bits
		if(joyt < 0b100){
			joyt+=1;//negative motion values are rounded to the ceiling, not the floor
		}
		axes[i]= joyt;
	}
}

void waitcom(){
	while(j & 1==PORTB & 1){//waits until the last bit of the remote clock toggles
	}
	j=PORTB & 0b00000011;
}

void readcom(){
	if(digitalRead(rda)){//copy the remote data value to depth
		bitSet(depth, j);//if onboard sent a 1
	}else{
		bitClear(depth, j);//if onboard sent a 0
	}
}

void sendcom(){
	PORTD &= 0b00011111;//clears pins 5-7
	PORTD |= axes[j] << 5;//writes whichever joytick value onboard wants to pins 5-7
}

void toggle(int pinnum){
	PORTD ^= 1<<pinnum;//flips pin 4, the local status pin
}


////////////////////////////////////////////////////////////

void setup(){
	Serial.begin(115200);
}

void loop(){
//com///////////////////////////////////////////////////////
#if COM
	Serial.println("com");

	waitcom();
	Serial.println("got something!");

	Serial.print("j=");
	Serial.println((int)j);

	readcom();
	Serial.println("read\t");
	Serial.println(digitalRead(rda));

	sendcom();
	Serial.print("sent\t");
	Serial.println(axes[j]);

	toggle(lst);

	Serial.println("com finished");
#endif
//controller////////////////////////////////////////////////
	//Serial.println("I hear your instructions, master");
	//grab all the potentiometer values and transform them
#if CONT
	if(!j){
		grab_axes();
	}
#endif
//serial////////////////////////////////////////////////////
	//topside will send motordata down to onboard, which will pull it apart and interpret it
	Serial.print(axes[j] | 0b10000000, BIN);
}

