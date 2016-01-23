
/**
*
* This is the program to test working of Magnetometer.
* This would seem to be a copy of the file masterCode.c 
* but that is so as to use the same functions with only change in loop()
*
* After loading the program onto the Arduino
* It will start LED_13 and will wait for the start button to be pressed.
* After the start button is pressed it waits for a second and then orients itself
* to NORTH direction
* it then wiats for another 5 second there
* and then orients itself to EAST direction
* wiats for another 5 seconds
* then orients itself to WEST and then similarly to south direction
* Check the orientation with help of the compass (in your smartphone probably)
* if orientations are right, the magnetometer is correctly conmfigured.
*
*/


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
//#include <IRremote.h>


/*********************************/
/*********************************/

//#define MAGNET_ALINGMENT 


/*********************************/
/*********************************/


#define NO                   -1
#define MAGNET                0
#define IR                    1
#define BOTH                  2
#define ONE                   1
#define TWO                   2
#define DEGREES_TO_RADIANS_CONVERSION_FACTOR 0.017

/* Prameters to be changed */

//#define ROUND

//#define SHOULD_ENABLE_MAGNET
//#define SHOULD_ENABLE_IR

//#define DEBUGGING 


/* Make no changes in this */  

#define SWITCH_PIN 11

#define LeftMotorPin_F 8
#define LeftMotorPin_B 9

#define RightMotorPin_F 7
#define RightMotorPin_B 6


// #define RECEIVING_PIN 
#define MAGNET_ADD            0x1E


// #if DEBUGGING == NO  
//   #define ENABLE_IR SHOULD_ENABLE_IR 
//   #define ENABLE_MAGNET SHOULD_ENABLE_MAGNET
// #endif

// #if DEBUGGING == MAGNET
  
//   #define ENABLE_MAGNET 1
// #endif

// #if DEBUGGING == IR
 
//   #define ENABLE_IR 1
// #endif
  
// #if DEBUGGING == BOTH
 
//   #define ENABLE_MAGNET 1
//   #define ENABLE_IR 1
// #endif



/* variables for IR receiving */
// IRrecv irReceiver(RECEIVING_PIN);
// decode_results irResults;

/* temprory variable */
char char_temp;

/* variables to store value data recieved from IR */
// int irReceivedByte;
int headingAngle;
char endPoi = -1;
double headingValue;

boolean reachedEnd;
volatile boolean flag_readingData=false ;

/* variables to store value of x and y magnetic declination */
int x,y;




/* functions to steer bot*/

void stop(){

  digitalWrite(LeftMotorPin_F,0);
  digitalWrite(LeftMotorPin_B,0);

  digitalWrite(RightMotorPin_F,0);
  digitalWrite(RightMotorPin_B,0);  
}

void turnLeft(){

  digitalWrite(LeftMotorPin_F,0);
  digitalWrite(LeftMotorPin_B,0);

  digitalWrite(RightMotorPin_F,1);
  digitalWrite(RightMotorPin_B,0);
}


void turnRightAtZero(){

  digitalWrite(LeftMotorPin_F,1);
  digitalWrite(LeftMotorPin_B,0);

  digitalWrite(RightMotorPin_F,0);
  digitalWrite(RightMotorPin_B,1);
}

void turnLeftAtZero(){
  
  digitalWrite(LeftMotorPin_F,0);
  digitalWrite(LeftMotorPin_B,1);

  digitalWrite(RightMotorPin_F,1);
  digitalWrite(RightMotorPin_B,0); 
}


void turnRight(){

  digitalWrite(LeftMotorPin_F,1);
  digitalWrite(LeftMotorPin_B,0);

  digitalWrite(RightMotorPin_F,0);
  digitalWrite(RightMotorPin_B,1);
}


void moveStraight(){

  digitalWrite(LeftMotorPin_F,1);
  digitalWrite(LeftMotorPin_B,0);

  digitalWrite(RightMotorPin_F,1);
  digitalWrite(RightMotorPin_B,0); 
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

// boolean isHexValid (){
  
//   irReceivedByte =  ((irResults.value & 0xf0000000) >> 5*4 );
//   irReceivedByte |= ((irResults.value & 0x0f000000) >> 3*4 );
//   irReceivedByte |= ((irResults.value & 0x0000f000) >> 3*4 );
//   irReceivedByte |= ((irResults.value & 0x00000f00) >> 1*4 ); 

//   if (((int)(irReceivedByte + ((int)((irResults.value & 0x00f00000) >> 3*4 ) |
//                                     ((irResults.value & 0x000f0000) >> 1*4 ) |
//                                     ((irResults.value & 0x000000f0) >> 1*4 ) |
//                                     ((irResults.value & 0x0000000f) << 1*4 )))))
//   {
//     return false;
//   }
//   return true;
// }

/*****************************************************************/

/* function to read the next IR message */

// boolean readNextIRdata(){

//   char i;
//   for (i=5; (!irReceiver.decode(&irResults)) && i; i-- ) {
//     delay(10);
//   }
//   if (i!=0){
//     irReceiver.resume(); // Receive the next value

//     if (isHexValid()){
//       return true;
//     }
//   }
//   return false;
// }

// /*****************************************************************/



// /* function to read the output from the IR */

// boolean readIRData(){

  

//   boolean flag_didnt_receive_any = true;
//   boolean flag_message_started = false;

//   if (readNextIRdata()){  /* -------------------------------------signal 400 start message*/
    
//     #if ROUND == ONE
//       if (irReceivedByte == 400){  /*-----------------------------if is start message */
//         if (readNextIRdata()){      /*----------------------------signal message_tag*/
//           if (irReceivedByte == 11 || irReceivedByte == 22){  /*--if is message_tag at a non start poi
//                                                                   or message 2 of start_poi */

//             if (readNextIRdata()){    /*--------------------------signal poi id */
//               if (irReceivedByte == endPoi){ /*-------------------if is endPoi */
//                 reachedEnd =true; /*------------------------------flag that it has reached end*/
//                 return true;/*------------------------------------end this function*/
//               }
//               else{   /*------------------------------------------if is not endPoi*/
//                 if(readNextIRdata()){ --------------------------signal next poi id
//                   if(readNextIRdata()){/*-------------------------signal next poi heading*/
//                     headingAngle = irReceivedByte;/*--------------save heading angle */
//                     if (readNextIRdata()){ /*---------------------signal next poi cost*/
//                       if (readNextIRdata()){/*--------------------signal stop byte */
//                         if(irReceivedByte == 500){ /*-------------if is stop byte*/
//                           return true;
//                         }
//                       }
//                     }
//                   }
//                 }
//               }
//             }
//           }else if(irReceivedByte == 12){ /*-------------if message_tag at start, message 1 */
//             if (readNextIRdata()){ /*--------------------signal poi_id, must be 0 as is start_poi*/
//               if(readNextIRdata()){/*--------------------signal end poi_id */
//                 endPoi = irReceivedByte; /*--------------save end poi_id*/
//                 if (readNextIRdata()) /*-----------------signal stop message */
//                 {
//                   if (irReceivedByte == 500) /*-----------if is stop message */
//                   {
//                     return true;
//                   }
//                 }
//               }
//             }
//           }
//         }
//       }
//     #endif

//     #if ROUND == TWO


//     #endif
//   }else {
//   /* if receivedbyte is invalid  */
//     return false;
//   }
// }

/*****************************************************************/

/* Function to change readingData flag using interrupt */

// void changeReadingDataFlag(){
//   flag_readingData=true;
//   detachInterrupt(RECEIVING_PIN);
// }

/*****************************************************************/


/* Function to change the orientation of the bot according to the angle provided */

void changeOrientetion (){

  if (headingAngle >= 0 && headingAngle < 90 ){

    /* These headingAngle would mean somewhere between N and E */    
    
    /* Aling bot somewhere in between N and W */
    
    readMagneticData();
    while(x < 0 || y > 0 ){
      turnRightAtZero();
      readMagneticData();
    }
    stop();


    /* Aling according to the headingValue */
    headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
    readMagneticData();
     while(y/x != headingValue ){
       turnLeftAtZero();
     }
    stop();
  }else if (headingAngle >= 90 && headingAngle < 180 ){
  
  /* These headingAngle would mean somewhere between E and S */    
    
  /* Aling bot somewhere in between E and N */

    readMagneticData();
    while(x < 0 || y < 0 ){
      turnRightAtZero();
      readMagneticData();
    }
    stop();

    /* Aling according to the headingValue */
    
    if (headingAngle =! 90){
      headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
      readMagneticData();
      while(y/x != headingValue ){
       turnLeftAtZero();
      }
      stop();
    }else{
      readMagneticData();
      while( x != 0 ){
        turnRightAtZero();
        readMagneticData();
      }
      stop();      
    }

  }else if (headingAngle >= 180 && headingAngle < 270 ){

    /* These headingAngle would mean somewhere between S and W */    
    
    /* Aling bot somewhere in between S and E */
    
    readMagneticData();
    while(x > 0 || y < 0 ){
      turnRightAtZero();
      readMagneticData();
    }
    stop();


    /* Aling according to the headingValue */
    headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
    readMagneticData();
     while(y/x != headingValue ){
       turnLeftAtZero();
     }
    stop();
  }else if (headingAngle >= 270 && headingAngle < 360 ){
  
  /* These headingAngle would mean somewhere between W and N */    
    
  /* Aling bot somewhere in between W and S */

    readMagneticData();
    while(x > 0 || y > 0 ){
      turnRightAtZero();
      readMagneticData();
    }
    stop();

    /* Aling according to the headingValue */

    if (headingAngle =! 270){
      headingValue = tan(headingAngle * DEGREES_TO_RADIANS_CONVERSION_FACTOR);
      readMagneticData();
      while(y/x != headingValue ){
       turnLeftAtZero();
      }
      stop();
    }else{
      readMagneticData();
      while( y != 0 ){
        turnRightAtZero();
        readMagneticData();
      }
      stop();      
    }

  }


}


void setup() {
  
 // #if DEBUGGING
  Serial.begin(9600);
 // #endif


  /* Enabling I2C and IR receiver */   
  Wire.begin();
//  irReceiver.enableIRIn();

  /* Setting GPIO modes */
  pinMode(13,OUTPUT);
  
  DDRB &= ~(1<<3);
  PORTB = (1<<3);

  pinMode(LeftMotorPin_B,OUTPUT);
  pinMode(LeftMotorPin_F,OUTPUT);
  pinMode(RightMotorPin_B,OUTPUT);
  pinMode(RightMotorPin_F,OUTPUT);

  /* Check the connection */  
 // #if DEBUGGING
    Serial.print("ConnectedTo(Should be H43) - ");
    Serial.print(readSingleRegister(0x0A));
    Serial.print(readSingleRegister(0x0B));
    Serial.println(readSingleRegister(0x0C));
  //#endif


  /* Setting the value of the A register (0x00)=0x28 */
  writeSingleRegister(0x00,0x28);


  /* Read A register (0x00). Correct value should be 0x28 */
  //#if DEBUGGING
    Serial.print("ARegisterValue(shouldBe 0x28): ");
    Serial.println(readSingleRegister(0x00),HEX);
  //#endif


  /* Read B register (0x01) .Correct value should be 0x20 */
  //#if DEBUGGING
    Serial.print("BRegisterValue(shouldBe 0x20): ");
    Serial.println(readSingleRegister(0x01),HEX);
  //#endif


  /* Setting the value of the Mode register (0x02)=0x00 */
  writeSingleRegister(0x02,0x00);


  /* Read Mode register (0x02) .Correct value should be 0x00 */
  //#if DEBUGGING
    Serial.print("ModeRegisterValue(shouldBe 0x00): ");
    Serial.println(readSingleRegister(0x02),HEX);
  //#endif


  //#if DEBUGGING
    Serial.println("The vaules printed will be as x y");
  //#endif

  /*to show that the bot is ready*/
  digitalWrite(13,1);
  Serial.println("Tell the above output to Zubeen");
  while(digitalRead(SWITCH_PIN) != LOW);
  delay(1000);
  // attachInterrupt(RECEIVING_PIN,changeReadingDataFlag,LOW);
}



void loop(){

  // if(flag_readingData) {
  //   /* if reading the data, then stop the bot */
  //   stop();

  //   if (readIRData()){

  //     /* if the data has been CORRECTLY read */
  //     /* convert the heading angle to the the magnetic declinations */


  //   }else{

  //     /* if the data has been INcorrectly read */
  //     /* move the bot a bit forward */

  //     moveStraight();
  //     delay(500);
  //     stop();
      
      
  //     ** Imagine the case in which the bot gets IR signal but since it is still far from
  //     ** the poi, it is not getting correct signals. So the solution to this thing 
  //     ** is that the bot should move ahead a bit more and then again start the procedure
  //     ** The above three lines cause this same thing to happen
  //     ** but remember, try the code in the file moveStraightDelayCheck in this same folder
  //     ** and see what delay seems to make it move a bit ahead.
  //     ** and replace that delay value with 500 in the above delay.
      
  //   }

  //   /* after reading data attach interrupt back to the reciving pin and clear the 
  //   ** reading data flag */
  //   flag_readingData = false;
  //   attachInterrupt(RECEIVING_PIN,changeReadingDataFlag,LOW);  
  // }else if (!reachedEnd){

    
  // }
  




  // #if DEBUGGING
   readMagneticData();
   // if (readIRData()){
     Serial.print (x);
     Serial.print (" ");
     Serial.println (y);
     delay(200);
     // if(x>0){
     //  if(y<5 && y>-5){
     //    Serial.println("+x axis");
     //  }
     // }if(y>0){
     //  if(x<5 && x>-5){
     //    Serial.println("+y axis");
     //  }
     // }

      // headingAngle=0;
      // changeOrientetion();
      // delay(5000);
      // headingAngle=90;
      // changeOrientetion();
      // delay(5000);
      // headingAngle=180;
      // changeOrientetion();
      // headingAngle=270;
      // changeOrientetion();
      // delay(5000);


   // }else{
     // Serial.println("Data was not ready");
   // }
  // #else

   // if (readIRData()){


   // }
  // #endif   
}
