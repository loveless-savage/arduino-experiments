//topside- unidirectional

//pins 5, 6 and 7 are the data ID pins: which type of movement?
//pins 8 and 9 are the scalar data pins: magnitude of movement (2 bits, 0-3)
//all pins are edited directly through DPM

int ax[5];//array for transforming controller data- see syntax document

int p;//variable for holding the selected axis
uint8_t reg;//holds ID values to be written to pins

////////////////////////////////////////////////////////////

void setup(){
	Serial.begin(115200);
	for(char i=5;i<10;i++){//even though we are using DPM instead of digitalWrite(), we need to set pins 5-9 to output
		pinMode(i, OUTPUT);//see syntax document for details on DPM
	}
	ax[4]=0;//buffer variable for when controller is idle
}

void loop(){

	//loop through all potentiometers
	for(char i=0; i<4; i++){
		ax[i]=analogRead(i);//read each potentiometer
		ax[i]>>=7;//shrink reading to 3 bits with a bit shift
		ax[i]-=4;//translate so range is half negative
	}

	reg=0;//clear the pin buffer
////////////////////////////////////////////////////////////

/*the topside arduino takes all controller axis readings and picks one based on this priority list:
 *	-sideways
 *	-back/forth; overcomes sideways axis if reading is bigger
 *	-angular
 *	-vertical
 *once it selects an axis and transforms the data, it writes an ID and scalar value to the data pins

 *the ID takes 3 pins- 5 to 7- and tells what direction:
 *	-111 = forward
 *	-110 = right
 *	-101 = left
 *	-100 = backward
 *	-011 = clockwise
 *	-010 = counterclockwise
 *	-001 = up
 *	-000 = down

 *the scalar takes 2 pins- 8 and 9- and writes a 2-bit binary scalar value
 *allows for 4 magnitudes of velocity in whatever the direction is
*/

//prioritize lateral movements over angular or vertical
	if(ax[0]<-1 || ax[0]>0 || ax[1]<-1 || ax[1]>0){//if any of the lateral joystick axes are off of zero
		reg |= 0b100;//bit 3 signifies whether or not movement is lateral
		p=abs(ax[1]) > abs(ax[0]);//select whichever axis is greater

		if(ax[p]>0){//test if the selected axis is positive or negative
			reg |= 0b010;//bit 2 on shows the lateral direction is positive
		}

		if(p){//forward axis
			reg |= (reg&2)>>1;//last bit same as one before
		}else{
			reg |=(~reg&2)>>1;//last bit opposite of one before
		}

//now we move on to the angular and vertical values: angular takes priority but has lower sensitivity
	}else if(ax[2]<-2 || ax[2]>1){
		reg |= 0b010;//bits 3 and 2 are 01 when motion is angular
		p=2;//selected axis is 2
		reg |= (ax[2]>0);//last bit is positive if the motion is clockwise, and vice versa

//finally we reach the vertical movement, which can be up or down
	}else{
		reg |= (ax[3]>0);//001 is the ID for vertical upward movement
		p=3;//selected axis is 3
	}

////////////////////////////////////////////////////////////

	//PORTB and PORTD are direct port manipulation (DPM)- see syntax document
	//numbers that start with 0b are binary numbers, like 0b00011111

	PORTD &= 0b00011111;//clear pins 5-7 using bitwise AND assignment operator
	PORTD |= reg << 5;//write reg to pins 5-7 using bitwise OR assignment operator

	if(ax[p]<0){//if selected axis is negative...
		ax[p]=~ax[p];//...invert it with a bitwise NOT
	}

	PORTB &= 0b11111100;//clear pins 8-9 using bitwise AND assignment operator
	PORTB |= ax[p];//write magnitude of selected axis to pins 8-9 using bitwise OR assignment operator

////////////////////////////////////////////////////////////

//debugging printout
	Serial.print(reg, BIN);//print ID variable value in binary, using the BIN keyword
	Serial.print("\t");//text shortcut for tab symbol
	Serial.print(PORTD>>5, BIN);//print pins 5-7, in binary as well
	Serial.print("\t");//tab
	Serial.print(PORTB, BIN);//print pins 8-9 in binary
	Serial.print("\t");//tab
	Serial.println(ax[p]);//print magnitude of selected axis
}

