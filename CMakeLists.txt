cmake_minimum_required(VERSION 3.8)
project(navigation_sim)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(tf2_msgs REQUIRED)

include_directories(
  /opt/ros/jazzy/include/nav_msgs
)

add_executable(navigation src/navigation.cpp)
ament_target_dependencies(navigation 
    rclcpp 
    geometry_msgs 
    nav_msgs
    tf2_msgs
)


add_executable(turn_navigation src/turn_navigation.cpp)
ament_target_dependencies(turn_navigation
    rclcpp
    rclcpp_action
    nav2_msgs
    geometry_msgs
    tf2_msgs
)

add_executable(half_roundabout_navigation src/half_roundabout_navigation.cpp)
ament_target_dependencies(half_roundabout_navigation
    rclcpp
    rclcpp_action
    nav2_msgs
    geometry_msgs
    tf2_msgs
)

install(TARGETS navigation
  DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS turn_navigation
    DESTINATION lib/${PROJECT_NAME}
)

install(TARGETS half_roundabout_navigation
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY urdf worlds launch config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()