//topside- vertical only

//these are compile macros: the compiler sees this and replaces all instances of these strings in the document with their corresponding value
//thus they don’t take any extra memory as variables, and they cannot be edited
#define pin_bit1 5 //pin#5 is the first bit of the magnitude
#define pin_bit2 6 //pin#6 is the second bit of the magnitude
#define pin_sign 7 //pin#7 is on when motion is negative
#define pin_leak 8 //pin#8 listens for leak signal from onboard

int vert;//stores the magnitude value

void setup(){
	Serial.begin(115200);//print things to the console
	pinMode(pin_leak, INPUT);
	pinMode(pin_sign, OUTPUT);
	pinMode(pin_bit1, OUTPUT);
	pinMode(pin_bit2, OUTPUT);
}

void loop(){
	vert=analogRead(1);//store the potentiometer value of pin A1

	//change the range of values: 0-1023 to 0-7
	vert>>=7;//this is called a bit shift assignment operator- see syntax document
	
	/*pins 6 and 5 work together to send a 2-bit, 4-value scalar
	 *6 and 5 can either be 00, 01, 10, or 11 respectively
	 *these correspond to the binary values 0-3
	 *combined, they can send those four motor speeds to onboard
	 *thus the vertical motors can be controlled with variable speed
	 *along with pin 7, they can direct the motors in 7 speeds positive or negative
	*/

	if(vert<0b100){//negative values if the potentiometer is below center position
		digitalWrite(pin_sign,HIGH);//turn on the sign pin to show it's negative

		digitalWrite(pin_bit2,~vert&2);//write negative value of negative number: bit #2 in vert to pin_bit2
		digitalWrite(pin_bit1,~vert&1);//squiggle is a bitwise NOT: see syntax document
	}else{
		digitalWrite(pin_sign,LOW);//sign pin is off when joystick value is positive

		digitalWrite(pin_bit2,vert&2);//write bit #2 in vert to pin_bit2
		digitalWrite(pin_bit1,vert&1);//single '&' is a bitwise AND function: see syntax document
	}

	if(digitalRead(pin_leak)){//signal from onboard arduino that leak detector is going off
		Serial.println("warning- leak possible");
	}
}
