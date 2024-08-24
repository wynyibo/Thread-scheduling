cmake_minimum_required(VERSION 3.8)
project(user)  # 修改为你的包名

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 启用 C++14 标准
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

include_directories(include)

# 添加 publisher_node 可执行文件
add_executable(publisher_node publisher_node.cpp)
ament_target_dependencies(publisher_node rclcpp std_msgs)

# 添加 subscriber_node 可执行文件
add_executable(subscriber_node subscriber_node.cpp)
ament_target_dependencies(subscriber_node rclcpp std_msgs)

# 安装目标
install(TARGETS
  publisher_node
  subscriber_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
