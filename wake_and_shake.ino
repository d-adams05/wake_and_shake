#include <LiquidCrystal.h>
#include "CountDown.h"
#include "Servo.h"


// LCD
#define RS_PIN 8
#define EN_PIN 9
#define D4_PIN 10
#define D5_PIN 11
#define D6_PIN 12
#define D7_PIN 13
LiquidCrystal lcd(RS_PIN, EN_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);
//////////////////////////////////////////////////////


// COUNTDOWN TIMER
CountDown timer;
int hour = 0, min = 0, sec = 0;
const int timeDelta = 1;  // changes time by 1 minute
bool timerActive = false;
//////////////////////////////////////////////////////


// USER INTERFACE BUTTONS
#define BUTTON1_PIN 2
#define BUTTON2_PIN 3
#define BUTTON3_PIN 4
#define BUTTON4_PIN 7
int b1PrevState = 0, b1State = 0;
int b2PrevState = 0, b2State = 0;
int b3PrevState = 0, b3State = 0;
int b4PrevState = 0, b4State = 0;
//////////////////////////////////////////////////////


// PHYSICAL MOVEMENT
// MIXER
#define MIXER_PIN A5
#define MIXER_DELAY_MS 5000
#define MIXER_DELAY_TICKS 1500 // note this is in clock ticks, not milliseconds
#define MIXER_FULL_POWER 1024
// MIXER SERVO
#define MIXER_SERVO_PIN 6
#define MIXER_SERVO_DELAY_UP 1800
#define MIXER_SERVO_DELAY_DOWN 800
#define MIXER_SERVO_LOWER_VALUE -100
#define MIXER_SERVO_RAISE_VALUE 100
Servo mixerServo;
//////////////////////////////////////////////////////


// ASSEMBLY
// PUMP
#define PUMP_CONTROL_PIN A4
#define PUMP_ON 1
#define PUMP_OFF 0
#define PUMP_DELAY_MS 6000
// POWDER SERVO
#define POWDER_SERVO_PIN 5
#define POWDER_SERVO_DELAY_MS 3000
#define POWDER_SERVO_MINI_DELAY_MS 500
#define POWDER_SERVO_WIDE_OPEN_ANGLE 120
#define POWDER_SERVO_HALF_OPEN_ANGLE 90
#define POWDER_SERVO_CLOSE_ANGLE 50
Servo powderServo;
//////////////////////////////////////////////////////


void setup() {
  // General initialization
  Serial.begin(9600);
  Serial.println("\nInitializing...");

  // User button setup
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  pinMode(BUTTON3_PIN, INPUT);
  pinMode(BUTTON4_PIN, INPUT);

  // LCD setup
  lcd.begin(16, 2);  // set up the LCD number of columns and rows
  lcd.print("Countdown Timer");

  // Countdown setup
  timer.setResolution(CountDown::SECONDS);

  // Mixer setup
  pinMode(MIXER_PIN, OUTPUT);
  digitalWrite(MIXER_PIN, LOW);  // ensure mixer is initially raised
  pinMode(MIXER_SERVO_PIN, OUTPUT);

  // Ingredient assembly setup
  pinMode(PUMP_CONTROL_PIN, OUTPUT);
  powderServo.attach(POWDER_SERVO_PIN);
  powderServo.write(POWDER_SERVO_CLOSE_ANGLE); // ensure servo is initially closed
  delay(POWDER_SERVO_MINI_DELAY_MS);
  powderServo.detach();

  Serial.println("Finished Initializing\n");
}


/***************************************************************************/


void loop() {
  // Read user interface buttons current states
  b1State = digitalRead(BUTTON1_PIN);
  b2State = digitalRead(BUTTON2_PIN);
  b3State = digitalRead(BUTTON3_PIN);
  b4State = digitalRead(BUTTON4_PIN);

  // Start mixing / start timer button
  if (b1State == 1 && b1PrevState == 0) {
    Serial.println("Button 1 pressed (start)");
    if (hour == 0 && min == 0 && sec == 0) {
      Serial.println("making now");
      startMaking();
    } else {
      startTimer();
    }
  }

  // Stop mixing / stop timer button
  if (b2State == 1 && b2PrevState == 0) {
    Serial.println("Button 2 pressed (stop)");
    stopTimer();
  }

  // If increment and decrement are pressed at the same time, reset timer
  if (b3State == 1 && b3PrevState == 0 && b4State == 1 && b4PrevState == 0) {
    Serial.println("RESETTING TIME");
    stopTimer();
    hour = 0;
    min = 0;
    sec = 0;

  // Otherwise do increment / decrement of timer
  } else {  
    // Increment timer
    if (b3State == 1 && b3PrevState == 0) {
      Serial.println("Button 3 pressed (increase)");
      increaseTime();
    }
    // Decrement timer
    if (b4State == 1 && b4PrevState == 0) {
      Serial.println("Button 4 pressed (decrease)");
      decreaseTime();
    }
  }

  // Set button variables for next iteration
  b1PrevState = b1State;
  b2PrevState = b2State;
  b3PrevState = b3State;
  b4PrevState = b4State;

  // LCD + Timer
  if (timer.isRunning()) {
    updateTimes();
    printCountdownToSerial();
  }
  printCountdownToLcd();

  // If timer is supposed to be active, but has no time left, start mixing
  if (timerActive && timer.remaining() == 0) {
    timerActive = false;
    printCountdownToLcd();
    updateTimes();
    startMaking();
  }

}


/***************************************************************************/


// Update the hour/min/sec variables based on current time left on timer
void updateTimes(void) {
  hour = timer.remaining() / 3600;
  min = timer.remaining() - (hour * 3600);
  min /= 60;
  sec = timer.remaining() - (hour * 3600);
  sec = sec - (min * 60);
}


// Print the time left on timer to Serial output
void printCountdownToSerial(void) {
  Serial.print(timer.remaining());
  Serial.print("       ");
  // hours
  if (hour < 10) {
    Serial.print("0");
    Serial.print(hour);
    Serial.print(":");
  } else {
    Serial.print(hour);
    Serial.print(":");
  }

  // minutes
  if (min < 10) {
    Serial.print("0");
    Serial.print(min);
    Serial.print(":");
  } else {
    Serial.print(min);
    Serial.print(":");
  }

  // seconds
  if (sec < 10) {
    Serial.print("0");
    Serial.println(sec);
  } else {
    Serial.println(sec);
  }
}


// Prints the current timer time onto the lcd
void printCountdownToLcd(void) {
  lcd.setCursor(0, 1);  // set the cursor to column 0, line 1 (line 1 is the second row, since counting begins with 0)
  // hours
  if (hour < 10) {
    lcd.print("0");
    lcd.print(hour);
    lcd.print(":");
  } else {
    lcd.print(hour);
    lcd.print(":");
  }

  // minutes
  if (min < 10) {
    lcd.print("0");
    lcd.print(min);
    lcd.print(":");
  } else {
    lcd.print(min);
    lcd.print(":");
  }

  // seconds
  if (sec < 10) {
    lcd.print("0");
    lcd.print(sec);
  } else {
    lcd.print(sec);
  }
}


// Starts countdown timer
void startTimer(void) {
  if (timer.isRunning()) return;
  timer.start(0, hour, min, sec);  // start(days, hours, minutes, seconds)
  timerActive = true;
}


// Stops countdown timer
void stopTimer(void) {
  if (!timer.isRunning()) return;
  timer.stop();
  timerActive = false;
}


// Resets countdown timer
void resetTimer(void) {
  stopTimer();
  hour = 0;
  min = 0;
  sec = 0;
  timerActive = false;
}


// Increases countdown timer time
void increaseTime(void) {
  // Increase time by preset amount
  min += timeDelta;

  // Max time of 24 hours
  if ((hour == 23 && min >= 60) || (hour == 24)) {
    hour = 24;
    min = 0;
    sec = 0;
    // If over 60 min, increase hour and set remaining minutes
  } else if (min >= 60) {
    hour += 1;
    min = min - 60;
  }

  // If the countdown timer was already running, stop it and rerun with new time
  if (timer.isRunning()) {
    timer.stop();
    startTimer();
  }
}


// Decreases countdown timer time
void decreaseTime(void) {
  if (min == 0 && hour == 0) return;
  // Decrease time by preset amount
  min -= timeDelta;

  if ((min < 0 && hour == 0) || (sec < 60 && min < 0 && hour == 0)) {
    stopTimer();
    hour = 0;
    min = 0;
    sec = 0;
    startMaking();
    return;
  }

  if (min < 0) {
    hour -= 1;
    Serial.println(min);
    min = 60 + min;
  }

  if (timer.isRunning()) {
    timer.stop();
    startTimer();
  }
}


// Pump water into the cup
void pumpWaterIntoCup(void) {
  Serial.println("pumpWaterIntoCup called");
  digitalWrite(PUMP_CONTROL_PIN, PUMP_ON);
  delay(PUMP_DELAY_MS);
  digitalWrite(PUMP_CONTROL_PIN, PUMP_OFF);
}


// Drop the protein powder into the cup
void pourPowderIntoCup(void) {
  Serial.println("powerPowderIntoCup called");
  powderServo.attach(POWDER_SERVO_PIN);
  powderServo.write(POWDER_SERVO_WIDE_OPEN_ANGLE);
  delay(POWDER_SERVO_MINI_DELAY_MS);
  powderServo.write(POWDER_SERVO_HALF_OPEN_ANGLE);
  delay(POWDER_SERVO_MINI_DELAY_MS);
  powderServo.write(POWDER_SERVO_CLOSE_ANGLE);
  delay(POWDER_SERVO_DELAY_MS);
  powderServo.detach();
}

// Lower the mixing whisk into the cup
void lowerWhisk(void) {
  Serial.println("lowerWhisk function called");
  mixerServo.attach(MIXER_SERVO_PIN);
  mixerServo.write(MIXER_SERVO_LOWER_VALUE);
  delay(MIXER_SERVO_DELAY_DOWN);
  mixerServo.detach();
}


// Raise the mixing whisk out of the cup
void raiseWhisk(void) {
  Serial.println("raiseWhisk function called");
  mixerServo.attach(MIXER_SERVO_PIN);
  mixerServo.write(MIXER_SERVO_RAISE_VALUE);
  delay(MIXER_SERVO_DELAY_UP);
  mixerServo.detach();
}


// Turns the whisk on to mix the shake
void mixProteinPowder(void) {
  Serial.println("mixProteinPowder function called");

  // This will PWM the whisk for 9 seconds
  for(int j = 0; j < MIXER_DELAY_TICKS; j++) {
    for(int i = 0; i < MIXER_FULL_POWER; i++) {
      if(i < MIXER_FULL_POWER/3){
        digitalWrite(MIXER_PIN, 1);
      } else {
        digitalWrite(MIXER_PIN, 0);
      }
    }
    delay(1);
  }

}


// The entire machine process of making the protein shake
void startMaking(void) {
  Serial.println("startMaking function called");
  lcd.noDisplay();
  pourPowderIntoCup();
  pumpWaterIntoCup();
  delay(1000); // Slight pause to settle
  lowerWhisk();
  mixProteinPowder();
  delay(1000); // Slight pause to settle
  raiseWhisk();
  lcd.display();
  Serial.println("startMaking ending\n");
}