#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

// Buttons & Switches
const int BUTTON_RESET = 0; // RST button
const int BUTTON_CUT = 8;
const int BUTTON_RETURN = 9;
const int SWITCH_MANUAL_TOGGLE = 10;
bool cutButtonState = false;
bool returnButtonState = false;
bool manualToggleState = false;
// LEDs
const int LED_POWER = 3;
const int LED_ERROR = 4;
const int LED_CUTTING = 5;
const int LED_MANUAL_MODE = 6;
const int LED_RETURN = 7;
// Pots
const int SERVO_POS = A0;
const int POT1 = A1;
const int POT2 = A2;
int pot1Value = 0;
int pot2Value = 0;
// PWM
const int PWM_PIN = 11;
int i = 0;
// Servo
const int POT_LOWER_LIM = 0;
const int POT_UPPER_LIM = 560;
const int PWM_LOWER_LIM = 75;
const int PWM_UPPER_LIM = 110;
const int CUT_TICK_DELAY = 10;
const int SRV_LOWER_LIM = 200;
const int SRV_UPPER_LIM = 340;
const int ICE_TOP_FB = 310;
const int ICE_TOP_PWM = 103;
const int ICE_BOT_FB = 233;
const int ICE_BOT_PWM = 82;
int POS = 0; // PLACEHOLDER
int servoValue = 0; // PLACEHOLDER
// States
bool isManualMode = false;
bool isCutting = false;
bool isReturning = false;
bool isError = false;

void setup() {
  pinMode(BUTTON_CUT, INPUT_PULLUP);
  pinMode(BUTTON_RETURN, INPUT_PULLUP);
  pinMode(SWITCH_MANUAL_TOGGLE, INPUT_PULLUP);
  pinMode(LED_POWER, OUTPUT);
  pinMode(LED_CUTTING, OUTPUT);
  pinMode(LED_MANUAL_MODE, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);
  pinMode(LED_RETURN, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);

  Serial.begin(9600);

  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.display();
  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Hello, World!");
  display.display();
  digitalWrite(LED_POWER, HIGH);
  //digitalWrite(LED_RETURN, HIGH);
  //digitalWrite(LED_CUTTING, HIGH);
  TCCR2B = TCCR2B & B11111000 | B00000101; // for PWM frequency of 245.10 Hz
  //TCCR2B = TCCR2B & B11111000 | B00000110; // for PWM frequency of 122.55 Hz
}

void loop() {
  //delay(50);
  digitalWrite(LED_POWER, HIGH);
  //digitalWrite(LED_RETURN, HIGH);
  //digitalWrite(LED_CUTTING, HIGH);
  //digitalWrite(LED_MANUAL_MODE, HIGH);
  
  // Poll buttons and switches
  bool cutButtonState = digitalRead(BUTTON_CUT);
  bool returnButtonState = digitalRead(BUTTON_RETURN);
  bool manualToggleState = digitalRead(SWITCH_MANUAL_TOGGLE);

  // Poll potentiometers
  pot1Value = analogRead(POT1);
  pot2Value = pollPot2();
  servoValue = analogRead(SERVO_POS);
  POS = map(servoValue, 0, 255, 500, 2500);
  //printStatus();
  //delay(500);
  // Button cases
  //printStatus();
  if (isError) {
    digitalWrite(LED_ERROR, HIGH);
  } else {
    digitalWrite(LED_ERROR, LOW);
  }

  if (cutButtonState == LOW && !isManualMode) {
    isCutting = true;
    isReturning = false;
    digitalWrite(LED_CUTTING, HIGH);
    digitalWrite(LED_RETURN, LOW);
    //digitalWrite(LED_RETURN, LOW);
    delay(50); // Delay for debouncing;
  }

  if (returnButtonState == LOW && !isManualMode) { 
    isCutting = false;
    isReturning = true;
    digitalWrite(LED_RETURN, HIGH);
    digitalWrite(LED_CUTTING, LOW);
    delay(50); // Delay for debouncing
  }

  if (manualToggleState == LOW) {
    //isCutting = false;
    //isReturning = false;
    isManualMode = true;
    digitalWrite(LED_CUTTING, LOW);
    digitalWrite(LED_RETURN, LOW);
    digitalWrite(LED_MANUAL_MODE, HIGH);
    //delay(50); // Delay for debouncing
  } else {
    isManualMode = false;
    digitalWrite(LED_MANUAL_MODE, LOW);
    //delay(50); // Delay for debouncing
  }

  if (isManualMode) {
    analogWrite(PWM_PIN, map(pot2Value, POT_LOWER_LIM, POT_UPPER_LIM, PWM_LOWER_LIM, PWM_UPPER_LIM));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("POT Value: ");
    display.println(pot2Value);
    display.print("PWM Value: ");
    display.println(map(pot2Value, POT_LOWER_LIM, POT_UPPER_LIM, PWM_LOWER_LIM, PWM_UPPER_LIM));
    display.print("SVO Value: ");
    display.println(servoValue);
    
    display.display();
    //Serial.println(map(pot2Value, 60, 665, 31, 160));
    //analogWrite(PWM_PIN, 55);
    //Serial.print("Pot2 Value: ");
    //Serial.println(pot2Value);
    //Serial.print("Mapped: ");
    //Serial.println(map(pot2Value, 0, 665, 32, 155));
    //Serial.print("Servo: ");
    //Serial.println(servoValue);
    //Serial.println("==============");
  }
  // Actuation
  if (isCutting) {
    // Actuate cutter down
    if (servoValue < SRV_UPPER_LIM - 10) {
      isError = true;
      digitalWrite(LED_ERROR, HIGH);
      digitalWrite(LED_CUTTING, LOW);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Servo pos must exceed");
      display.print(SRV_UPPER_LIM);
      display.println(" to initiate cut.");
      display.println();
      display.print("Current SRV Pos: ");
      display.println(servoValue);
      display.println();
      display.println("Press return.");
      display.display();
      isCutting = false;
      digitalWrite(LED_CUTTING, LOW);
    } else {
      display.clearDisplay();
      unsigned long startMillis = millis();
      analogWrite(PWM_PIN, ICE_TOP_PWM);
      //delay(4000);
      int pos = ICE_TOP_PWM;
      while (pos > ICE_BOT_PWM +3) {
        display.clearDisplay();
        display.setCursor(0, 0);
        display.print("ICE_TOP_PWM: ");
        display.println(ICE_TOP_PWM);
        display.print("ICE_BOT_PWM: ");
        display.println(ICE_BOT_PWM);
        display.println(pos);
        display.display();
        delay(map(analogRead(POT1), 0, 560, 0, 100));
        analogWrite(PWM_PIN, pos);
        pos = pos - 1;
      }
      //display.clearDisplay();
      //delay(5000);
      analogWrite(PWM_PIN, PWM_LOWER_LIM);
      long hgf = analogRead(SERVO_POS);
      //unsigned long grand = 1000;
      while(abs(map(hgf, SRV_LOWER_LIM, SRV_UPPER_LIM, PWM_LOWER_LIM, PWM_UPPER_LIM) - ICE_BOT_PWM) > 5){
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(abs(map(analogRead(SERVO_POS), SRV_LOWER_LIM, SRV_UPPER_LIM, PWM_LOWER_LIM, PWM_UPPER_LIM) - ICE_BOT_PWM));
        display.println((millis()- startMillis));
        display.display();
        hgf = analogRead(SERVO_POS);
      }
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println((startMillis - millis())/1000);
      isCutting = false;
      digitalWrite(LED_CUTTING, LOW);
    }
    
    
    }
  
  if (isReturning) { 
    // Actuate cutter up
    isError = false;
    digitalWrite(LED_RETURN, HIGH);
    digitalWrite(LED_ERROR, LOW);
    analogWrite(PWM_PIN, PWM_UPPER_LIM);
    isReturning = false;
    digitalWrite(LED_RETURN, LOW);
  }
  //printStatus();

}

long pollPot2(){
  long thresh = 50;
  long val = 0;
  for (int i = 0; i < thresh; i++){
    val = val + analogRead(POT2);
    //delay(1);
  }
  //Serial.print("Value: ");
  //Serial.println(val);
  //Serial.print("Return: ");
  //Serial.println(val/thresh, 4);
  //delay(1000);
  return val/(thresh);
}

void printStatus() {
  Serial.println("=====================");
  Serial.print("Cutting: ");
  Serial.println(isCutting);
  Serial.print("Returning: ");
  Serial.println(isReturning);
  Serial.print("Manual: ");
  Serial.println(isManualMode);
  Serial.print("Error: ");
  Serial.println(isError);
  Serial.print("Speed: ");
  Serial.println(pot1Value);
  Serial.print("Position: ");
  Serial.println(pot2Value);
  Serial.println("=====================");
  delay(1000);
}