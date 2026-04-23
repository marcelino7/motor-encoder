// ============================================================
//  servo_config.h — All tunable parameters in one place
//  Edit this file to change positions and speeds
// ============================================================

#ifndef SERVO_CONFIG_H
#define SERVO_CONFIG_H

// ── Positions (centi-degrees, range 0–24000) ─────────────────
#define POS_HOME        2500    // Home position   (~25°)
#define POS_BRAKE       7500    // Brake position  (~75°)

// ── Move durations (milliseconds) ────────────────────────────
// Lower = faster,  Higher = slower
#define SPEED_TO_HOME   100     // ms to reach home
#define SPEED_TO_BRAKE  100     // ms to reach brake

// ── Delays after each move (must be > move duration) ─────────
#define DELAY_AFTER_HOME    500   // ms
#define DELAY_AFTER_BRAKE   500   // ms

// ── Startup behaviour ─────────────────────────────────────────
// On power-on the servo will move to this position first
#define STARTUP_POS     POS_BRAKE
#define STARTUP_SPEED   SPEED_TO_BRAKE
#define STARTUP_DELAY   DELAY_AFTER_BRAKE

#endif
