//onboard- vertical only

//these are compile macros: the compiler sees this and replaces all instances of these strings in the document with their corresponding value
//thus they don’t take any extra memory as variables, and they cannot be edited
#define pin_bit1 5 //pin#5 is the first bit of the magnitude
#define pin_bit2 6 //pin#6 is the second bit of the magnitude
#define pin_sign 7 //pin#7 is on when motion is negative

#define pin_leak 8 //pin#8 sends leak signal to topside
#define pin_mvup 10//pin#10 is the vertical up motion value
#define pin_mvdn 11//pin#11 is the vertical down motion value

int vert;//stores the magnitude value

int leak_cutoff=255;//cutoff analogRead value for leak detector being on/off
//important note- value of leak_cutoff is arbitrary, and needs testing to find the best number

void setup(){
	pinMode(pin_bit1,INPUT);//accept bits 1 and 2, as well as sign bit, from topside
	pinMode(pin_bit2,INPUT);
	pinMode(pin_sign,INPUT);

	pinMode(pin_leak,OUTPUT);//send leak signal to topside
	pinMode(pin_mvup,OUTPUT);//send pin_mvup and pin_mvdn to motors
	pinMode(pin_mvdn,OUTPUT);
}

void loop(){
	vert = 0;//reset vert every loop

	//to rewrite bits in vert, use "|=", a bitwise OR assignment operator- see syntax document
	//double carrots are bitshifts- see syntax document
	vert |= digitalRead(pin_bit2)<<8;//write bit 9 of vert as state of pin_bit2
	vert |= digitalRead(pin_bit1)<<7;//write bit 8 of vert as state of pin_bit1

	analogWrite(digitalRead(pin_sign)? pin_mvup:pin_mvdn ,0);//write 0 to up pin if sign pin is on, otherwise to down pin
	analogWrite(digitalRead(pin_sign)? pin_mvdn:pin_mvup ,vert);//write scalar motion value to down pin if sign pin is on, otherwise to up pin

	if(analogRead(0)>leak_cutoff){//if the leak detector at pin A0 reads more current than the cutoff value
		digitalWrite(pin_leak, HIGH);//send leak signal
	}else{//clear the leak signal if the leak detector stops reading a leak
		digitalWrite(pin_leak, LOW);//send no-leak signal
	}
	
}
