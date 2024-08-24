cmake_minimum_required(VERSION 3.8)
project(user_thread_scheduler)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 找到必要的包
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

# 包含头文件目录
include_directories(
  include
  include/user_thread_scheduler
  /opt/ros/foxy/include  # 确保包含rclcpp头文件
)

# 链接静态库
link_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/src  # 确保CMake知道libtask.a的位置
  /opt/ros/foxy/lib  # 确保包含rclcpp库
)

# 添加可执行文件
add_executable(thread_scheduler
  main.c
  device/console.c
  device/ioqueue.c
  interrupt/analog_interrupt.c
  interrupt/timer.c
  lib/assert.c
  lib/bitmap.c
  lib/list.c
  task/context_swap.asm
  task/debug.c
  task/died_context_swap.asm
  task/get_reg.asm
  task/init.c
  task/sync.c
  task/task.c
)

# 链接库
ament_target_dependencies(thread_scheduler
  rclcpp
  std_msgs
)

target_link_libraries(thread_scheduler
  ${CMAKE_CURRENT_SOURCE_DIR}/libtask.a  # 静态库
  dl  # 添加动态库链接
  rclcpp  # 添加rclcpp库链接
)

# 安装目标
install(TARGETS
  thread_scheduler
  DESTINATION lib/${PROJECT_NAME})
#启用测试,添加测试依赖项
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()
#将项目标记为ament包，供ROS 2使用
ament_package()
