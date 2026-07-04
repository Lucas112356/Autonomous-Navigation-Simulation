#include <rclcpp/rclcpp.hpp>
#include <rclcpp_action/rclcpp_action.hpp>
#include <nav2_msgs/action/follow_waypoints.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <cmath>
#include <vector>
#include <tuple>

using FollowWaypoints = nav2_msgs::action::FollowWaypoints;

class TurnNavigator : public rclcpp::Node
{
public:
    TurnNavigator() : Node("turn_navigator")
    {
        client = rclcpp_action::create_client<FollowWaypoints>(this, "/follow_waypoints");

        timer = create_wall_timer(
            std::chrono::milliseconds(100),
            [this]() { send_waypoints(); });
    }

private:
    rclcpp_action::Client<FollowWaypoints>::SharedPtr client;
    rclcpp::TimerBase::SharedPtr timer;

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
        timer->cancel();

        if (!client->wait_for_action_server(std::chrono::seconds(10))) {
            RCLCPP_ERROR(get_logger(), "Action server not available");
            return;
        }

        std::vector<std::tuple<double, double, double>> wp = {
            {5.0,  0.0,  0.0}, // straight line 
            {7.00, 0.54, 0.6}, // first turn
            {7.83, 1.17, 0.9}, // middle turn
            {8.47, 2.00, 1.1}, // end of turn
            {9.00, 4.00, 1.20}, // straight going right
            {9.00, 9.00, 1.25}, // endpoint
        };

        auto goal = FollowWaypoints::Goal();
        for (auto& [x, y, yaw] : wp) {
            goal.poses.push_back(make_pose(x, y, yaw));
        }

        RCLCPP_INFO(get_logger(), "Sending %zu waypoints", goal.poses.size());

        auto options = rclcpp_action::Client<FollowWaypoints>::SendGoalOptions();
        options.result_callback = [this](const auto&) {
            RCLCPP_INFO(get_logger(), "Course complete!");
        };

        client->async_send_goal(goal, options);
    }
};

int main(int argc, char ** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<TurnNavigator>());
    rclcpp::shutdown();
    return 0;
}