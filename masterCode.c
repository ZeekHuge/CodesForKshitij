
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


/* Prameters to be changed */

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
int int_temp;


/* variables to store value data recieved from IR */
int irReceivedByte;

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

/********************************************/



/* function to stop IR data reception */

boolean stopReadingIRData(){

  if (irReceiver.decode(&irResults)) {
    //irReceiver.resume(); // commented out to stop the data reception
  }
}

/* function to read the output from the IR */

boolean readIRData(){

  detachExtInterrupt;
  stopMotor;

  char flag = 0;

  if (irReceiver.decode(&irResults)) {
    irReceiver.resume(); // Receive the next value
  }
   

  if (irReceiver.decode(&irResults)) {
    
  /* check data validity */
  irReceivedByte =  ((irResults.value & 0xf0000000) >> 5*4 );
  irReceivedByte |= ((irResults.value & 0x0f000000) >> 3*4 );
  irReceivedByte |= ((irResults.value & 0x0000f000) >> 3*4 );
  irReceivedByte |= ((irResults.value & 0x00000f00) >> 1*4 );

  int_temp =  ((irResults.value & 0x00f00000) >> 3*4 );
  int_temp |= ((irResults.value & 0x000f0000) >> 1*4 );
  int_temp |= ((irResults.value & 0x000000f0) >> 1*4 );
  int_temp |= ((irResults.value & 0x0000000f) << 1*4 );

  /* check valid  */

  if (((int)(irReceivedByte + int_temp)) == 0){
    /* if valid  */

  }
  
  irReceiver.resume(); // Receive the next value

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