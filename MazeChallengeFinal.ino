#include <Keypad.h>
#include <LiquidCrystal.h>
#include <MPU6050_tockn.h>
#include <Wire.h>
#include <stdlib.h>
#include <math.h>

MPU6050 imu(Wire);
#define I2C_SLAVE_ADDR 0x04 // 4 in hexadecimal
#define PERFORM_CALIBRATION //Comment to disable startup calibration
           //Change to the name of any supported IMU! 

long timer = 0;
LiquidCrystal lcd(15, 2, 27, 26, 25, 33);
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
 {'1','2','3'},
 {'4','5','6'},
 {'7','8','9'},
 {'*','0','#'}
};
byte rowPins[ROWS] = {16, 17, 5, 18};
byte colPins[COLS] = {19, 4, 23};
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
int inputArray[50]; // maximum number of digits for a number is 20, change if needed
int arrayIndex = 0;


void steer(int angle) {
  int left, right;
  int maxSpeed = 150; // Set max speed to 150 (or whatever value is appropriate for your use case)
  angle = constrain(angle, 0,90 );
  float angleRad = angle * M_PI / 180.0; // Convert angle to radians
  float sinVal = sin(angleRad);

  if (angle > 45) {
    left = maxSpeed;
    right = maxSpeed * sinVal;
  } else if(angle < 0){
    left = 1.5*maxSpeed * sinVal;
    right = maxSpeed;
  }

  transmit(left, right, angle);
}

void rotate(int destinationAngle, bool align = false) {
  imu.update();
  float origin = imu.getAngleZ(),
        angle = 0;

  steer(45 + destinationAngle);
  
  while (angle > -abs(destinationAngle) && angle < abs(destinationAngle)) {
    imu.update();
    angle = origin - imu.getAngleZ();
    if (align) {
      steer(45+ destinationAngle - angle);
    }
    
  }

  transmit(0,0,45);
}
 
void setup()
{
 imu.calcGyroOffsets(true, 0, 0);
 Serial.begin(115200);
 Wire.begin();

 // set up the LCD's number of columns and rows:
 lcd.begin(16, 2);
 lcd.setCursor(0, 0);
  lcd.print("2->Fwds    6->R");
  lcd.setCursor(0, 1);
  lcd.print("8->Back    4->L");
}
 
void loop()
{
 char key = keypad.getKey();
 if (key)
 {
    if (key >= '0' && key <= '9') 
    {     // only act on numeric keys
      inputArray[arrayIndex++] = key - '0'; // convert char to int and add to array
      if (arrayIndex >= 20) { // if array is full, execute the movements and reset
        executeMovements(inputArray, arrayIndex);
        arrayIndex = 0;
      }
    } 
    else if (key == '#') 
    {
      if (arrayIndex > 0) 
      {
        executeMovements(inputArray, arrayIndex);
        arrayIndex = 0;               // clear input
      }
    } 
    else if (key == '*') 
    {
        arrayIndex = 0;                 // clear input
    }
   lcd.clear();
for (int i = 0; i < arrayIndex; i++) {
  lcd.setCursor(i % 16, i / 16); // set cursor position based on the current index

  lcd.print(inputArray[i]);

  if (i % 16 == 15) { // if we've printed 16 characters, move to next row
    lcd.setCursor(0, i / 16 + 1);
  }
}


  }
}
 
void executeMovements(int *inputArray, int arraySize)
{

  for(int i=0;i<arraySize;i++)
  {
   
   if(inputArray[i] ==2)
    {
     
      transmit(150,150,43);
      delay(568);
    }  
    else if(inputArray[i]==4)
    {
      
    rotate(-135);
    }
    else if(inputArray[i]==6)
    {
      rotate(180);
      
    
    }
    else if(inputArray[i]==8)
    {
      transmit(150,150,90);
      delay(1300);
    }
    else if(inputArray[i]==3)
    {
      transmit(255,203,43);
      delay(1227);
    }
    else if(inputArray[i]==7)
    {
      lcd.print("I DID IT");
      
    }
     else if(inputArray[i]==5)
    {
      transmit(0,0,43);
      delay(100);
    }
    else if(inputArray[i]==9)
    {
      transmit(255,203,43);
      delay(3150);
    }
     else if(inputArray[i]==1)
    {
      transmit(255,203,43);
      delay(100);
    }
  }
    transmit(0,0,45);
}

void transmit(int left,int right,int angle)
{
   Serial.println("angle");
      Serial.println(angle);
  Wire.beginTransmission(I2C_SLAVE_ADDR); // transmit to device #4
  Wire.write((byte)((left & 0x0000FF00) >> 8));    // first byte of x, containing bits 16 to 9
  Wire.write((byte)(left & 0x000000FF));           // second byte of x, containing the 8 LSB - bits 8 to 1
  Wire.write((byte)((right & 0x0000FF00) >> 8));   
  Wire.write((byte)(right & 0x000000FF));          
  Wire.write((byte)((angle & 0x0000FF00) >> 8));    
  Wire.write((byte)(angle & 0x000000FF));
  Wire.endTransmission();   // stop transmitting
}
