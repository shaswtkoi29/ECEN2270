/*
ECEN2270 motor position control example (left wheel only)
*/

// define pins
const int pinON = 6;         // connect pin 6 to ON/OFF switch, active HIGH
const int pinCW_Left = 8;    // connect pin 7 to clock-wise PMOS gate
const int pinCC_Left = 7;    // connect pin 8 to counter-clock-wise PMOS gate
const int pinSpeed_Left = 9; // connect pin 9 to speed reference
const int pinCW_Right = 11;    // connect pin 11 to clock-wise PMOS gate
const int pinCC_Right = 12;    // connect pin 12 to counter-clock-wise PMOS gate
const int pinSpeed_Right = 10; // connect pin 10 to speed reference
const float speedDiff = 30/70;
const int encperinch = 13*51*(3/2.75)/(2.55906*2*PI);
const int crash = 7;
const int safeDist = 11;
volatile int totaldistance = 0;

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
  
  //distance sensing pins
  pinMode(A5, OUTPUT);//left trig
  pinMode(A4, INPUT);//left echo
  pinMode(A3, OUTPUT);//front trig
  pinMode(A2, INPUT);//front echo
  pinMode(A1, OUTPUT);//right trig
  pinMode(A0, INPUT);//right echo
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

//distance sensing functions
long leftFunc(){
  digitalWrite(A5, LOW);
  delayMicroseconds(5);
  digitalWrite(A5, HIGH);
  delayMicroseconds(10);
  digitalWrite(A5,LOW);
  long duration = pulseIn(A4,HIGH);
  long inches = (duration/2)/74;
  return inches;
}

long rightFunc(){
  digitalWrite(A1, LOW);
  delayMicroseconds(5);
  digitalWrite(A1, HIGH);
  delayMicroseconds(10);
  digitalWrite(A1,LOW);
  long duration = pulseIn(A0,HIGH);
  long inches = (duration/2)/74;
  return inches;
}

long frontFunc(){
  digitalWrite(A3, LOW);
  delayMicroseconds(5);
  digitalWrite(A3, HIGH);
  delayMicroseconds(10);
  digitalWrite(A3,LOW);
  long duration = pulseIn(A2,HIGH);
  long inches = (duration/2)/74;
  return inches;
}

int safeDelay(){
  int encInit = enc_count_Right;
  do{}while((enc_count_Right-encInit) < encperinch*safeDist);
}

int pathCorrectHelper(int spd, int dir){
  enc_count_Left = 0;
  enc_count_Right = 0;
  delay(500);
  //just turned left
  if(dir == 1){
    digitalWrite(pinCC_Left,HIGH);            
    digitalWrite(pinCW_Right,HIGH);
    while(rightFunc() <= crash){
      if(frontFunc() <= crash ){
        pathCorrect(spd);
        //check this out
      }
      
    }
    safeDelay();
    digitalWrite(pinCC_Left,LOW);            
    digitalWrite(pinCW_Right,LOW);
    delay(500);
    turn_right(90, spd);
    delay(500);
    int enc_dist = enc_count_Right;
    enc_count_Left = 0;
    enc_count_Right = 0;
    digitalWrite(pinCC_Left,HIGH);            
    digitalWrite(pinCW_Right,HIGH);
    safeDelay();
    while(rightFunc() <= crash){
      if(frontFunc() <= crash){
        pathCorrect(spd);
      }
    }
    safeDelay();
    digitalWrite(pinCC_Left,LOW);            
    digitalWrite(pinCW_Right,LOW);
    delay(500);
    turn_right(90, spd);
    delay(500);
    totaldistance = totaldistance + enc_count_Right;
    enc_count_Left = 0;
    enc_count_Right = 0;
    digitalWrite(pinCC_Left,HIGH);            
    digitalWrite(pinCW_Right,HIGH);
    do{
      if(frontFunc() <= crash){
        pathCorrect(spd);
      }
     }while(enc_count_Right < enc_dist);
     digitalWrite(pinCC_Left,LOW);            
     digitalWrite(pinCW_Right,LOW);
     delay(500);
     turn_left(80, spd);
     delay(500);
     digitalWrite(pinCC_Left, HIGH);
     digitalWrite(pinCW_Right, HIGH);
  }
  //just turned right
  else if(dir == 2){
    digitalWrite(pinCC_Left,HIGH);            
    digitalWrite(pinCW_Right,HIGH);
    while(leftFunc() <= crash){
      if(frontFunc() <= crash){
        pathCorrect(spd);
      }
    }
    safeDelay();
    digitalWrite(pinCC_Left,LOW);            
    digitalWrite(pinCW_Right,LOW);
    delay(500);
    turn_left(80, spd);
    delay(500);
    int enc_dist = enc_count_Right;
    enc_count_Left = 0;
    enc_count_Right = 0;
    digitalWrite(pinCC_Left,HIGH);            
    digitalWrite(pinCW_Right,HIGH);
    safeDelay();
    do{
      
      if(frontFunc() <= crash){
        pathCorrect(spd);
      }
    }while(leftFunc() <= crash);
    safeDelay();
    digitalWrite(pinCC_Left,LOW);            
    digitalWrite(pinCW_Right,LOW);
    delay(500);
    turn_left(80, spd);
    delay(500);
    totaldistance = totaldistance + enc_count_Left;
    enc_count_Left = 0;
    enc_count_Right = 0;
    digitalWrite(pinCC_Left,HIGH);            
    digitalWrite(pinCW_Right,HIGH);
    do{
      if(frontFunc() <= crash){
        pathCorrect(spd);
      }
     }while(enc_count_Right < enc_dist);
     digitalWrite(pinCC_Left,LOW);            
     digitalWrite(pinCW_Right,LOW);
     delay(500);
     turn_right(80, spd);
     delay(500);
     digitalWrite(pinCC_Left, HIGH);
     digitalWrite(pinCW_Right, HIGH);
  }
}

int pathCorrect(int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  digitalWrite(pinCC_Left,LOW);            
  digitalWrite(pinCW_Right,LOW);
  digitalWrite(pinCW_Left,LOW);            
  digitalWrite(pinCC_Right,LOW);
  delay(500);
  if((frontFunc() <= crash) && (leftFunc() <= crash) && (rightFunc() <= crash)){
    turn_right(360, spd);
    pathCorrect(spd);
  }
  else if((leftFunc() <= crash) && (rightFunc() <= crash)){
    analogWrite(pinSpeed_Left,spd+spd*speedDiff);  
    analogWrite(pinSpeed_Right,spd);
    digitalWrite(pinCC_Left,HIGH);           
    digitalWrite(pinCW_Right,HIGH);
    do{}while((leftFunc() <= crash) && (rightFunc() <= crash));
    delay(500);
    digitalWrite(pinCC_Left,LOW);           
    digitalWrite(pinCW_Right,LOW);
    if(leftFunc() > crash){
      turn_left(90, spd);
      totaldistance = totaldistance - enc_count_Left;
      pathCorrectHelper(spd, 1);
    }
    else if(rightFunc() > crash){
      turn_right(90, spd);
      totaldistance = totaldistance - enc_count_Right;
      pathCorrectHelper(spd, 2);
    }
    else{
      //in case while loop was broken by an erant signal
      pathCorrect(spd);
    }
  }
  else if((leftFunc() <= crash) && (frontFunc() <= crash)){
    turn_right(90,spd);
    pathCorrectHelper(spd, 2);
  }
  else if((rightFunc() <= crash) && (frontFunc() <= crash)){
    turn_left(90, spd);
    pathCorrectHelper(spd, 1);
  }
  else{
    turn_right(90,spd);
    pathCorrectHelper(spd, 2);
  }
}

int forward( int distance, int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  analogWrite(pinSpeed_Left,spd+spd*speedDiff);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,spd);
  digitalWrite(pinCC_Left,HIGH);            // go forward
  digitalWrite(pinCW_Right,HIGH);
  int conversion = distance*encperinch; //distance in inches/wheel circumf*encoder per rotation*conversion factor
  do {
    
    long currDist = frontFunc();
    if(currDist <= crash){
      totaldistance = totaldistance + enc_count_Right;
      pathCorrect(spd);
    }
    if(totaldistance + enc_count_Right >= conversion){
      break;
    }
    } while ((enc_count_Right < conversion) && (enc_count_Left < conversion));
  digitalWrite(pinCC_Left,LOW);            
  digitalWrite(pinCW_Right,LOW);
  }

  
int reverse( int distance, int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  analogWrite(pinSpeed_Left,spd+spd*speedDiff);  // set speed reference, duty-cycle = 50/255
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
  analogWrite(pinSpeed_Left,spd+spd*speedDiff);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,spd);
  digitalWrite(pinCW_Left,HIGH);            // go forward
  digitalWrite(pinCW_Right,HIGH);
  int conversion = 8.5*deg*PI/180*encperinch; //robot radius in inches*angle in radians/wheel circumf*encoder per rotation*conversion factor
  do {} while (((enc_count_Right <= conversion) && (enc_count_Left <= conversion)));
  digitalWrite(pinCW_Left,LOW);            
  digitalWrite(pinCW_Right,LOW);
  }
int turn_right( int deg, int spd){
  enc_count_Left = 0;
  enc_count_Right = 0;
  analogWrite(pinSpeed_Left,spd+spd*speedDiff);  // set speed reference, duty-cycle = 50/255
  analogWrite(pinSpeed_Right,spd);
  digitalWrite(pinCC_Left,HIGH);            // go forward
  digitalWrite(pinCC_Right,HIGH);
  int conversion = 5.5*deg*PI/180*encperinch; //robot radius in inches*angle in radians/wheel circumf*encoder per rotation*conversion factor
  do {} while ((enc_count_Right < conversion) && (enc_count_Left < conversion));
  digitalWrite(pinCC_Left,LOW);            
  digitalWrite(pinCC_Right,LOW);
  }
  

void loop() {
  totaldistance = 0;
  do {
    enc_count_Left = 0; // reset encoder counter to 0
    enc_count_Right = 0;
  } while (digitalRead(pinON) == LOW);      // wait for ON switch
  digitalWrite(13,HIGH);                    // turn LED on
  delay(1000);
  forward(10*12,70);
  delay(500); 
  delay(1000);                              // wait 1 second
}
