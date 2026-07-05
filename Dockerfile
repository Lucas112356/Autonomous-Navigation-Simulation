FROM osrf/ros:jazzy-desktop-full

# Install dependencies
RUN apt-get update && apt-get install -y \
    ros-jazzy-nav2-bringup \
    ros-jazzy-nav2-msgs \
    ros-jazzy-ros-gz-bridge \
    ros-jazzy-ros-gz-sim \
    ros-jazzy-robot-state-publisher \
    ros-jazzy-xacro \
    ros-jazzy-tf2-ros \
    ros-jazzy-tf2-msgs \
    python3-colcon-common-extensions \
    && rm -rf /var/lib/apt/lists/*

# Create workspace and copy source
WORKDIR /ros2_ws/src
COPY . navigation_sim/

# Build
WORKDIR /ros2_ws
RUN /bin/bash -c "source /opt/ros/jazzy/setup.bash && colcon build --symlink-install"

# Source workspace on shell entry
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc
RUN echo "source /ros2_ws/install/setup.bash" >> /root/.bashrc

WORKDIR /ros2_ws
CMD ["/bin/bash"]