// ============================================================
//  main.ino — Spring-return brake with AS5600 PID position hold
//
//  SYSTEM BEHAVIOUR:
//  - Power on  → motor drives to 30°, PID holds against spring
//  - 'D' command or power off → motor stops, spring returns to 0°
//
//  WIRING:
//  AS5600 SDA → A4,  SCL → A5,  VCC → 3.3V,  GND → GND
//  Motor driver pins as defined in config.h
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

// ── System state ──────────────────────────────────────────────
enum State { DISENGAGED, ENGAGING, HOLDING };
State systemState = DISENGAGED;

// ── PID state ─────────────────────────────────────────────────
float pidIntegral   = 0.0f;
float pidLastError  = 0.0f;
unsigned long lastPidTime = 0;

// ============================================================
//  AS5600 — Read angle in degrees (0.0 to 360.0)
// ============================================================
float readAngleDeg() {
  Wire.beginTransmission(AS5600_ADDR);
  Wire.write(0x0C);               // RAW ANGLE high byte register
  Wire.endTransmission(false);    // Repeated start (keeps bus active)
  Wire.requestFrom(AS5600_ADDR, 2);
  uint16_t raw = ((uint16_t)Wire.read() << 8) | Wire.read();
  return raw * (360.0f / AS5600_RAW_MAX);
}

// ============================================================
//  Motor driver helpers
//  
// ============================================================
void motorCCW(int pwm) {
  // CCW = pushes against the spring toward 30 degrees

  analogWrite(PIN_SPEED_PWM, pwm);
  digitalWrite(PIN_CW,  LOW);
  digitalWrite(PIN_CCW, HIGH);
}

void motorCW(int pwm) {
  // CW = same direction as spring (toward 0 degrees)
  // PID uses this if it overshoots past 30 degrees
  analogWrite(PIN_SPEED_PWM, pwm);
  digitalWrite(PIN_CW,  HIGH);
  digitalWrite(PIN_CCW, LOW);
}

void motorStop() {
  analogWrite(PIN_SPEED_PWM, 0);
  digitalWrite(PIN_CW,  LOW);
  digitalWrite(PIN_CCW, LOW);
}

// ============================================================
//  PID controller — runs every PID_INTERVAL_MS
//
//  Why all three terms matter for a spring-load:
//
//  P alone: motor pushes toward 30° but spring creates a constant
//           opposing torque. P balances at some steady-state error
//           (e.g. holds at 27° instead of 30°). Never reaches target.
//
//  I fixes this: integral accumulates the steady-state error over
//           time and adds more and more output until the position
//           is exactly correct. Essential for spring loads.
//
//  D damps: when the arm approaches 30° fast, D sees a rapidly
//           changing error and brakes the output early, preventing
//           overshoot and oscillation around the target.
// ============================================================
void runPID(float currentAngle) {
  float error = TARGET_ANGLE - currentAngle;

  // Deadband — within this range, stop fighting the spring
  // Reduces heat, vibration, and motor wear at steady state
  if (abs(error) < DEADBAND) {
    motorStop();
    return;
  }

  // ── Time delta ──────────────────────────────────────────────
  unsigned long now = millis();
  float dt = (now - lastPidTime) / 1000.0f;  // convert ms to seconds
  lastPidTime = now;
  // Safety: if dt is zero or suspiciously large, use nominal interval
  if (dt <= 0.0f || dt > 0.5f) dt = PID_INTERVAL_MS / 1000.0f;

  // ── Proportional ────────────────────────────────────────────
  float pTerm = KP * error;

  // ── Integral ────────────────────────────────────────────────
  pidIntegral += error * dt;
  // Clamp integral to prevent windup (if stalled, integral
  // would grow forever and cause a violent lurch when freed)
  pidIntegral = constrain(pidIntegral, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);
  float iTerm = KI * pidIntegral;

  // ── Derivative ──────────────────────────────────────────────
  float dTerm = KD * ((error - pidLastError) / dt);
  pidLastError = error;

  // ── Sum and apply ───────────────────────────────────────────
  float output = pTerm + iTerm + dTerm;

  if (output > 0) {
    // Error is positive: we are below 30°, push against spring
    int pwm = constrain((int)abs(output), PWM_MIN, PWM_MAX);
    motorCCW(pwm);
  } else {
    // Error is negative: we overshot past 30°, gently reverse
    // In practice this rarely fires because spring helps push back
    int pwm = constrain((int)abs(output), PWM_MIN, PWM_MAX);
    motorCW(pwm);
  }
}

// ============================================================
//  Reset PID state (must call before every fresh engagement)
//  Without this, stale integral from previous run causes a
//  lurch on the next engage command
// ============================================================
void resetPID() {
  pidIntegral  = 0.0f;
  pidLastError = 0.0f;
  lastPidTime  = millis();
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(PIN_CW,        OUTPUT);
  pinMode(PIN_CCW,       OUTPUT);
  pinMode(PIN_STOP_MODE, OUTPUT);
  pinMode(PIN_SPD_SET,   OUTPUT);
  pinMode(PIN_ALM_RST,   OUTPUT);
  pinMode(PIN_SPEED_PWM, OUTPUT);

  // Driver enable signals
  digitalWrite(PIN_STOP_MODE, HIGH);
  digitalWrite(PIN_SPD_SET,   LOW);
  digitalWrite(PIN_ALM_RST,   HIGH);

  motorStop();
  delay(200);  // Let AS5600 finish booting

  Serial.println("=== Spring-Return Brake Controller ===");
  Serial.println("  E = Engage   (drive to 30deg + PID hold)");
  Serial.println("  D = Disengage(motor off, spring returns)");
  Serial.println("  T = Print current angle");
  Serial.println();

  // Auto-engage on power-on
  Serial.println("Power-on: engaging brake to 30deg...");
  systemState = ENGAGING;
  resetPID();
}

// ============================================================
//  LOOP
// ============================================================
void loop() {

  // ── Serial commands ───────────────────────────────────────
  if (Serial.available()) {
    char cmd = toupper(Serial.read());
    while (Serial.available()) Serial.read(); // flush remainder

    if (cmd == 'E') {
      Serial.println(">> ENGAGE");
      systemState = ENGAGING;
      resetPID();
    }
    else if (cmd == 'D') {
      Serial.println(">> DISENGAGE — spring returning to 0deg");
      motorStop();
      systemState = DISENGAGED;
    }
    else if (cmd == 'T') {
      Serial.print("Angle: ");
      Serial.print(readAngleDeg(), 2);
      Serial.println(" deg");
    }
  }

  // ── PID loop (runs only when engaged or engaging) ─────────
  if (systemState == ENGAGING || systemState == HOLDING) {

    unsigned long now = millis();
    if (now - lastPidTime >= PID_INTERVAL_MS) {

      float angle = readAngleDeg();
      runPID(angle);

      // Transition ENGAGING → HOLDING once settled at target
      if (systemState == ENGAGING && abs(TARGET_ANGLE - angle) < DEADBAND) {
        systemState = HOLDING;
        Serial.print("Settled. Holding at ");
        Serial.print(angle, 1);
        Serial.println(" deg");
      }

      // Print live data every 500ms — comment out after tuning
      static unsigned long lastPrint = 0;
      if (now - lastPrint >= 500) {
        Serial.print("Angle: ");
        Serial.print(angle, 1);
        Serial.print(" deg  |  Error: ");
        Serial.print(TARGET_ANGLE - angle, 1);
        Serial.print(" deg  |  I: ");
        Serial.println(pidIntegral, 2);
        lastPrint = now;
      }
    }
  }
}
