#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD I2C address and the LCD dimensions (16x2)




const int joyXPin = 27;            // Joystick X-axis analog pin
const int joyYPin = 12;            // Joystick Y-axis analog pin
const int joyButtonPin = 14;       // Joystick button pin (SM)
const int buzzerPin = 17;         
unsigned long timerValue = 10000;  
unsigned long lastTimerUpdate = 0;
unsigned long currentTime = 0;          
unsigned long initialTimerValue = 10000;  
bool timerRunning = false;               
char verificationPIN[5] = "XXXX";     
int pinDigitIndex = 0;                    
bool confirmDigit = false;            
int beepInterval = 500;                   
bool timerBeeping = false;  
int beepCount;


enum State {
  WELCOME,
  LOADING,
  LOADING_COMPLETE,
  DONE,
  SELECT_TIMER,
  SET_TIMER,
  TIMER_RUNNING,
  VERIFY_PIN,
  AFTER_TIMER
};

State currentState = WELCOME;

void setup() {
  lcd.begin();
  lcd.backlight();                      
  pinMode(joyButtonPin, INPUT_PULLUP);  
  pinMode(buzzerPin, OUTPUT);
}

void playBeep(int frequency, int duration) {
  tone(buzzerPin, frequency, duration);
  delay(duration + 50);  
  noTone(buzzerPin);
}

void playBeepSequence(int frequency, int duration, int numBeeps) {
  for (int i = 0; i < numBeeps; ++i) {
    tone(buzzerPin, frequency, duration);
    delay(duration - 10); 
    noTone(buzzerPin);
    delay(100); 
  }
}




void formatTimer(unsigned long timeValue, char* timerStr) {
  unsigned long seconds = timeValue / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;
  seconds = seconds % 60;
  minutes = minutes % 60;
  sprintf(timerStr, "%02lu:%02lu:%02lu", hours, minutes, seconds);
}

void loop() {
  int joyXValue = analogRead(joyXPin);  // Read X-axis value
  int joyYValue = analogRead(joyYPin);  // Read Y-axis value
  currentTime = millis();               // Update the current time
  beepCount = map(timerValue, 0, 9000, 5, 10);  // Adjust as needed
  switch (currentState) {
    case WELCOME:
   
      lcd.setCursor(0, 0);
      lcd.print("     Welcome    ");
      lcd.setCursor(0, 1);
      lcd.print(" DH PRODUCTIONS ");
      playBeep(2000, 5000);
      delay(6000); 
      lcd.clear();  

      currentState = LOADING;
      break;

    case LOADING:
  
      lcd.setCursor(0, 0);
      lcd.print("     Loading    ");
      lcd.setCursor(0, 1);
      for (int i = 0; i < 16; i++) {
        lcd.print("#");
        delay(500);  
      }
      lcd.clear(); 
      currentState = LOADING_COMPLETE;
      break;

    case LOADING_COMPLETE:
     
     
      lcd.setCursor(0, 0);
      lcd.print("Loading Complete");
      delay(2000);  
      lcd.clear(); 

    
      currentState = VERIFY_PIN;
      pinDigitIndex = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
      lcd.setCursor(0, 1);
      lcd.print("                ");  
      break;

    case VERIFY_PIN:
     
      lcd.setCursor(0, 0);
      lcd.print(" Child  Lock :) ");


      lcd.setCursor(0, 1);
      lcd.print("Enter Pin:");

      
      lcd.print(verificationPIN);

      if (joyXValue == 4095) {
        if (!confirmDigit) {
          verificationPIN[pinDigitIndex]++;
          if (verificationPIN[pinDigitIndex] > '9') {
            verificationPIN[pinDigitIndex] = '0';
          }
        }
      } else if (joyXValue == 0) {
        if (!confirmDigit) {
          verificationPIN[pinDigitIndex]--;
          if (verificationPIN[pinDigitIndex] < '0') {
            verificationPIN[pinDigitIndex] = '9';
          }
        }
      }

   
      if (digitalRead(joyButtonPin) == LOW) {
        confirmDigit = true;
      } else {
       
        if (confirmDigit) {
          pinDigitIndex++;
          if (pinDigitIndex >= 4) {
           
            if (strcmp(verificationPIN, "4545") == 0) {
             
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Access Granted!");
              playBeep(2000, 100);  
              delay(100);         
              playBeep(2000, 100); 
              delay(100);          
              playBeep(2000, 100); 
              delay(1000);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("   Select Timer ");
              lcd.setCursor(0, 1);
              lcd.print("Default:");
              pinDigitIndex = 0;
              confirmDigit = false;
              currentState = SET_TIMER;

            } else {
            
            
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Access Denied!");
              playBeep(2000, 100);
              delay(100);
              playBeep(2000, 100);
              delay(2000);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("  Verification ");
              lcd.setCursor(0, 1);
              lcd.print("Enter Pin:");
              lcd.print("                "); 
              pinDigitIndex = 0;
              confirmDigit = false;
            }
          } else {
            // Move to the next digit
            lcd.setCursor(pinDigitIndex + 10, 1);
            confirmDigit = false;
          }
        }
      }
      break;

    case SET_TIMER:
    
      if (joyXValue == 4095) {
        timerValue += 1000;  
      } else if (joyXValue == 0) {
        if (timerValue >= 1000) {
          timerValue -= 1000;  
        }
      }

     
      lcd.setCursor(0, 1);
      lcd.print("Timer: ");
      char timerStrSetting[9];
      formatTimer(timerValue, timerStrSetting);
      lcd.print(timerStrSetting);

     
      if (digitalRead(joyButtonPin) == LOW) {
        lcd.clear();
        currentState = TIMER_RUNNING;
        lastTimerUpdate = currentTime; 
      }
      break;


    case TIMER_RUNNING:
      char timerStrRunning[9];
      formatTimer(timerValue, timerStrRunning);
      lcd.setCursor(0, 0);
      lcd.print(" Timer Running");
      lcd.setCursor(0, 1);
      lcd.print("Timer: ");
      lcd.print(timerStrRunning);
      lcd.print("   ");

     

      if (currentTime - lastTimerUpdate >= 1000) {
        lastTimerUpdate = currentTime;

        if (timerValue > 0) {
          timerValue -= 1000;

          int beepInterval = 1000;

          if (timerValue > 9000) {
            if (timerValue % beepInterval == 0) {
              playBeep(2000, 300);
            }
          } else {
            int beepDuration = 2000;
            int beepInterval = 100;
            int beepCount = (10000 - timerValue) / 500;

            playBeepSequence(2000, 100, beepCount);  
          }
        } else {
          currentState = AFTER_TIMER;
          lcd.clear();
          lcd.setCursor(0, 0);
        }
      }
      break;










    case AFTER_TIMER:
      lcd.setCursor(0, 0);
      lcd.print(" Have A Blast :)");
      delay(4000); 
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    Set Timer   ");
      currentState = SET_TIMER;
      break;
  }
}

