

/**
*
* This is the program to test working of Motors.
* This would seem to be a copy of the file masterCode.c 
* but that is so as to use the same functions with only change in loop()
*
* After loading the program onto the Arduino
* It will start LED_13 and will wait for the start button to be pressed.
* After the start button is pressed it waits for a second and then 
* The bot will move straight ahead for 3 seconds
* The bot will turn right for 3 seconds and this will be repeated onn !
*
**/

/**
*
--  The program do not reads or cares about the z magnetic declination as its not required.
--  The program assumes the x axis of the magentometer is the forward direction of the bot.
*
*  Pin connection is as :
*
*  Arduino :   Connection
*  A5          Magnet - SCL
*  A4          Magnet - SDA
*
**/


#include <Wire.h>
#include <IRremote.h>


/*********************************/
/*********************************/

#define MAGNET_ALINGMENT 1


/*********************************/
/*********************************/

#define TEMP_RATE 1


#define NO                   -1
#define MAGNET                0
#define IR                    1
#define BOTH                  2
#define ONE                   1
#define TWO                   2
#define DEGREES_TO_RADIANS_CONVERSION_FACTOR 0.017

/* Prameters to be changed */

#define ROUND ONE

#define SHOULD_ENABLE_MAGNET 1
#define SHOULD_ENABLE_IR 1

#define DEBUGGING 0


/* Make no changes in this */  

#define DIRECTION_ZERO_LEFT   0
#define DIRECTION_ZERO_RIGHT  1
#define DIRECTION_STRAIGHT    2
#define DIRECTION_RIGHT       3
#define DIRECTION_LEFT        4

#define SWITCH_PIN 11

#define LeftMotorPin_F 8
#define LeftMotorPin_B 9

#define RightMotorPin_F 7
#define RightMotorPin_B 6


#define RECEIVING_PIN         3
#define MAGNET_ADD            0x1E


#if DEBUGGING == NO  
  #define ENABLE_IR SHOULD_ENABLE_IR 
  #define ENABLE_MAGNET SHOULD_ENABLE_MAGNET
#endif

#if DEBUGGING == MAGNET
  
  #define ENABLE_MAGNET 1
#endif

#if DEBUGGING == IR
 
  #define ENABLE_IR 1
#endif
  
#if DEBUGGING == BOTH
 
  #define ENABLE_MAGNET 1
  #define ENABLE_IR 1
#endif



/* variables for IR receiving */
IRrecv irReceiver(RECEIVING_PIN);
decode_results irResults;

/* temprory variable */
char char_temp;
volatile char movingDirection;

/* variables to store value data recieved from IR */
int irReceivedByte;
int headingAngle;
char endPoi = -1;
double headingValue;

boolean reachedEnd;
volatile boolean flag_readingData=false ;

/* variables to store value of x and y magnetic declination */
int x,y;




/* functions to steer bot*/

void stop(){

  TCCR2 &= ~(1<<0);  
  TCNT2 = 0x0000;

  PORTB &= ~(1<<0);
  PORTB &= ~(1<<1);

  PORTD &= ~(1<<7);
  PORTD &= ~(1<<6);

  // digitalWrite(LeftMotorPin_F,0);
  // digitalWrite(LeftMotorPin_B,0);

  // digitalWrite(RightMotorPin_F,0);
  // digitalWrite(RightMotorPin_B,0);  
}

void turnLeft(unsigned int rate){

  movingDirection = DIRECTION_LEFT;
  
  TCCR2 &= ~(1<<0);  
  TCNT2 = 0x0000;
  OCR2 = rate;

  PORTB |= (1<<0);
  PORTB &= ~(1<<1);

  PORTD |= (1<<7);
  PORTD &= ~(1<<6);

  TCCR2 |= (1<<0);

  // digitalWrite(LeftMotorPin_F,0);
  // digitalWrite(LeftMotorPin_B,0);

  // digitalWrite(RightMotorPin_F,1);
  // digitalWrite(RightMotorPin_B,0);
}


void turnRightAtZero(unsigned int rate){

  movingDirection = DIRECTION_ZERO_RIGHT;

  TCCR2 &= ~(1<<0);  
  TCNT2 = 0x0000;
  OCR2 = rate;

  PORTB |= (1<<0);
  PORTB &= ~(1<<1);

  PORTD |= (1<<6);
  PORTD &= ~(1<<7);

  TCCR2 |= (1<<0);

  // digitalWrite(LeftMotorPin_F,1);
  // digitalWrite(LeftMotorPin_B,0);

  // digitalWrite(RightMotorPin_F,0);
  // digitalWrite(RightMotorPin_B,1);
}

void turnLeftAtZero(unsigned int rate){
  
  movingDirection = DIRECTION_ZERO_LEFT;

  TCCR2 &= ~(1<<0);  
  TCNT2 = 0x0000;
  OCR2 = rate;

  PORTB |= (1<<1);
  PORTB &= ~(1<<0);

  PORTD |= (1<<7);
  PORTD &= ~(1<<6);

  TCCR2 |= (1<<0);

  // digitalWrite(LeftMotorPin_F,0);
  // digitalWrite(LeftMotorPin_B,1);

  // digitalWrite(RightMotorPin_F,1);
  // digitalWrite(RightMotorPin_B,0); 
}


void turnRight(unsigned int rate){

  movingDirection = DIRECTION_RIGHT;

  TCCR2 &= ~(1<<0);  
  TCNT2 = 0x0000;
  OCR2 = rate;

  PORTB |= (1<<0);
  PORTB &= ~(1<<1);

  PORTD |= (1<<7);
  PORTD &= ~(1<<6);

  TCCR2 |= (1<<0);

  // digitalWrite(LeftMotorPin_F,1);
  // digitalWrite(LeftMotorPin_B,0);

  // digitalWrite(RightMotorPin_F,0);
  // digitalWrite(RightMotorPin_B,1);
}


void moveStraight(unsigned int rate){

  movingDirection = DIRECTION_STRAIGHT;
  
  TCCR2 &= ~(1<<0);  
  TCNT2 = 0x0000;


  PORTB |= (1<<0);
  PORTB &= ~(1<<1);

  PORTD |= (1<<7);
  PORTD &= ~(1<<6);

}

/*****************************************************************/


/* function to write a single 8bit register from the magnetometer */

void writeSingleRegister (int reg, char value){

  Wire.beginTransmission(MAGNET_ADD);
  Wire.write(reg);  
  Wire.write(value);
  Wire.endTransmission();
}

/*****************************************************************/


/* function to read a single 8bit register from the magnetometer */

char readSingleRegister(int reg){

  Wire.beginTransmission(MAGNET_ADD);
  Wire.write(reg);  
  Wire.endTransmission();
  Wire.requestFrom(MAGNET_ADD,1);
  char_temp = Wire.read();
  Wire.endTransmission();

  return char_temp;  
}

/*****************************************************************/


/* function to read x and y magnetic declinations */

boolean readMagneticData(){

  /* Reading x value */
  Wire.beginTransmission(MAGNET_ADD);
  Wire.write(0x03);  
  Wire.endTransmission();
  Wire.requestFrom(MAGNET_ADD,2);
  x = Wire.read();
  x = x << 8;
  x = (x | Wire.read());
  Wire.endTransmission();

  /* converting from two's complement" */
  x =~x;
  x+=1;
  
  
  /* Reading z value */
  Wire.beginTransmission(MAGNET_ADD);
  Wire.write(0x05);  
  Wire.endTransmission();
  Wire.requestFrom(MAGNET_ADD,2);
  Wire.read();
  Wire.read();
  Wire.endTransmission();
  
  
  /* Reading y value */
  Wire.beginTransmission(MAGNET_ADD);
  Wire.write(0x07);  
  Wire.endTransmission();
  Wire.requestFrom(MAGNET_ADD,2);
  y = Wire.read();
  y = y << 8;
  y = (y | Wire.read());
  Wire.endTransmission();

  /* converting from two's complement" */
  y =~y;
  y+=1;

  return true;
}

/*****************************************************************/

/* check if the hex data (ie IR Received data) is valid */

boolean isHexValid (){
  
  irReceivedByte =  ((irResults.value & 0xf0000000) >> 5*4 );
  irReceivedByte |= ((irResults.value & 0x0f000000) >> 3*4 );
  irReceivedByte |= ((irResults.value & 0x0000f000) >> 3*4 );
  irReceivedByte |= ((irResults.value & 0x00000f00) >> 1*4 ); 

  if (((int)(irReceivedByte + ((int)((irResults.value & 0x00f00000) >> 3*4 ) |
                                    ((irResults.value & 0x000f0000) >> 1*4 ) |
                                    ((irResults.value & 0x000000f0) >> 1*4 ) |
                                    ((irResults.value & 0x0000000f) << 1*4 )))))
  {
    return false;
  }
  return true;
}

/*****************************************************************/

/* function to read the next IR message */

boolean readNextIRdata(){

  char i;
  for (i=5; (!irReceiver.decode(&irResults)) && i; i-- ) {
    delay(10);
  }
  if (i!=0){
    irReceiver.resume(); // Receive the next value

    if (isHexValid()){
      return true;
    }
  }
  return false;
}

/*****************************************************************/



/* function to read the output from the IR */

boolean readIRData(){

  

  boolean flag_didnt_receive_any = true;
  boolean flag_message_started = false;

  if (readNextIRdata()){  /* -------------------------------------signal 400 start message*/
    
    #if ROUND == ONE
      if (irReceivedByte == 400){  /*-----------------------------if is start message */
        if (readNextIRdata()){      /*----------------------------signal message_tag*/
          if (irReceivedByte == 11 || irReceivedByte == 22){  /*--if is message_tag at a non start poi
                                                                  or message 2 of start_poi */

            if (readNextIRdata()){    /*--------------------------signal poi id */
              if (irReceivedByte == endPoi){ /*-------------------if is endPoi */
                reachedEnd =true; /*------------------------------flag that it has reached end*/
                return true;/*------------------------------------end this function*/
              }
              else{   /*------------------------------------------if is not endPoi*/
                if(readNextIRdata()){ /*--------------------------signal next poi id*/
                  if(readNextIRdata()){/*-------------------------signal next poi heading*/
                    headingAngle = irReceivedByte;/*--------------save heading angle */
                    if (readNextIRdata()){ /*---------------------signal next poi cost*/
                      if (readNextIRdata()){/*--------------------signal stop byte */
                        if(irReceivedByte == 500){ /*-------------if is stop byte*/
                          return true;
                        }
                      }
                    }
                  }
                }
              }
            }
          }else if(irReceivedByte == 12){ /*-------------if message_tag at start, message 1 */
            if (readNextIRdata()){ /*--------------------signal poi_id, must be 0 as is start_poi*/
              if(readNextIRdata()){/*--------------------signal end poi_id */
                endPoi = irReceivedByte; /*--------------save end poi_id*/
                if (readNextIRdata()) /*-----------------signal stop message */
                {
                  if (irReceivedByte == 500) /*-----------if is stop message */
                  {
                    return true;
                  }
                }
              }
            }
          }
        }
      }
    #endif

    #if ROUND == TWO


    #endif
  }else {
  /* if receivedbyte is invalid  */
    return false;
  }
}

/*****************************************************************/

/* Function to change readingData flag using interrupt */

void changeReadingDataFlag(){
  flag_readingData=true;
  detachInterrupt(RECEIVING_PIN);
}

/*****************************************************************/


/* Function to change the orientation of the bot according to the angle provided */

void changeOrientetion (){

  if (headingAngle >= 0 && headingAngle < 90 ){

    /* These headingAngle would mean somewhere between N and E */    
    
    /* Aling bot somewhere in between N and W */
    
    readMagneticData();
    while(x < 0 || y > 0 ){
      turnRightAtZero(TEMP_RATE);
      readMagneticData();
    }
    stop();


    /* Aling according to the headingValue */
    headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
    readMagneticData();
    while(y/x != headingValue ){
      turnLeftAtZero(TEMP_RATE);
    }
    stop();
  }else if (headingAngle >= 90 && headingAngle < 180 ){
  
  /* These headingAngle would mean somewhere between E and S */    
    
  /* Aling bot somewhere in between E and N */

    readMagneticData();
    while(x < 0 || y < 0 ){
      turnRightAtZero(TEMP_RATE);
      readMagneticData();
    }
    stop();

    /* Aling according to the headingValue */
    
    if (headingAngle =! 90){
      headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
      readMagneticData();
      while(y/x != headingValue ){
       turnLeftAtZero(TEMP_RATE);
      }
      stop();
    }else{
      readMagneticData();
      while( x != 0 ){
        turnRightAtZero(TEMP_RATE);
        readMagneticData();
      }
      stop();      
    }

  }else if (headingAngle >= 180 && headingAngle < 270 ){

    /* These headingAngle would mean somewhere between S and W */    
    
    /* Aling bot somewhere in between S and E */
    
    readMagneticData();
    while(x > 0 || y < 0 ){
      turnRightAtZero(TEMP_RATE);
      readMagneticData();
    }
    stop();


    /* Aling according to the headingValue */
    headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
    readMagneticData();
    while(y/x != headingValue ){
      turnLeftAtZero(TEMP_RATE);
    }
    stop();
  }else if (headingAngle >= 270 && headingAngle < 360 ){
  
  /* These headingAngle would mean somewhere between W and N */    
    
  /* Aling bot somewhere in between W and S */

    readMagneticData();
    while(x > 0 || y > 0 ){
      turnRightAtZero(TEMP_RATE);
      readMagneticData();
    }
    stop();

    /* Aling according to the headingValue */

    if (headingAngle =! 270){
      headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
      readMagneticData();
      while(y/x != headingValue ){
       turnLeftAtZero(TEMP_RATE);
      }
      stop();
    }else{
      readMagneticData();
      while( y != 0 ){
        turnRightAtZero(TEMP_RATE);
        readMagneticData();
      }
      stop();      
    }

  }


}


void setup() {
  
  TCCR2 = (1<<6);
  TIMSK = (1<<7);

  #if DEBUGGING
  Serial.begin(9600);
  #endif


  /* Enabling I2C and IR receiver */   
  Wire.begin();
  irReceiver.enableIRIn();

  /* Setting GPIO modes */
  pinMode(13,OUTPUT);
  pinMode(SWITCH_PIN,INPUT);
  pinMode(LeftMotorPin_B,OUTPUT);
  pinMode(LeftMotorPin_F,OUTPUT);
  pinMode(RightMotorPin_B,OUTPUT);
  pinMode(RightMotorPin_F,OUTPUT);

  /* Check the connection */  
  #if DEBUGGING
    Serial.print("ConnectedTo(Should be H43) - ");
    Serial.print(readSingleRegister(0x0A));
    Serial.print(readSingleRegister(0x0B));
    Serial.println(readSingleRegister(0x0C));
  #endif


  /* Setting the value of the A register (0x00)=0x28 */
  writeSingleRegister(0x00,0x28);


  /* Read A register (0x00). Correct value should be 0x28 */
  #if DEBUGGING
    Serial.print("ARegisterValue(shouldBe 0x28): ");
    Serial.println(readSingleRegister(0x00),HEX);
  #endif


  /* Read B register (0x01) .Correct value should be 0x20 */
  #if DEBUGGING
    Serial.print("BRegisterValue(shouldBe 0x20): ");
    Serial.println(readSingleRegister(0x01),HEX);
  #endif


  /* Setting the value of the Mode register (0x02)=0x00 */
  writeSingleRegister(0x02,0x00);


  /* Read Mode register (0x02) .Correct value should be 0x00 */
  #if DEBUGGING
    Serial.print("ModeRegisterValue(shouldBe 0x00): ");
    Serial.println(readSingleRegister(0x02),HEX);
  #endif


  #if DEBUGGING
    Serial.println("The vaules printed will be as x y");
  #endif

  /*to show that the bot is ready*/
  digitalWrite(13,1);

  while(digitalRead(SWITCH_PIN) != HIGH);
  attachInterrupt(RECEIVING_PIN,changeReadingDataFlag,LOW);
}



void loop(){

  digitalWrite(LeftMotorPin_F,1);
  digitalWrite(LeftMotorPin_B,0);

  digitalWrite(RightMotorPin_F,1);
  digitalWrite(RightMotorPin_B,0);
  delay(3000);

  digitalWrite(LeftMotorPin_F,1);
  digitalWrite(LeftMotorPin_B,0);

  digitalWrite(RightMotorPin_F,0);
  digitalWrite(RightMotorPin_B,1);
  delay(3000);
}



ISR(TIMER2_COMP_vect){

  switch(movingDirection){
    case DIRECTION_STRAIGHT:
      PORTB ^= (1<<0);
      PORTD ^= (1<<7);
    break;

    case DIRECTION_RIGHT:
      PORTD ^= (1<<7);
    break;

    case DIRECTION_LEFT:
      PORTB ^= (1<<0);
    break;

    case DIRECTION_ZERO_LEFT:
      PORTB ^= (1<<1);
      PORTD ^= (1<<7);
    break;

    case DIRECTION_ZERO_RIGHT:
      PORTB ^= (1<<0);
      PORTD ^= (1<<6);
    break;
  }

}

