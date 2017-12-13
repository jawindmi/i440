// RFID reader ID-12 for Arduino 
// Based on code by BARRAGAN <http://people.interaction-ivrea.it/h.barragan> 
// and code from HC Gilje - http://hcgilje.wordpress.com/resources/rfid_id12_tagreader/
// Modified for Arduino by djmatic
// Modified for ID-12 and checksum by Martijn The - http://www.martijnthe.nl/
//
// Use the drawings from HC Gilje to wire up the ID-12.
// Remark: disconnect the rx serial wire to the ID-12 when uploading the sketch

// include the library code:
#include <LiquidCrystal.h>
#include <Servo.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);
Servo myservo;
#define GREEN 5
#define RED 6

void setup() {
  // connect to the serial port
  Serial.begin(9600);
  myservo.attach(3);
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, LOW);
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("I'm KiBo, the");
  lcd.setCursor(0,1);
  lcd.print("Kitchen Robot");
}

// 3 different RFID cards
// t1 is in robot
boolean t1 = true; // 65 = 101
boolean t2 = true; // 7D = 125
boolean t3 = true; // E9 = 233
String pancakes[] = {"1 & 1/2 Cup", " Flower","3 & 1/2 tsp"," Baking Powder","1 tsp", " Salt","1 Tbsp"," Sugar","1 & 1/4 Cup", " Milk","1 Egg"," ","3 Tbsp", " Butter"};
int pancakeSize = 7;
int pos = 0; 

void showRecipe(String ls[], int sz) {
  int pos;
  for (int i = 0; i < sz; i += 1){
    pos = i*2;
    delay(1000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(ls[pos]);
    lcd.setCursor(0,1);
    lcd.print(ls[pos + 1]);
  }
}
boolean toolsIn() {
  byte i = 0;
  byte val = 0;
  byte code[6];
  byte checksum = 0;
  byte bytesread = 0;
  byte tempbyte = 0;

  if(Serial.available() > 0) {
    if((val = Serial.read()) == 2) {                  // check for header 
      bytesread = 0; 
      while (bytesread < 12) {                        // read 10 digit code + 2 digit checksum
        if( Serial.available() > 0) { 
          val = Serial.read();
          if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) { // if header or stop bytes before the 10 digit reading 
            break;                                    // stop reading
          }

          // Do Ascii/Hex conversion:
          if ((val >= '0') && (val <= '9')) {
            val = val - '0';
          } else if ((val >= 'A') && (val <= 'F')) {
            val = 10 + val - 'A';
          }

          // Every two hex-digits, add byte to code:
          if (bytesread & 1 == 1) {
            // make some space for this hex-digit by
            // shifting the previous hex-digit with 4 bits to the left:
            code[bytesread >> 1] = (val | (tempbyte << 4));

            if (bytesread >> 1 != 5) {                // If we're at the checksum byte,
              checksum ^= code[bytesread >> 1];       // Calculate the checksum... (XOR)
            };
          } else {
            tempbyte = val;                           // Store the first hex digit first...
          };

          bytesread++;                                // ready to read next digit
        } 
      } 

      // Output to Serial:

      if (bytesread == 12) {
        int cur = code[5];

        if(cur == 101){
          t1 = !t1;
          lcd.clear();
          Serial.print("tool 1");
          Serial.println(t1 ? " -- in" : " -- out");
          lcd.setCursor(0,1);
          lcd.print(t1 ? "T1 is in" : "T1 is out");
//          Serial.println(t1);
        } else if(cur == 125){
          t2 = !t2;
          lcd.clear();
          Serial.print("tool 2");
          Serial.println(t2 ? " -- in" : " -- out");
          lcd.setCursor(0,1);
          lcd.print(t2 ? "T2 is in" : "T2 is out");
          Serial.println(t2);
        } else if(cur == 233){
          t3 = !t3;
          lcd.clear();
          Serial.print("tool 3");
          Serial.println(t3 ? " -- in" : " -- out");
          lcd.setCursor(0,1);
          lcd.print(t3 ? "T3 is in" : "T3 is out");
          Serial.println(t3);
        }
      }

      bytesread = 0;
    }
  }
  
}


void shakeHead(int shakeTimes) {
  int movement = 40;
  int origin = 90;
  int stepper = 1;
  for (int j = 0; j <= shakeTimes; j += 1) {
    
  //  left
    for (int i = origin; i <= origin + movement; i += stepper){
      myservo.write(i);
      delay(15);
    }
  //  right
    for (int i = origin + movement; i >= origin - movement; i -= stepper){
      myservo.write(i);
      delay(15);
    }
  //  center
    for (int i = origin - movement; i <= origin; i += stepper){
      myservo.write(i);
      delay(15);
    }
  }
}
boolean once = true;
boolean dirty = false;
boolean redOn = false;

void loop () {
//  always set head to propper position
  myservo.write(90);
  analogWrite(GREEN, 255);
  analogWrite(RED, 0);




//  check if at least one tool is taken
  toolsIn();
  int timeDelay = 100;
  int TD = 0;
  while(!(t1 && t2 && t3)){
    analogWrite(GREEN, 0);
    Serial.print("a tool is missing");
    if (once) {
      showRecipe(pancakes,pancakeSize);
      once = !once;
      delay(1000);
//      shakeHead(2);
    }
    delay(100);
//    time to clean up first
    if (TD >= timeDelay && !dirty){
      dirty = true;
      analogWrite(RED, 255);
      redOn = true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Time to");
      lcd.setCursor(0,1);
      lcd.print("clean up");
      shakeHead(2);
      TD = 1;

      //    time to clean up !first
    } else if (TD >= timeDelay && dirty){
      analogWrite(RED, 255);
      redOn = true;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Why haven't");
      lcd.setCursor(0,1);
      lcd.print("you cleaned?");
      shakeHead(2);
      TD = 1;
    } 

    if (dirty) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("You should");
      lcd.setCursor(0,1);
      lcd.print("clean now");
    } else {
      
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enjoy your");
      lcd.setCursor(0,1);
      lcd.print("meal!");
    }
    
    TD += 1;

    if (TD % 5 == 0 && dirty){
      if (redOn) {
        analogWrite(RED, 0);
      } else {
         analogWrite(RED, 255);
      }
      redOn = !redOn;
       
    }
    
    toolsIn();
    if (t1 && t2 && t3) {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("thank you for");
      lcd.setCursor(0,1);
      lcd.print("cleaning");
      delay(1000);
      lcd.setCursor(0,0);
      lcd.print("I'm KiBo, the");
      lcd.setCursor(0,1);
      lcd.print("Kitchen Robot");
    }
  }
  once = true;
  dirty = false;
  redOn = false;
  
//  showRecipe(pancakes,pancakeSize);       
  
//  if (once){
//    once = !once;
//    shakeHead(2);
//  }

}
