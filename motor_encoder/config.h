// ============================================================
//  config.h — All tunable parameters
//  0° = spring fully released (home)
//  30° = brake engaged (motor holding against spring)
// ============================================================

#ifndef CONFIG_H
#define CONFIG_H

// ── Target position ───────────────────────────────────────────
#define TARGET_ANGLE       80.0f   // degrees — brake engaged position
#define DEADBAND            5.0f    // degrees — PID won't act within ±this

// ── PID gains (tune these on your bench) ─────────────────────
// Start with only Kp, keep Ki and Kd at 0 until stable
#define KP                  1.2f    // Proportional: main correction force
#define KI                  0.1f   // Integral: removes steady-state error
#define KD                  0.1f    // Derivative: damps oscillation

// ── Integral windup clamp ─────────────────────────────────────
// Prevents integral from growing too large if motor stalls
#define INTEGRAL_LIMIT      50.0f

// ── PWM output limits ─────────────────────────────────────────
#define PWM_MIN             30      // Minimum PWM to overcome static friction
#define PWM_MAX             220     // Maximum PWM (leave headroom from 255)

// ── Motor pins (PMBA30F / CBA30 driver) ──────────────────────
#define PIN_CW              2
#define PIN_CCW             3
#define PIN_STOP_MODE       4
#define PIN_SPD_SET         5
#define PIN_ALM_RST         6
#define PIN_SPEED_PWM       9

// ── AS5600 ────────────────────────────────────────────────────
#define AS5600_ADDR         0x36
#define AS5600_RAW_MAX      4096.0f

// ── PID loop timing ───────────────────────────────────────────
#define PID_INTERVAL_MS     20      // Run PID every 20ms = 50 Hz

// ── Serial commands ───────────────────────────────────────────
// Send these via Serial Monitor (no line ending or newline)
// 'E' = Engage brake (motor on, PID holds 30°)
// 'D' = Disengage (motor off, spring returns)
// 'T' = Print current angle (for tuning/debug)

#endif
