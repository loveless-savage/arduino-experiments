//onboard- unidirectional
//finished, but does not work

//pins 2,4,7 are the ID value: what direction
int ID[3]={2,4,7};//hold these pin values
//pins 8,12 are the scalar value: how fast

int p1,p2;//selected motor pin numbers: only two are ever going at once
int mp[6]={3,9,10,11,5,6};//holds motor pin numbers in an array
uint8_t scl=0;//store scalar value

uint8_t deb[6];//debugging, read pin values

void setup(){
	Serial.begin(115200);//this is only for debugging: once code is operational, remove all Serial functions

	for(char i=0; i<6; i++){//initiate all motor pins
		pinMode(mp[i], OUTPUT);
	}
}

void loop(){
	restart://this is a label- other parts of the program can refer to this point and go back to it
	//any word can be a label, not just 'restart'- just put a colon after it

	//debugging code: loop through each of the 6 motor pin states
	for(char i=0;i<6;i++){
		if(deb[i]){//if the currently selected pin was set to HIGH, print out 1
		Serial.print(1);
		}else{//if the currently selected pin was set to LOW, print out 0
		Serial.print(0);
		}
	}

	//more debugging code: print out the states of the data pins from topside
	Serial.print("\t");//tab
	Serial.print(digitalRead(ID[2]));//ID pins
	Serial.print(digitalRead(ID[1]));
	Serial.print(digitalRead(ID[0]));
	Serial.print("\t");//tab
	Serial.print(digitalRead(12));//scalar pins
	Serial.print(digitalRead(8));
	//all the debugging prints to one line per loop, carriage returning after all the printouts are complete
	Serial.println();


//collect scalar value
	scl=0;//clear scalar value stored from last frame
	scl |= digitalRead(12)<<7;//grab state of pin 12 and insert to bit 8, using a bitwise OR assignment operator and a bit shift
	scl |= digitalRead(8)<<6;//grab state of pin 8 and insert to bit 7, using a bitwise OR assignment operator and a bit shift
	
//wipe all motor pins
	for(char i=0;i<6;i++){//loop through the 6 pins
		digitalWrite(mp[i],0);//clear literal pin values
		deb[i]=0;//clear debugging buffer
	}


////////////////////////////////////////////////////////////
	/*now we select the two motors to move
	 *with all but vertical directions, only two motors ever move at once
	 *so based on the ID, we select two of the four lateral motors

	 *motors are laid out as follows:
	 *	    (front)
	 *	    [0] [1]
	 *	[4/5] ROV [4/5]
	 *	    [3] [2]

	 *motors 4/5 are vertical: they are bi-directional, and bound
	 *they are wired each with a positive lead and a negative lead from their H-bridges
	 *connect both positive leads to pin mp[4] and both negative leads to pin mp[5]
	 *so pin mp[4] corresponds to positive movement of both vertical motors, and pin mp[5] corresponds to negative

	 *when we take the ID we can identify logical patterns

	 *ID	 motor  0?  1?  2?  3?  4?  5?
	 -------------------------------------
	 *111		X   X                
	 *110		X           X        
	 *101		    X   X            
	 *100		        X   X        
	 -------------------------------------
	 *011		X       X            
	 *010		    X       X        
	 -------------------------------------
	 *001		                X    
	 *000		                    X

	 *we select whichever two motors to run, unless vertical or idle
	 *then we store the pin numbers into p1 and p2
	 *at the end, we write the scalar value to these two pins

	 *in the case of vertical motion, the scalar value is written to either mp[4] or mp[5]
	 *after that, the loop is restarted using the label 'restart'

	 *finally, in the case of scalar with a value of zero, skip the process entirely
	 *just wait for a moment and goto label 'restart'
	*/


//if ID bit 2 is on, motion is lateral
	if(digitalRead(ID[2])){
		//motor 0 if ID bit 1 is on, motor 2 if ID bit 1 is off
		p1=digitalRead(ID[1])? mp[0]:mp[2];

		//motor 1 if ID bit 0 is on, motor 3 if ID bit 0 is off
		p2=digitalRead(ID[0])? mp[1]:mp[3];

//else if ID bit 1 is on, motion is angular
	}else if(digitalRead(ID[1])){
		//motor 0 if ID bit 0 is on, motor 1 if ID bit 0 is off
		p1=digitalRead(ID[0])? mp[0]:mp[1];

		//motor 2 if ID bit 0 is on, motor 3 if ID bit 0 is off
		p2=digitalRead(ID[0])? mp[2]:mp[3];

//else if ID bit 0 is on, motion is vertical up
	}else if(digitalRead(ID[0])){
		analogWrite(mp[5],0);//wipe the vertical down pin
		analogWrite(mp[4],scl);//write scalar to the vertical up pin
		goto restart;//go to the label 'restart', which restarts the loop

//else if scalar is non-zero, motion is vertical down
	}else if(scl){
		analogWrite(mp[4],0);//wipe the vertical up pin
		analogWrite(mp[5],scl);///write scalar to the vertical down pin
		goto restart;//go to the label 'restart', which restarts the loop
//else if scalar is zero, ROV is idle
	}else{
		delay(5);//give the processor a rest for 5 milliseconds
		goto restart;//go to the label 'restart', which restarts the loop
	}

////////////////////////////////////////////////////////////

//write to proper motors
	analogWrite(p1, scl);//write scalar to first of two selected pins
	analogWrite(p2, scl);//write scalar to second of two selected pins

	deb[p1]=scl;//write scalar to debugging spot of first of two selected pins
	deb[p2]=scl;//write scalar to debugging spot of second of two selected pins
}

