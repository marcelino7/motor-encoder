// ============================================================
//  main.ino
//  Controls: 1 = Brake position, 2 = Home position
//  On power-on: automatically moves to Brake position
// ============================================================

#include <Arduino.h>
#include <lx16a-servo.h>
#include "servo_config.h"     // <-- all positions & speeds here

LX16ABus servoBus;
LX16AServo servo(&servoBus, 1);

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  servoBus.beginOnePinMode(&Serial1, 1);
  servoBus.debug(false);
  servoBus.retry = 0;

  Serial.println("=== Servo Controller Ready ===");
  Serial.println("1 = Brake position");
  Serial.println("2 = Home position");

  
}

void loop() {
  Serial.println("Startup: moving to BRAKE...");
  servo.move_time(STARTUP_POS, STARTUP_SPEED);
  delay(STARTUP_DELAY);


  if (!Serial.available()) return;


  char key = Serial.read();

  if (key == '1') {
    Serial.println(">> BRAKE");
    servo.move_time(POS_BRAKE, SPEED_TO_BRAKE);
    delay(DELAY_AFTER_BRAKE);
  }
  else if (key == '2') {
    Serial.println(">> HOME");
    servo.move_time(POS_HOME, SPEED_TO_HOME);
    delay(DELAY_AFTER_HOME);
  }
}
