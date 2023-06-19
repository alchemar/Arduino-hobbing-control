#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int gearArrayCount = 8;
int gearArrayIndex = 0;


int gearTeeth[] = {12,16,18,24,36,48,72,144};
int gearSteps[] = {12,9,8,6,4,3,2,1};
int teeth = gearTeeth[0];
int steps = gearSteps[0];
bool running = false;

int menuSelect = 0;
int menu = 0;

unsigned long messageMillis = millis();
unsigned long keypadMillis = messageMillis;
unsigned long accelMicros = micros();
unsigned long stepMicros = accelMicros;
unsigned int stepTime = 500;
unsigned int accelTime = 1000;


float rpm = 0;
float maxRPM = 800;
float currentRPM = 0;
float accel = 2000;
unsigned int pulseDelay = 100;
int mtrBstepCount = 0;

int debounceDelay = 50;

int keypad = 0;
int keypadState = 0;
int keypadFlicker = 0;

int buttonCount = 0;

void setMenu();
void setSelect();
void keypadMenuFunction();
void updateSteps();
void readKeypad();

#define select  1
#define cancel  2
#define up  8
#define down  4
#define menuHome 0
#define menuGear 1
#define menuStart 2

#define mtrAstep  4
#define mtrAdir   5
#define mtrBstep  6
#define mtrBdir   7


void setup()
{
  lcd.init();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();
  setMenu();
  setSelect();
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
  pinMode(14,INPUT_PULLUP);
  pinMode(15,INPUT_PULLUP);
  pinMode(16,INPUT_PULLUP);
  pinMode(17,INPUT_PULLUP);
  pinMode(mtrAstep,OUTPUT);
  pinMode(mtrAdir,OUTPUT);
  pinMode(mtrBstep,OUTPUT);
  pinMode(mtrBdir,OUTPUT);


  //Serial.begin(9600);
}


void loop()
{
  if((millis() - messageMillis)>1000){
    messageMillis = millis();
   
  }
  readKeypad();
  updateSteps();

 
}

void setMenu(){
 if(menu == menuHome){
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("No Teeth");
  lcd.setCursor(15,0);
  lcd.print(teeth);
  lcd.setCursor(3,1);
  lcd.print("Start/Stop");
  
  lcd.setCursor(3,3);
  if(running){
  lcd.print("RUNNING!!!");
  } else {
    lcd.print("STOPPED!!!");
  }
} else if(menu == menuGear){
  //set gear
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("No Teeth");
  lcd.setCursor(15,0);
  lcd.print(teeth);
  lcd.setCursor(3,1);
  lcd.print("Steps");
  lcd.setCursor(15,1);
  lcd.print(steps);
  lcd.setCursor(3,2);
  lcd.print("+/- to set gear");
}  else if(menu == menuStart){
  //set gear
  lcd.clear();
  lcd.setCursor(3,0);
  lcd.print("Start");
  lcd.setCursor(3,1);
  lcd.print("Stop");
  lcd.setCursor(3,3);
  if(running){
  lcd.print("RUNNING!!!");
  } else {
    lcd.print("STOPPED!!!");
  }
  
}


}

void setSelect(){
    int line = menuSelect;

    lcd.setCursor(1,0);
    if(line == 0){
      lcd.print("*");
    } else {
      lcd.print(" ");
    }
    lcd.setCursor(1,1);
    if(line == 1){
      lcd.print("*");
    } else {
      lcd.print(" ");
    }
    lcd.setCursor(1,2);
    if(line == 2){setSelect();
      lcd.print("*");
    } else {
      lcd.print(" ");
    }
}

void keypadMenuFunction(){
  if(menu == menuHome){
    if(keypad == up){
      menuSelect = 0;
      setSelect();
    } else if(keypad == down){
      menuSelect = 1;
      setSelect();
    } else if((keypad == select) and menuSelect == 0){
      menu = menuGear;
      menuSelect = 0;
      setMenu();
      setSelect();
    } else if((keypad == select) and menuSelect == 1){
      menu = menuStart;
      menuSelect = 0;
      setMenu();
      setSelect();
    }
  } else if(menu == menuGear){
  if(keypad == cancel){
    menu = 0;
    menuSelect = 0;
    setMenu();
    setSelect();
  } else if(!running and (keypad == up) and (gearArrayIndex < (gearArrayCount - 1))){
    gearArrayIndex++;
    teeth = gearTeeth[gearArrayIndex];
    steps = gearSteps[gearArrayIndex];

    setMenu();
  } else if(!running and (keypad == down) and (gearArrayIndex > 0)){
    gearArrayIndex--;
    teeth = gearTeeth[gearArrayIndex];
    steps = gearSteps[gearArrayIndex];

    setMenu();
  }

 } else if(menu == menuStart){
  if(keypad == up){
    menuSelect = 0;
    setSelect();
  } else if(keypad == down){
    menuSelect = 1;
    setSelect();
  } else if(keypad == cancel){
    menu = menuHome;
    menuSelect = 0;
    setMenu();
    setSelect();
  } else if((keypad == select) and (menuSelect == 0)){
    running = true;
    rpm = maxRPM;
    setMenu();
  } else if((keypad == select) and (menuSelect == 1)){
    running = false;
    rpm = 0;
    setMenu();
  }
 }

}

void updateSteps(){
 int currentMicros = micros();
 if(((currentMicros - stepMicros)>stepTime) and (currentRPM > 1)) {
   stepMicros = currentMicros;
  //step on
  mtrBstepCount++;
  if(mtrBstepCount >= (steps-1)){
    mtrBstepCount = 0;
    digitalWrite(mtrBstep,HIGH);
  }
  digitalWrite(mtrAstep,HIGH);
 } else if ((currentMicros - stepMicros) > pulseDelay){
  //step off
  digitalWrite(mtrBstep,LOW);
  digitalWrite(mtrAstep,LOW);
 }

 if((currentMicros - accelMicros)>accelTime){
  accelMicros = currentMicros;
  if((currentRPM - rpm)>1){
    //decel
    currentRPM = currentRPM-accel*float(accelTime)/(1000.0*1000.0*60.0);
    stepTime = 300000.0/currentRPM;


  } else if ((rpm - currentRPM)>1){
    //accel
    currentRPM = currentRPM+accel*float(accelTime)/(1000.0*1000.0*60.0);
    stepTime = 300000.0/currentRPM;

    
  } else if(currentRPM <= 1){
    currentRPM = 0;
  }
 }

}
void readKeypad(){
 //read keypad
  keypad = !digitalRead(17)<< 3;
  keypad = keypad + (!digitalRead(16) << 2);
  keypad = keypad + (!digitalRead(15) << 1);
  keypad = keypad + !digitalRead(14);

  //debounce keypad
  if(keypad != keypadFlicker){
    keypadMillis = millis();
    keypadFlicker = keypad;
  }

  if((millis()-keypadMillis)>debounceDelay){
    //Serial.println(keypad);
    if(keypad != keypadState){
      keypadState = keypad;
      
      if(keypad){
       
        keypadMenuFunction();
      }
    }
  }
  }