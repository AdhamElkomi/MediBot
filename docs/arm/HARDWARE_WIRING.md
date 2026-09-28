# MediBot Arm - Hardware Wiring

This document summarizes the hardware connections used by the current MediBot arm controller.

> The ROS 2 software only communicates with the Arduino Mega through USB Serial.
> Motor drivers and limit switches are handled by the Arduino firmware.

---

## 1. Controller

- Arduino Mega 2560
- USB connection to the ROS 2 computer
- Serial baud rate: `115200`

---

## 2. Vertical Rail - NEMA11 + TB6600

Arduino connections:

| Function | Arduino Mega Pin |
|---|---:|
| STEP / PUL | D2 |
| DIR | D3 |

Firmware definitions:

```cpp
const int STEP_PIN = 2;
const int DIR_PIN  = 3;
```

The TB6600 controls the NEMA11 stepper motor used for the vertical rail.

---

## 3. Rail Limit Switches

The limit switches use `INPUT_PULLUP`.

This means:

```text
Not pressed = HIGH
Pressed     = LOW
```

Connections:

| Limit switch | Arduino Mega Pin |
|---|---:|
| Top limit | D22 |
| Bottom limit | D23 |

Firmware definitions:

```cpp
const int LIMIT_TOP_PIN    = 22;
const int LIMIT_BOTTOM_PIN = 23;
```

The top switch is used to stop upward motion.

The bottom switch is used to stop downward motion and defines the vertical HOME reference.

---

## 4. Horizontal Actuator - BTS7960

Arduino connections:

| BTS7960 Signal | Arduino Mega Pin |
|---|---:|
| RPWM | D44 |
| LPWM | D45 |
| REN | D46 |
| LEN | D47 |

Firmware definitions:

```cpp
const int RPWM_PIN = 44;
const int LPWM_PIN = 45;
const int REN_PIN  = 46;
const int LEN_PIN  = 47;
```

The BTS7960 drives the 12 V linear actuator.

The actuator direction is controlled by RPWM / LPWM.

---

## 5. Current Motion Logic

### Vertical Rail

The rail is controlled by:

```text
STEP + DIR
```

and protected by two physical limit switches.

Current firmware commands:

```text
w -> rail up
s -> rail down
```

These internal serial commands are translated by the ROS 2 node.

---

### Linear Actuator

The actuator is controlled through the BTS7960.

Current firmware commands:

```text
e -> extend
r -> retract
```

The current actuator implementation uses a timed full movement.

Current firmware parameter:

```cpp
const unsigned long ACT_FULL_MS = 2500;
```

This value must be calibrated if the actuator, supply voltage or mechanical system changes.

---

## 6. Parallel Motions

Two high-level operations currently use parallel motion:

### HOME

```text
Rail     -> DOWN
Actuator -> RETRACT
```

Both motions run simultaneously.

Final state:

```text
Rail     = DOWN
Actuator = RETRACTED
```

---

### GRAB

Expected initial state:

```text
Rail     = DOWN
Actuator = EXTENDED
```

Parallel movement:

```text
Rail     -> UP
Actuator -> RETRACT
```

Final state:

```text
Rail     = UP
Actuator = RETRACTED
```

---

## 7. Software STOP

The ROS command:

```text
stop
```

is translated internally to:

```text
x
```

It interrupts:

- vertical rail movement
- linear actuator movement
- HOME
- GRAB

This is a software stop and does not replace a physical emergency-stop circuit.

---

## 8. Power Notes

The motor drivers must be powered from their appropriate external power supplies.

The Arduino Mega is connected to the ROS 2 computer through USB for communication.

All control electronics that exchange logic signals must share the required common ground according to the validated wiring of the robot.

Do not power the motors directly from the Arduino Mega.

---

## 9. ROS 2 Connection

The ROS 2 computer communicates only with the Arduino through USB.

Typical device:

```text
/dev/ttyACM0
```

Check with:

```bash
ls -l /dev/ttyACM*
```

The serial port can be changed when launching the ROS 2 driver:

```bash
ros2 run medibot_arm_control arm_serial_node \
  --ros-args -p port:=/dev/ttyACM1
```

---

## 10. Arduino Pin Summary

| Function | Pin |
|---|---:|
| Rail STEP | D2 |
| Rail DIR | D3 |
| Top limit switch | D22 |
| Bottom limit switch | D23 |
| BTS7960 RPWM | D44 |
| BTS7960 LPWM | D45 |
| BTS7960 REN | D46 |
| BTS7960 LEN | D47 |

