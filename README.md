# MediBot

**MediBot** is a modular mobile robotics project designed for the autonomous transport of medical sample boxes in indoor healthcare environments.

The platform is based on a **TurtleBot2 / Kobuki mobile base** and combines:

- autonomous navigation and mapping,
- camera-assisted docking,
- a custom robotic arm for box handling,
- ROS 2-based modular software,
- Arduino-based low-level arm control.

The project is structured so that navigation, docking and arm-control development can evolve independently while remaining easy to integrate at mission level.

---

## Project Scope

MediBot is intended to support a complete sample-transport workflow:

1. Navigate autonomously to a pickup station.
2. Position the robot in front of the sample box.
3. Extend and operate the robotic arm.
4. Pick up the box.
5. Transport it to the requested destination.
6. Deposit the box.
7. Return to an operating or charging area.
8. Perform automatic docking when required.

---

## System Architecture

```text
                     MediBot
                        |
        +---------------+---------------+
        |                               |
        v                               v
 Navigation / SLAM                 Robotic Arm
        |                               |
        v                               v
   Mission Logic                 ROS 2 Arm Driver
        |                               |
        +---------------+---------------+
                        |
                        v
                    TurtleBot2
                        |
                        v
               Docking / Charging
```

The arm subsystem is intentionally independent from the navigation stack.

Navigation, docking and mission-level nodes should interact with the arm only through its ROS 2 interface.

---

## Main Hardware

### Mobile Platform

- TurtleBot2
- Kobuki mobile base
- Kobuki charging dock

### Perception

- RPLIDAR A1M8 2D LiDAR
- Intel RealSense D435 RGB-D camera

### Robotic Arm

- Arduino Mega 2560
- NEMA11 stepper motor
- TB6600 stepper driver
- 12 V linear actuator
- BTS7960 motor driver
- Top and bottom rail limit switches
- Custom mechanical box-handling structure

---

## Software Environment

The current development environment is based on:

- Ubuntu 24.04
- ROS 2 Jazzy
- Python 3
- Kobuki ROS 2 packages
- RPLIDAR ROS 2
- RealSense ROS 2
- SLAM / Nav2 integration
- Arduino firmware for low-level arm control

---

## Repository Structure

```text
MediBot/
├── medibot_arm_control/          ROS 2 package for robotic-arm control
├── firmware/
│   └── medibot_arm_controller/   Arduino Mega firmware
├── docs/
│   └── arm/
│       ├── ROS_INTERFACE.md
│       ├── HARDWARE_WIRING.md
│       └── CLONE_ARM_ONLY.md
├── cad/                          CAD files and assemblies
├── README.md
└── .gitignore
```

Additional packages for navigation, docking, robot description and mission management can be added without changing the public arm interface.

---

## Robotic Arm Control

The robotic arm is implemented through the ROS 2 package:

```text
medibot_arm_control/
```

Its main role is to provide a clean interface between ROS 2 and the Arduino Mega controlling the arm hardware.

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

The internal Arduino one-character protocol is intentionally hidden from other ROS 2 packages.

For complete arm usage:

- [Arm Package Documentation](medibot_arm_control/README.md)
- [ROS 2 Arm Interface](docs/arm/ROS_INTERFACE.md)
- [Hardware Wiring](docs/arm/HARDWARE_WIRING.md)

Arduino firmware:

```text
firmware/medibot_arm_controller/medibot_arm_controller.ino
```

---

## Quick Arm Example

Start the arm driver:

```bash
ros2 run medibot_arm_control arm_serial_node
```

Return the arm to HOME:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'home'}"
```

Extend the actuator:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'extend'}"
```

Execute the grab motion:

```bash
ros2 topic pub --once /arm_command std_msgs/msg/String "{data: 'grab'}"
```

Monitor arm feedback:

```bash
ros2 topic echo /arm_status
```

---

## Clone Only the Robotic Arm

If you only need the **robotic-arm software, firmware and documentation**, you do not need to check out the complete MediBot repository or future CAD files.

A dedicated sparse-checkout guide is available here:

**[Clone Only the Arm](docs/arm/CLONE_ARM_ONLY.md)**

This is the recommended option for collaborators working only on arm integration.

---

## Full Repository Installation

Clone the project inside the `src` directory of a ROS 2 workspace:

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

Source the workspace:

```bash
source ~/turtlebot2_ws/install/setup.bash
```

---

## Arm Integration With Navigation and Docking

A mission node can control the arm through ROS 2 without accessing the Arduino directly.

Example pickup sequence:

```text
Navigate to pickup station
        |
        v
Position robot in front of box
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

This separation allows navigation, docking and arm-control development to remain independent.

---

## Current Arm Limitations

- Vertical rail position is referenced using physical limit switches.
- The horizontal actuator currently uses timed motion rather than direct position feedback.
- `/arm_command` currently uses `std_msgs/msg/String`.
- The software `stop` command does not replace a physical emergency-stop system.

---

## Planned Improvements

Possible future developments include:

- autonomous SLAM and Nav2 integration,
- camera-assisted docking,
- Kobuki charging-dock integration,
- unified ROS 2 launch files,
- URDF / Xacro robot description,
- box detection and pickup alignment,
- ROS 2 Actions for `grab` and `home`,
- actuator position feedback,
- mission-level state machine,
- CAD versioning with Git LFS if required.

---

## Documentation

### Arm Control

- [Arm Package README](medibot_arm_control/README.md)
- [ROS 2 Interface](docs/arm/ROS_INTERFACE.md)
- [Hardware Wiring](docs/arm/HARDWARE_WIRING.md)
- [Clone Only the Arm](docs/arm/CLONE_ARM_ONLY.md)

---

## Collaboration

Feature branches are recommended for collaborative development, for example:

```text
feature/arm-control
feature/slam-navigation
feature/docking
feature/robot-description
```

Changes should preferably be merged into `main` through pull requests after validation.

---

## License

A project license can be added once selected by the team.

