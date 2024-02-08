#include <Keypad.h>
#include <SevSegShift.h>
#include <Servo.h>

const byte ROWS = 4; 
const byte COLS = 4; 
const int SHIFT_PIN_SHCP = 4;
const int SHIFT_PIN_STCP = 3;
const int SHIFT_PIN_DS   = 2;
const int buzzer = 10; //buzzer to arduino pin 10

SevSegShift sevseg(SHIFT_PIN_DS, SHIFT_PIN_SHCP, SHIFT_PIN_STCP, 1, true);
Servo myservo;

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {A0, A1, A2, A3}; 
byte colPins[COLS] = {13, 12, 11, }; 

Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

String passCode;
String codePass;
long numCode;
String passWord = "123456";
int countDown = 20;
bool exitCode = false;
bool nextBool = true;

unsigned long previousMillis = 0;
unsigned long previousTimeMillis = 0;
long intervalOpen = countDown*1000;
const long interval = 1000;

void setup(){
  Serial.begin(9600);
  pinMode(buzzer, OUTPUT);
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  byte numDigits = 4;
  byte digitPins[] = {8, 7, 6, 5}; // These are the PINS of the ** Arduino **
  byte segmentPins[] = {0, 1, 2, 3, 4, 5, 6, 7}; // these are the PINs of the ** Shift register **
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = false; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected
 
  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments, updateWithDelays, leadingZeros, disableDecPoint);
  sevseg.setBrightness(100);

  myservo.write(90);
}
  
void loop(){
  keypadCode();
  displayPin();

}

void keypadCode() {
  char customKey = customKeypad.getKey();
  
  if (customKey){
    if (customKey != '*' && customKey != '#') {
      passCode += (char)customKey;
      Serial.println(passCode);
    }
  }

  if (passCode.length() >= 6) {
    codePass = passCode;
    passCode = "";
    verifyCode();
  }
}

void displayPin() {
  numCode = passCode.toInt();
  numCode %= 10000;
  sevseg.setNumber(numCode);
  sevseg.refreshDisplay(); // Must run repeatedly; don't use blocking code (ex: delay()) in the loop() function or this won't work right
}

void verifyCode() {
  //Serial.println(passCode);
  Serial.println(passWord);
  if (codePass == passWord) {
    Serial.println("SUCCESS");
    correctCode();
  }
  else {
    wrongCode();
    Serial.println("WRONG");
  }
}

void correctCode() {
  countDown = 20;
  exitCode = false;
  while(!exitCode) {
    unsigned long currentMillis = millis();
    if (nextBool) {
      previousMillis = currentMillis;
      previousTimeMillis = currentMillis;
      nextBool = false;
    }
    //Serial.println(currentMillis);
    myservo.write(0);
    sevseg.setNumber(countDown);
    sevseg.refreshDisplay();
    if (currentMillis - previousMillis >= intervalOpen) {
      myservo.write(90);
      //Serial.println("INSIDE2");
      previousMillis = currentMillis;
      nextBool = true;
      exitCode = !exitCode;
    }
    if (currentMillis - previousTimeMillis >= interval) {
      //Serial.println("INSIDE3");
      countDown--;  
      sevseg.setNumber(countDown);
      sevseg.refreshDisplay();   
      previousTimeMillis = currentMillis;   
    }

    char numKey = customKeypad.getKey();
    if (numKey){
      if (numKey == '#') {
        countDown += 10;
        intervalOpen += 10000;
      }
    }
  }
  codePass = "";
}

void wrongCode() {
  tone(buzzer, 500); // Send 1KHz sound signal...
  delay(300);        // ...for 1 sec
  tone(buzzer, 200); // Send 1KHz sound signal...
  delay(200); 
  noTone(buzzer);     // Stop sound...
  codePass = "";
}