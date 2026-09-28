import rclpy
from rclpy.node import Node
from std_msgs.msg import String


class ArmSequenceNode(Node):

    def __init__(self):
        super().__init__('arm_sequence_node')

        self.command_publisher = self.create_publisher(
            String,
            '/arm_command',
            10
        )

        self.status_subscription = self.create_subscription(
            String,
            '/arm_status',
            self.status_callback,
            10
        )

        self.sequence_started = False
        self.sequence_finished = False
        self.current_step = 'rail_up'

        self.start_timer = self.create_timer(
            1.0,
            self.start_sequence
        )

        self.get_logger().info(
            'Arm sequence node ready.'
        )

    def start_sequence(self):

        if self.sequence_started:
            return

        self.sequence_started = True

        msg = String()
        msg.data = 'rail_up'

        self.command_publisher.publish(msg)

        self.get_logger().info(
            'Sequence: rail_up sent'
        )

    def status_callback(self, msg):

        status = msg.data

        self.get_logger().info(
            f'Status received: {status}'
        )

        # ETAPE 1 : rail en haut
        if (
            self.current_step == 'rail_up'
            and 'Rail: STOP (fin de course haut)' in status
        ):
            self.get_logger().info(
                'Sequence step 1 completed: rail is at top.'
            )

            self.current_step = 'extend'

            command = String()
            command.data = 'extend'
            self.command_publisher.publish(command)

            self.get_logger().info(
                'Sequence step 2: extend sent'
            )

        # ETAPE 2 : verin sorti
        elif (
            self.current_step == 'extend'
            and 'Verin: STOP (fin mouvement)' in status
        ):
            self.get_logger().info(
                'Sequence step 2 completed: actuator is extended.'
            )

            self.current_step = 'home'

            command = String()
            command.data = 'home'
            self.command_publisher.publish(command)

            self.get_logger().info(
                'Sequence step 3: home sent'
            )

        # ETAPE 3 : retour HOME termine
        elif (
            self.current_step == 'home'
            and 'HOME: termine' in status
        ):
            self.current_step = 'finished'
            self.sequence_finished = True

            self.get_logger().info(
                'Sequence completed: arm is HOME.'
            )


def main(args=None):

    rclpy.init(args=args)

    node = ArmSequenceNode()

    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()
