cmake_minimum_required(VERSION 2.8.12)
project(ur10driver)

add_compile_options(-O3 -Wall -fPIC -std=c++0x)

include_directories(
  include
)

set(${PROJECT_NAME}_SOURCES
    src/ros/lowbandwidth_trajectory_follower.cpp
    src/ros/trajectory_follower.cpp
    src/tcp_socket.cpp
    src/ur/commander.cpp
    src/ur/master_board.cpp
    src/ur/messages.cpp
    src/ur/robot_mode.cpp
    src/ur/rt_state.cpp
    src/ur/server.cpp
  src/ur/stream.cpp
  src/ur10driver.cpp
 )

add_executable(ur10test src/test.cpp)

add_library(ur10driver SHARED ${${PROJECT_NAME}_SOURCES})

target_link_libraries(ur10test ur10driver)

target_link_libraries(ur10driver
  pthread atomic
)


