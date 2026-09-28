import time
import serial

import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class ArmSerialNode(Node):

    def __init__(self):
        super().__init__('arm_serial_node')

        self.declare_parameter('port', '/dev/ttyACM0')
        self.declare_parameter('baudrate', 115200)
        
        self.port = self.get_parameter('port').value
        self.baudrate = self.get_parameter('baudrate').value
	
        self.get_logger().info(
            f'Opening Arduino on {self.port} at {self.baudrate} baud...'
        )

        self.serial_port = serial.Serial(
            self.port,
            self.baudrate,
            timeout=0.1
        )

        # Arduino Mega resets when serial port is opened
        time.sleep(2)

        self.get_logger().info('Arduino connected.')

        self.subscription = self.create_subscription(
            String,
            '/arm_command',
            self.command_callback,
            10
        )

        self.status_publisher = self.create_publisher(
            String,
            '/arm_status',
            10
        )

        self.serial_timer = self.create_timer(
            0.05,
            self.read_serial
        )

        self.get_logger().info(
            'Waiting for commands: rail_up, rail_down, extend, retract, stop, home, grab'
        )

    def command_callback(self, msg):
        command = msg.data.strip().lower()

        command_map = {
	    'w': 'w',
	    's': 's',
	    'e': 'e',
	    'r': 'r',
	    'x': 'x',
	    'h': 'h',
	    'g': 'g',
	    

	    'rail_up': 'w',
	    'rail_down': 's',
	    'extend': 'e',
	    'retract': 'r',
	    'stop': 'x',
	    'home': 'h',
	    'grab': 'g'
	}

        if command not in command_map:
            self.get_logger().warning(
                f'Invalid command: {command}'
            )    
            return

        arduino_command = command_map[command]

        self.serial_port.write(
            arduino_command.encode()
        )
        self.serial_port.flush()

        self.get_logger().info(
            f'ROS command: {command} -> Arduino: {arduino_command}'
        )

    def read_serial(self):
        while self.serial_port.in_waiting:
            line = self.serial_port.readline().decode(
                'utf-8',
                errors='replace'
            ).strip()

            if line:
                self.get_logger().info(f'Arduino: {line}')

                msg = String()
                msg.data = line
                self.status_publisher.publish(msg)

    def destroy_node(self):
        if self.serial_port.is_open:
            self.serial_port.close()

        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)

    node = ArmSerialNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
