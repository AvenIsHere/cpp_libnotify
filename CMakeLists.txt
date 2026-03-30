cmake_minimum_required(VERSION 4.1)
project(cpp_libnotify)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)

add_library(cpp_libnotify STATIC Notification.cpp)

target_include_directories(cpp_libnotify PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${LIBNOTIFY_INCLUDE_DIRS})
target_link_libraries(cpp_libnotify PUBLIC ${LIBNOTIFY_LIBRARIES})
