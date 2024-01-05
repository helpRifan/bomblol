#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD I2C address and the LCD dimensions (16x2)




const int joyXPin = 27;            // Joystick X-axis analog pin
const int joyYPin = 12;            // Joystick Y-axis analog pin
const int joyButtonPin = 14;       // Joystick button pin (SM)
const int buzzerPin = 17;          // Replace 8 with the actual pin you used for the buzzer.
unsigned long timerValue = 10000;  // Default timer value: 10 seconds
unsigned long lastTimerUpdate = 0;
unsigned long currentTime = 0;            // Declare currentTime outside the switch statement
unsigned long initialTimerValue = 10000;  // Initial timer value in milliseconds
bool timerRunning = false;                // Flag to indicate if the timer  is running
char verificationPIN[5] = "XXXX";         // 4-digit verification PIN
int pinDigitIndex = 0;                    // Index of the currently entered PIN digit
bool confirmDigit = false;                // Flag to confirm the current digit
int beepInterval = 500;                   // Initial beep interval in milliseconds
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
  lcd.backlight();                      // Turn on the backlight
  pinMode(joyButtonPin, INPUT_PULLUP);  // Set the joystick button pin as an input with a pull-up resistor
  pinMode(buzzerPin, OUTPUT);
}

void playBeep(int frequency, int duration) {
  tone(buzzerPin, frequency, duration);
  delay(duration + 50);  // Add a small delay for better separation of beeps
  noTone(buzzerPin);
}

void playBeepSequence(int frequency, int duration, int numBeeps) {
  for (int i = 0; i < numBeeps; ++i) {
    tone(buzzerPin, frequency, duration);
    delay(duration - 10);  // Adjust as needed
    noTone(buzzerPin);
    delay(100);  // Reduce the delay between beeps
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
      // Display the welcome screen
      lcd.setCursor(0, 0);
      lcd.print("     Welcome    ");
      lcd.setCursor(0, 1);
      lcd.print(" DH PRODUCTIONS ");
      playBeep(2000, 5000);
      delay(6000);  // Display for 6 seconds
      lcd.clear();  // Clear the LCD screen

      currentState = LOADING;
      break;

    case LOADING:
      // Display the loading animation
      lcd.setCursor(0, 0);
      lcd.print("     Loading    ");
      lcd.setCursor(0, 1);
      for (int i = 0; i < 16; i++) {
        lcd.print("#");
        delay(500);  // Add a delay to control the animation speed
      }
      lcd.clear();  // Clear the LCD screen
      currentState = LOADING_COMPLETE;
      break;

    case LOADING_COMPLETE:
      // Additional state after loading is complete (you can add specific logic here)
      // For example, you could display some information or instructions.
      lcd.setCursor(0, 0);
      lcd.print("Loading Complete");
      delay(2000);  // Display for 2 seconds
      lcd.clear();  // Clear the LCD screen

      // Transition to VERIFY_PIN for entering the PIN
      currentState = VERIFY_PIN;
      pinDigitIndex = 0;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter PIN:");
      lcd.setCursor(0, 1);
      lcd.print("                ");  // Clear previous PIN display
      break;

    case VERIFY_PIN:
      // Display "Verification" on top
      lcd.setCursor(0, 0);
      lcd.print(" Child  Lock :) ");

      // Display "Enter Pin:" on the bottom
      lcd.setCursor(0, 1);
      lcd.print("Enter Pin:");

      // Display the PIN input with leading zeros
      lcd.print(verificationPIN);

      // Check the joystick X-axis for digit selection (0 to 9)
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

      // Check the joystick button (SM) for confirmation and move to the next digit
      if (digitalRead(joyButtonPin) == LOW) {
        confirmDigit = true;
      } else {
        // The button is released
        if (confirmDigit) {
          pinDigitIndex++;
          if (pinDigitIndex >= 4) {
            // All 4 digits entered, verify the PIN
            if (strcmp(verificationPIN, "4545") == 0) {
              // PIN is correct, display "Access Granted" and move to SELECT_TIMER
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Access Granted!");
              playBeep(2000, 100);  // Play the first beep
              delay(100);           // Wait for a short interval (e.g., 200 milliseconds)
              playBeep(2000, 100);  // Play the second beep
              delay(100);           // Wait for a short interval
              playBeep(2000, 100);  // Play the third beep
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
              // PIN is incorrect, show "Access Denied" and return to VERIFY_PIN
              // PIN is incorrect, show "Access Denied" and return to VERIFY_PIN
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Access Denied!");
              playBeep(2000, 100);
              delay(100);
              playBeep(2000, 100);  // Play 2 quick beeps
              delay(2000);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("  Verification ");
              lcd.setCursor(0, 1);
              lcd.print("Enter Pin:");
              lcd.print("                ");  // Clear previous PIN display
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
      // Adjust timer based on joystick position
      if (joyXValue == 4095) {
        timerValue += 1000;  // Increase timer by 1 second
      } else if (joyXValue == 0) {
        if (timerValue >= 1000) {
          timerValue -= 1000;  // Decrease timer by 1 second (minimum 1 second)
        }
      }

      // Display the timer value for setting
      lcd.setCursor(0, 1);
      lcd.print("Timer: ");
      char timerStrSetting[9];
      formatTimer(timerValue, timerStrSetting);
      lcd.print(timerStrSetting);

      // Check the joystick button for confirmation and transition to TIMER_RUNNING
      if (digitalRead(joyButtonPin) == LOW) {
        lcd.clear();
        currentState = TIMER_RUNNING;
        lastTimerUpdate = currentTime;  // Reset the timer update timestamp
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

            playBeepSequence(2000, 100, beepCount);  // Play the beep sequence
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
      delay(4000);  // Display for 2 seconds
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    Set Timer   ");
      currentState = SET_TIMER;
      break;
  }
}

