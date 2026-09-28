# medibot_arm_control

ROS 2 package used to control the MediBot robotic arm through an Arduino Mega 2560.

This package is the public ROS 2 interface of the arm. Navigation, SLAM, docking and mission nodes should interact with the arm through this package rather than sending serial commands directly to the Arduino.

## Overview

The current arm has two controlled motions:

1. Vertical rail
   - NEMA11 stepper motor
   - TB6600 driver
   - Top and bottom limit switches

2. Horizontal actuator
   - 12 V linear actuator
   - BTS7960 driver

Architecture:

```text
ROS 2
  |
  | /arm_command
  v
arm_serial_node
  |
  | USB Serial - 115200 baud
  v
Arduino Mega
  |
  +-- TB6600 -> NEMA11 vertical rail
  |
  +-- BTS7960 -> Linear actuator
```

Feedback:

```text
Arduino Mega
     |
     | Serial status
     v
arm_serial_node
     |
     | /arm_status
     v
ROS 2
```

## Package Contents

```text
medibot_arm_control/
├── medibot_arm_control/
│   ├── __init__.py
│   ├── arm_serial_node.py
│   └── arm_sequence_node.py
├── resource/
├── test/
├── package.xml
├── setup.cfg
├── setup.py
└── README.md
```

## Nodes

### `arm_serial_node`

Main arm driver.

It:
- opens the Arduino serial port;
- receives semantic ROS 2 commands;
- translates them into the Arduino protocol;
- sends them to the Arduino;
- reads Arduino feedback;
- publishes feedback on `/arm_status`.

Run:

```bash
ros2 run medibot_arm_control arm_serial_node
```

### `arm_sequence_node`

Development node used to validate automatic arm sequences based on `/arm_status`.

Run:

```bash
ros2 run medibot_arm_control arm_sequence_node
```

For integration with navigation or docking, `arm_serial_node` is the essential node.

## Build

```bash
cd ~/turtlebot2_ws
rosdep install --from-paths src --ignore-src -r -y
colcon build --symlink-install
source ~/turtlebot2_ws/install/setup.bash
```

Verify:

```bash
ros2 pkg executables medibot_arm_control
```

Expected:

```text
medibot_arm_control arm_serial_node
medibot_arm_control arm_sequence_node
```

## Arduino Connection

Connect the Arduino Mega through USB.

Check the port:

```bash
ls -l /dev/ttyACM*
```

Typical result:

```text
/dev/ttyACM0
```

The user should belong to the `dialout` group:

```bash
groups
```

If needed:

```bash
sudo usermod -aG dialout $USER
```

Then log out or reboot.

## Start the Driver

Default:

```bash
source ~/turtlebot2_ws/install/setup.bash
ros2 run medibot_arm_control arm_serial_node
```

Default parameters:

```text
port     = /dev/ttyACM0
baudrate = 115200
```

Use another port:

```bash
ros2 run medibot_arm_control arm_serial_node   --ros-args -p port:=/dev/ttyACM1
```

## ROS 2 Interface

Command topic:

```text
/arm_command
```

Type:

```text
std_msgs/msg/String
```

Status topic:

```text
/arm_status
```

Type:

```text
std_msgs/msg/String
```

Monitor feedback:

```bash
ros2 topic echo /arm_status
```

## Available Commands

| Command | Function |
|---|---|
| `home` | Return the arm to HOME |
| `rail_up` | Move the vertical rail upward |
| `rail_down` | Move the vertical rail downward |
| `extend` | Extend the horizontal actuator |
| `retract` | Retract the horizontal actuator |
| `grab` | Raise the rail and retract the actuator in parallel |
| `stop` | Interrupt the active arm motion |

External ROS 2 nodes should use these semantic commands.

---

## `home`

Return the arm to:

```text
Rail     = DOWN
Actuator = RETRACTED
```

Command:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'home'}"
```

The rail and actuator move in parallel.

Typical feedback:

```text
HOME: debut (mouvements paralleles)
HOME: verin rentre
HOME: rail en bas
HOME: termine
```

Completion:

```text
HOME: termine
```

---

## `rail_up`

Move the rail upward to the top limit switch:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'rail_up'}"
```

Typical feedback:

```text
Rail: MONTER...
Rail: STOP (fin de course haut)
```

Completion:

```text
Rail: STOP (fin de course haut)
```

---

## `rail_down`

Move the rail downward to the bottom limit switch:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'rail_down'}"
```

Typical feedback:

```text
Rail: DESCENDRE...
Rail: STOP (fin de course bas)
```

Completion:

```text
Rail: STOP (fin de course bas)
```

---

## `extend`

Extend the horizontal actuator:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'extend'}"
```

Typical feedback:

```text
Verin: SORTIR complet...
Verin: STOP (fin mouvement)
```

The current actuator has no position sensor, so full movement is time-based in firmware.

---

## `retract`

Retract the horizontal actuator:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'retract'}"
```

Typical feedback:

```text
Verin: RENTRER complet...
Verin: STOP (fin mouvement)
```

---

## `grab`

Execute the box-grabbing movement:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'grab'}"
```

Expected initial state:

```text
Rail     = DOWN
Actuator = EXTENDED
```

Parallel motion:

```text
Rail     -> UP
Actuator -> RETRACT
```

Typical feedback:

```text
GRAB: debut (rail UP + verin RETRACT)
GRAB: verin rentre
GRAB: rail en haut
GRAB: termine
```

Completion:

```text
GRAB: termine
```

Expected final state:

```text
Rail     = UP
Actuator = RETRACTED
```

---

## `stop`

Interrupt the current movement:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'stop'}"
```

It can interrupt:

- `rail_up`
- `rail_down`
- `extend`
- `retract`
- `home`
- `grab`

Possible feedback:

```text
Rail: STOP (commande x)
Verin: STOP (commande x)
HOME: STOP (commande x)
GRAB: STOP (commande x)
```

If no movement is active:

```text
STOP: aucun mouvement actif
```

`stop` is a software stop and does not replace a physical emergency stop.

## Quick Reference

```bash
# HOME
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'home'}"

# Rail up
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'rail_up'}"

# Rail down
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'rail_down'}"

# Extend
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'extend'}"

# Retract
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'retract'}"

# Grab
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'grab'}"

# Stop
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'stop'}"
```

## Integration From Another ROS 2 Python Node

Publisher:

```python
from std_msgs.msg import String

self.arm_command_pub = self.create_publisher(
    String,
    '/arm_command',
    10
)
```

Send a command:

```python
msg = String()
msg.data = 'grab'
self.arm_command_pub.publish(msg)
```

Subscribe to status:

```python
self.arm_status_sub = self.create_subscription(
    String,
    '/arm_status',
    self.arm_status_callback,
    10
)

def arm_status_callback(self, msg):
    if msg.data == 'GRAB: termine':
        self.get_logger().info('Box grabbing completed.')
```

## Example Mission Logic

```text
Navigate to pickup station
        |
        v
Position robot in front of box
        |
        v
Publish "extend"
        |
        v
Wait for actuator completion
        |
        v
Publish "grab"
        |
        v
Wait for "GRAB: termine"
        |
        v
Continue navigation
```

For HOME:

```text
Publish "home"
        |
        v
Wait for "HOME: termine"
        |
        v
Continue mission
```

## Internal Arduino Protocol

The ROS 2 driver translates semantic commands internally:

```text
rail_up   -> w
rail_down -> s
extend    -> e
retract   -> r
stop      -> x
home      -> h
grab      -> g
```

Other ROS 2 packages should not use these characters directly.

## Troubleshooting

### Arduino port missing

```bash
lsusb
ls -l /dev/ttyACM*
```

After flashing firmware, the port may temporarily disappear and reappear.

### Permission denied

Check:

```bash
groups
```

Ensure `dialout` is present.

### Port already in use

Close any Arduino Serial Monitor, Python serial test or previous `arm_serial_node`.

### Command published but no movement

Check the driver:

```bash
ros2 node list
```

Check the topic:

```bash
ros2 topic info /arm_command
```

Check feedback:

```bash
ros2 topic echo /arm_status
```

## Current Limitations

- Vertical position is referenced using top and bottom limit switches.
- Horizontal actuator movement is time-based.
- `/arm_command` uses `std_msgs/msg/String`.
- `grab` expects the rail down and actuator extended before starting.
- `home` moves the rail and actuator simultaneously.
- Future versions may use ROS 2 Actions, Services and explicit arm state messages.

## Related Files

Firmware:

```text
../firmware/medibot_arm_controller/medibot_arm_controller.ino
```

ROS interface:

```text
../docs/arm/ROS_INTERFACE.md
```

Hardware wiring:

```text
../docs/arm/HARDWARE_WIRING.md
```
