# MediBot Arm - ROS 2 Interface

This document describes the public ROS 2 interface of the MediBot arm.

The navigation, SLAM and docking nodes do not need to communicate directly
with the Arduino.

They only interact with:

- `/arm_command`
- `/arm_status`

---

## 1. Start the Arm Driver

```bash
source ~/turtlebot2_ws/install/setup.bash

ros2 run medibot_arm_control arm_serial_node
```

Default Arduino port:

```text
/dev/ttyACM0
```

If another port is used:

```bash
ros2 run medibot_arm_control arm_serial_node \
  --ros-args \
  -p port:=/dev/ttyACM1
```

---

## 2. Command Topic

Topic:

```text
/arm_command
```

Message type:

```text
std_msgs/msg/String
```

Available commands:

| Command | Action |
|---|---|
| `home` | Return arm to HOME position |
| `rail_up` | Move vertical rail upward |
| `rail_down` | Move vertical rail downward |
| `extend` | Extend horizontal actuator |
| `retract` | Retract horizontal actuator |
| `grab` | Raise rail and retract actuator simultaneously |
| `stop` | Interrupt the current arm motion |

---

## 3. Status Topic

Topic:

```text
/arm_status
```

Message type:

```text
std_msgs/msg/String
```

Monitor status:

```bash
ros2 topic echo /arm_status
```

Examples:

```text
Rail: MONTER...
Rail: STOP (fin de course haut)

Rail: DESCENDRE...
Rail: STOP (fin de course bas)

Verin: SORTIR complet...
Verin: STOP (fin mouvement)

HOME: termine

GRAB: termine
```

---

## 4. HOME

Command:

```text
home
```

Final state:

```text
Rail     = DOWN
Actuator = RETRACTED
```

During HOME, the rail and actuator move simultaneously.

A mission node can detect completion by waiting for:

```text
HOME: termine
```

---

## 5. GRAB

Before executing `grab`, the expected configuration is:

```text
Rail     = DOWN
Actuator = EXTENDED
```

Command:

```text
grab
```

The controller performs:

```text
Rail     -> UP
Actuator -> RETRACT
```

simultaneously.

Completion message:

```text
GRAB: termine
```

Expected final state:

```text
Rail     = UP
Actuator = RETRACTED
```

---

## 6. STOP

Command:

```text
stop
```

It can interrupt:

- rail movement
- actuator movement
- HOME
- GRAB

The arm remains at its current position after the stop.

---

## 7. Example From Another ROS 2 Python Node

A navigation, docking or mission node can publish arm commands directly.

```python
from std_msgs.msg import String

self.arm_command_pub = self.create_publisher(
    String,
    '/arm_command',
    10
)

msg = String()
msg.data = 'grab'

self.arm_command_pub.publish(msg)
```

To monitor arm feedback:

```python
self.arm_status_sub = self.create_subscription(
    String,
    '/arm_status',
    self.arm_status_callback,
    10
)

def arm_status_callback(self, msg):

    if msg.data == 'GRAB: termine':
        self.get_logger().info(
            'Box successfully grabbed.'
        )
```

---

## 8. Example Integration With Navigation

Typical mission:

```text
Navigate to pickup station
        |
        v
Robot positioning / docking
        |
        v
Send "extend"
        |
        v
Position arm in front of box
        |
        v
Send "grab"
        |
        v
Wait for "GRAB: termine"
        |
        v
Continue navigation
```

This allows the navigation stack and the arm controller to remain independent.

---

## 9. Important Integration Rule

External ROS 2 nodes should use semantic commands:

```text
home
grab
extend
retract
rail_up
rail_down
stop
```

They should NOT send the internal Arduino characters:

```text
w
s
e
r
x
h
g
```

The Arduino serial protocol is considered an internal implementation detail
of `medibot_arm_control`.
