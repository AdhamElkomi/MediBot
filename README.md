# MediBot

MediBot is an autonomous mobile robot project designed for the transport of medical sample boxes in indoor healthcare environments.

The platform is based on a TurtleBot2 / Kobuki mobile base and combines autonomous navigation, mapping, docking and a custom robotic arm for handling sample boxes.

## Project Objectives

MediBot is intended to perform a complete transport mission:

1. Navigate autonomously inside the environment.
2. Reach a pickup station.
3. Position the robot in front of the sample box.
4. Use the robotic arm to pick up the box.
5. Transport the box to the destination.
6. Deposit the box.
7. Return to its operating or charging area.
8. Perform automatic docking when required.

## Main Hardware

- TurtleBot2 / Kobuki mobile base
- RPLIDAR A1M8 2D LiDAR
- Intel RealSense D435 RGB-D camera
- Arduino Mega 2560
- NEMA11 stepper motor
- TB6600 stepper driver
- 12 V linear actuator
- BTS7960 motor driver
- Rail limit switches
- Custom mechanical arm / box handling system

## Software Environment

- Ubuntu 24.04
- ROS 2 Jazzy
- Python 3
- Kobuki ROS 2 packages
- RPLIDAR ROS 2
- RealSense ROS 2
- SLAM / Nav2 integration
- Arduino firmware for low-level arm control

## Repository Structure

```text
MediBot/
├── medibot_arm_control/          ROS 2 package for the robotic arm
├── firmware/
│   └── medibot_arm_controller/   Arduino Mega firmware
├── docs/
│   └── arm/
│       ├── ROS_INTERFACE.md
│       └── HARDWARE_WIRING.md
├── cad/                          CAD files (when added)
├── README.md
└── .gitignore
```

## Robotic Arm Control

The robotic arm is implemented as an independent ROS 2 package:

```text
medibot_arm_control/
```

It provides the communication layer between ROS 2 and the Arduino Mega controlling the arm.

### ROS 2 Topics

Commands:

```text
/arm_command
```

Type:

```text
std_msgs/msg/String
```

Feedback:

```text
/arm_status
```

Type:

```text
std_msgs/msg/String
```

### Available High-Level Commands

```text
home
rail_up
rail_down
extend
retract
grab
stop
```

Documentation:

- [Arm ROS 2 Package](medibot_arm_control/README.md)
- [ROS 2 Arm Interface](docs/arm/ROS_INTERFACE.md)
- [Hardware Wiring](docs/arm/HARDWARE_WIRING.md)

Arduino firmware:

```text
firmware/medibot_arm_controller/medibot_arm_controller.ino
```

## Build

Clone the repository inside a ROS 2 workspace:

```bash
cd ~/turtlebot2_ws/src
git clone <REPOSITORY_URL>
```

Install dependencies:

```bash
cd ~/turtlebot2_ws
rosdep install --from-paths src --ignore-src -r -y
```

Build:

```bash
colcon build --symlink-install
```

Source:

```bash
source ~/turtlebot2_ws/install/setup.bash
```

## Integration Philosophy

```text
Navigation / SLAM
       |
       v
Mission logic
   /       \
  v         v
Docking   Arm control
             |
             v
        Arduino Mega
```

The navigation, docking and mission software should interact with the arm through ROS 2, not directly through the Arduino serial protocol.

## Arm Integration Example

```text
Navigate to pickup station
        |
        v
Position robot
        |
        v
Send "extend"
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

## Sparse Checkout

If the repository later contains large CAD files, a developer who only needs the arm software can use sparse checkout:

```bash
git clone --filter=blob:none --no-checkout <REPOSITORY_URL> MediBot
cd MediBot

git sparse-checkout init --cone
git sparse-checkout set medibot_arm_control firmware docs
git checkout main
```

## Current Arm Limitations

- The vertical rail uses physical limit switches.
- The horizontal actuator currently uses timed motion rather than position feedback.
- `/arm_command` currently uses `std_msgs/msg/String`.
- The software `stop` command does not replace a physical emergency-stop system.

## Future Work

- SLAM and autonomous navigation integration
- Camera-based docking
- Kobuki charging dock integration
- Unified launch files
- Robot URDF / Xacro model
- Box detection and pickup positioning
- ROS 2 Actions for arm operations
- Actuator position feedback
- Mission-level state machine
- CAD integration and Git LFS for large binary files

