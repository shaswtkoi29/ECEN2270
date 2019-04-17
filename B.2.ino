/*
ECEN2830 motor position control example (left wheel only)
*/

// define pins
const int pinON = 6;         // connect pin 6 to ON/OFF switch, active HIGH
const int pinCW_Left = 8;    // connect pin 7 to clock-wise PMOS gate
const int pinCC_Left = 7;    // connect pin 8 to counter-clock-wise PMOS gate
const int pinSpeed_Left = 9; // connect pin 9 to speed reference
const int pinCW_Right = 11;    // connect pin 11 to clock-wise PMOS gate
const int pinCC_Right = 12;    // connect pin 12 to counter-clock-wise PMOS gate
const int pinSpeed_Right = 10; // connect pin 10 to speed reference
const int speedDiff = 35;
const int encperinch = 13*51*(3/2.75)/(2.55906*2*PI);

// encoder counter variable
volatile int enc_count_Left = 0; // "volatile" means the variable is stored in RAM
volatile int enc_count_Right = 0; 

// setup pins and initial values
void setup() {
  Serial.begin(9600);
  pinMode(pinON,INPUT);
  pinMode(pinCW_Left,OUTPUT);
  pinMode(pinCC_Left,OUTPUT);
  pinMode(pinSpeed_Left,OUTPUT);
  pinMode(pinCW_Right,OUTPUT);
  pinMode(pinCC_Right,OUTPUT);
  pinMode(pinSpeed_Right,OUTPUT);
  pinMode(13,OUTPUT);             // on-board LED
  digitalWrite(13,LOW);           // turn LED off
  digitalWrite(pinCW_Left,LOW);   // stop clockwise
  digitalWrite(pinCC_Left,LOW);   // stop counter-clockwise
  digitalWrite(pinCW_Right,LOW);   // stop clockwise
  digitalWrite(pinCC_Right,LOW);   // stop counter-clockwise
  analogWrite(pinSpeed_Left,138);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,110);
  /* 
    Connect left-wheel encoder output to pin 2 (external Interrupt 0) via a 1k resistor
    Rising edge of the encoder signal triggers an interrupt 
    count_Left is the interrupt service routine attached to Interrupt 0 (pin 2)
  */
  attachInterrupt(0, count_Left, RISING);
  attachInterrupt(1, count_Right, RISING);
}

/*
  Interrupt 0 service routine
  Increment enc_count_Left on each rising edge of the 
  encoder signal connected to pin 2
*/ 
void count_Left(){
  enc_count_Left++;
}

void count_Right(){
  enc_count_Right++;
}
int forward( int distance, int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  analogWrite(pinSpeed_Left,spd+speedDiff);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,spd);
  digitalWrite(pinCC_Left,HIGH);            // go forward
  digitalWrite(pinCW_Right,HIGH);
  int conversion = distance*encperinch; //distance in inches/wheel circumf*encoder per rotation*conversion factor
  do {
    if(enc_count_Right >= conversion){
      digitalWrite(pinCW_Right,LOW);
    }
    if(enc_count_Left >= conversion){
      digitalWrite(pinCC_Left,LOW);
    }
    
    } while ((enc_count_Right < conversion) && (enc_count_Left < conversion));
  digitalWrite(pinCC_Left,LOW);            
  digitalWrite(pinCW_Right,LOW);
  }
int reverse( int distance, int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  analogWrite(pinSpeed_Left,spd+speedDiff);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,spd);
  digitalWrite(pinCW_Left,HIGH);            // go forward
  digitalWrite(pinCC_Right,HIGH);
  int conversion = distance*encperinch; //distance in inches/wheel circumf*encoder per rotation*conversion factor
  do {} while ((enc_count_Right < conversion) && (enc_count_Left < conversion));
  digitalWrite(pinCW_Left,LOW);            
  digitalWrite(pinCC_Right,LOW);
  }
int turn_left( int deg, int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  analogWrite(pinSpeed_Left,spd);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,spd);
  digitalWrite(pinCW_Left,HIGH);            // go forward
  digitalWrite(pinCW_Right,HIGH);
  int conversion = 5.5*deg*PI/180*encperinch; //robot radius in inches*angle in radians/wheel circumf*encoder per rotation*conversion factor
  do {} while (((enc_count_Right <= conversion) && (enc_count_Left <= conversion)));
  digitalWrite(pinCW_Left,LOW);            
  digitalWrite(pinCW_Right,LOW);
  }
int turn_right( int deg, int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  analogWrite(pinSpeed_Left,spd);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,spd);
  digitalWrite(pinCC_Left,HIGH);            // go forward
  digitalWrite(pinCC_Right,HIGH);
  int conversion = 5.5*deg*PI/180*encperinch; //robot radius in inches*angle in radians/wheel circumf*encoder per rotation*conversion factor
  do {} while ((enc_count_Right < conversion) && (enc_count_Left < conversion));
  digitalWrite(pinCC_Left,LOW);            
  digitalWrite(pinCC_Right,LOW);
  }

void loop() {
  do {
    enc_count_Left = 0; // reset encoder counter to 0
    enc_count_Right = 0;
  } while (digitalRead(pinON) == LOW);      // wait for ON switch
  digitalWrite(13,HIGH);                    // turn LED on
  delay(1000);
  forward(2*12,100);
  
  delay(500);
  turn_right(180,100);
  
  delay(500);
  forward(2*12,100);
 
  delay(500);
  turn_left(180,100);
  

  delay(1000);                              // wait 1 second
}
