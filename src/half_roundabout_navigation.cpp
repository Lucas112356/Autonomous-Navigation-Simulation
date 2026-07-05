#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/follow_waypoints.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <tf2_msgs/msg/tf_message.hpp>
#include <cmath>
#include <vector>
#include <tuple>

using FollowWaypoints = nav2_msgs::action::FollowWaypoints;

class HalfRoundaboutNavigator : public rclcpp::Node
{
public:
    HalfRoundaboutNavigator() : Node("half_roundabout_navigator")
    {
        client = rclcpp_action::create_client<FollowWaypoints>(this, "/follow_waypoints");

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

        log_timer = create_wall_timer(
            std::chrono::seconds(1),
            [this]()
            {
                RCLCPP_INFO(get_logger(), "Position: x=%.2f y=%.2f", true_x, true_y);
            });

        send_timer = create_wall_timer(
            std::chrono::milliseconds(500),
            [this]()
            { send_waypoints(); });
    }

private:
    rclcpp_action::Client<FollowWaypoints>::SharedPtr client;
    rclcpp::Subscription<tf2_msgs::msg::TFMessage>::SharedPtr ground_truth_sub;
    rclcpp::TimerBase::SharedPtr log_timer;
    rclcpp::TimerBase::SharedPtr send_timer;
    rclcpp::Time start_time;
    double true_x = 0.0;
    double true_y = 0.0;

    geometry_msgs::msg::PoseStamped make_pose(double x, double y, double yaw)
    {
        geometry_msgs::msg::PoseStamped pose;
        pose.header.frame_id = "odom";
        pose.header.stamp = now();
        pose.pose.position.x = x;
        pose.pose.position.y = y;
        pose.pose.orientation.z = std::sin(yaw / 2.0);
        pose.pose.orientation.w = std::cos(yaw / 2.0);
        return pose;
    }

    void send_waypoints()
    {
        send_timer->cancel();

        if (!client->wait_for_action_server(std::chrono::seconds(10)))
        {
            RCLCPP_ERROR(get_logger(), "Action server not available");
            return;
        }

        std::vector<std::tuple<double, double, double>> wp = {
            {3.400, 0.000, 0.93},   // Finishing first straight
            {5.000, 3.100, 0.78},   // Climbing Up-Left
            {7.500, 4.500, 0.27},   // Nearing the left shoulder
            {9.300, 5.000, 0.00},   // Transitioning to flat peak
            {11.200, 5.000, 5.96},  // Flat apex, heading straight Left
            {13.000, 4.400, 5.59},  // Descending Down-Left
            {14.700, 3.000, 5.44},  // Continuing down the slope
            {17.400, 0.000, 0.00},  // Approaching the bottom floor
            {20.000, 0.000, 0.00}   // Exiting straight Left
        };

        auto goal = FollowWaypoints::Goal();
        for (auto &[x, y, yaw] : wp)
        {
            goal.poses.push_back(make_pose(x, y, yaw));
        }

        RCLCPP_INFO(get_logger(), "Sending %zu waypoints", goal.poses.size());
        start_time = rclcpp::Clock(RCL_STEADY_TIME).now();

        auto options = rclcpp_action::Client<FollowWaypoints>::SendGoalOptions();
        options.result_callback = [this](const auto &)
        {
            double elapsed = (rclcpp::Clock(RCL_STEADY_TIME).now() - start_time).seconds();
            RCLCPP_INFO(get_logger(), "Course complete!");
            RCLCPP_INFO(get_logger(), "Navigation completed in %.3f seconds.", elapsed);
            RCLCPP_INFO(get_logger(), "Final position: x=%.2f y=%.2f", true_x, true_y);
        };

        client->async_send_goal(goal, options);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<HalfRoundaboutNavigator>());
    rclcpp::shutdown();
    return 0;
}