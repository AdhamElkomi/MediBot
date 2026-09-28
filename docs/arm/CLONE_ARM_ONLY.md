# Clone Only the MediBot Arm Control

This guide explains how to retrieve only the files required for the MediBot robotic arm without checking out the full repository or future CAD files.

The arm-related parts are:

```text
medibot_arm_control/
firmware/medibot_arm_controller/
docs/arm/
```

## 1. Go to your ROS 2 workspace

```bash
cd ~/turtlebot2_ws/src
```

## 2. Clone the repository without downloading all file contents

Replace `<REPOSITORY_URL>` with the HTTPS or SSH URL of the MediBot repository.

```bash
git clone --filter=blob:none --no-checkout <REPOSITORY_URL> MediBot-arm-only
```

Then enter the repository:

```bash
cd MediBot-arm-only
```

## 3. Enable sparse checkout

```bash
git sparse-checkout init --cone
```

Select only the robotic-arm files:

```bash
git sparse-checkout set   medibot_arm_control   firmware/medibot_arm_controller   docs/arm
```

Checkout the main branch:

```bash
git checkout main
```

## 4. Verify the retrieved files

```bash
tree -L 3
```

The useful structure should contain:

```text
MediBot-arm-only/
├── medibot_arm_control/
├── firmware/
│   └── medibot_arm_controller/
└── docs/
    └── arm/
```

Large CAD directories do not need to be checked out.

## 5. Build the ROS 2 arm package

Return to the workspace root:

```bash
cd ~/turtlebot2_ws
```

Install dependencies:

```bash
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

Verify the package:

```bash
ros2 pkg executables medibot_arm_control
```

Expected executables:

```text
medibot_arm_control arm_serial_node
medibot_arm_control arm_sequence_node
```

## 6. Start the Arm Driver

Connect the Arduino Mega and check the serial port:

```bash
ls -l /dev/ttyACM*
```

Then run:

```bash
ros2 run medibot_arm_control arm_serial_node
```

If the Arduino is not on `/dev/ttyACM0`:

```bash
ros2 run medibot_arm_control arm_serial_node   --ros-args -p port:=/dev/ttyACM1
```

## 7. Updating the Arm Files Later

Inside the sparse clone:

```bash
cd ~/turtlebot2_ws/src/MediBot-arm-only
git pull
```

Git keeps the sparse-checkout configuration, so only the selected arm-related directories are checked out.

## Notes

- The Arduino firmware is available in `firmware/medibot_arm_controller/`.
- ROS 2 usage is documented in `medibot_arm_control/README.md`.
- The public arm interface is documented in `docs/arm/ROS_INTERFACE.md`.
- Wiring is documented in `docs/arm/HARDWARE_WIRING.md`.
- If the repository is private, GitHub authentication is still required.
