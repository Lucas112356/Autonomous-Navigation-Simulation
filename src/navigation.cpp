#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_msgs/msg/tf_message.hpp>
#include <chrono>

using namespace std::chrono_literals;

class Navigation : public rclcpp::Node
{
public:
  Navigation() : Node("navigation"), done(false), started(false)
  {
    publisher = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);

    odom_sub = this->create_subscription<nav_msgs::msg::Odometry>(
      "/odom", 10,
      std::bind(&Navigation::odom_callback, this, std::placeholders::_1)
    );

    ground_truth_sub = this->create_subscription<tf2_msgs::msg::TFMessage>(
      "/ground_truth_pose", 10,
      [this](const tf2_msgs::msg::TFMessage::SharedPtr msg)
      {
        if (!msg->transforms.empty())
        {
          true_x = msg->transforms[0].transform.translation.x;
          true_y = msg->transforms[0].transform.translation.y;
        }
      });

    timer = this->create_wall_timer(100ms, std::bind(&Navigation::timer_callback, this));

    speed = 1.0;
    distance = 5.5;
  }

private:
  void odom_callback(const nav_msgs::msg::Odometry::SharedPtr msg)
  {
    current_x = msg->pose.pose.position.x;

    if (!started) {
      start_x = current_x;
      started = true;
      start_time = this->now();
      RCLCPP_INFO(this->get_logger(), "Start position: %.2f", start_x);
    }
  }

  void timer_callback()
  {
    if (done || !started) return;

    auto msg = geometry_msgs::msg::Twist();

    double travelled = std::abs(current_x - start_x);

    if (travelled < distance)
    {
      msg.linear.x = speed;
      RCLCPP_INFO(this->get_logger(), "Travelled: %.2fm / %.1fm", travelled, distance);
    }
    else
    {
      msg.linear.x = 0.0;
      done = true;

      double elapsed = (this->now() - start_time).seconds();
      RCLCPP_INFO(this->get_logger(), "Target distance reached at x=%.2f — stopping.", current_x);
      RCLCPP_INFO(this->get_logger(), "Navigation completed in %.3f seconds.", elapsed);
      RCLCPP_INFO(this->get_logger(), "Final position: x=%.2f y=%.2f", true_x, true_y);
    }

    publisher->publish(msg);
  }

  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub;
  rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr ground_truth_sub;
  rclcpp::TimerBase::SharedPtr timer;

  double speed;
  double distance;
  double start_x = 0.0;
  double current_x = 0.0;
  double true_x = 0.0;
  double true_y = 0.0;
  bool done;
  bool started;
  rclcpp::Time start_time;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<Navigation>());
  rclcpp::shutdown();
  return 0;
}