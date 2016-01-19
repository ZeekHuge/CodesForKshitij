
/**
*
*  The program do not reads or cares abour the z magnetic declination 
*  Pin connection is as :
*
*  Arduino :   Connection
*  A5          Magnet - SCL
*  A4          Magnet - SDA
*
**/


#include <Wire.h>
#include <IRremote.h>


#define NO                   -1
#define MAGNET                0
#define IR                    1
#define BOTH                  2
#define ONE                   1
#define TWO                   2


/* Prameters to be changed */

#define ROUND

#define SHOULD_ENABLE_MAGNET
#define SHOULD_ENABLE_IR

#define DEBUGGING


/* Make no changes in this */  
#define RECEIVING_PIN 
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

/* variables to store value data recieved from IR */
int irReceivedByte;
int headingAngle;
char endPoi = -1;


boolean reachedEnd;
/* variables to store value of x and y magnetic declination */
int x,y;

/* function to write a single 8bit register */

void writeSingleRegister (int reg, char value){

  Wire.beginTransmission(MAGNET_ADD);
  Wire.write(reg);  
  Wire.write(value);
  Wire.endTransmission();
}

/********************************************/


/* function to read a single 8bit register */

char readSingleRegister(int reg){

  Wire.beginTransmission(MAGNET_ADD);
  Wire.write(reg);  
  Wire.endTransmission();
  Wire.requestFrom(MAGNET_ADD,1);
  char_temp = Wire.read();
  Wire.endTransmission();

  return char_temp;  
}

/********************************************/


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

/********************************************/

/* check if the hex data is valid */
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

/********************************************/


/* function to read the output from the IR */

boolean readIRData(){

  detachExtInterrupt;
  stopMotor;

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
  }else {
  /* if receivedbyte is invalid  */
    return false;
  }
}

/********************************************/



void setup() {
  
  #if DEBUGGING
  Serial.begin(9600);
  #endif


  /* Enabling I2C and IR receiver */   
  Wire.begin();
  irReceiver.enableIRIn();
  pinMode(13,OUTPUT);


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
}

void loop(){


  #if DEBUGGING
   
   if (readIRData()){
     Serial.print (x,HEX);
     Serial.print (" ");
     Serial.println (y,HEX);
   }else{
     Serial.println("Data was not ready");
   }
  #else

   if (readIRData()){


   }
  #endif   
}