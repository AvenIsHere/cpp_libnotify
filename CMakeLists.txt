cmake_minimum_required(VERSION 4.1)
project(cpp_libnotify)

set(CMAKE_CXX_STANDARD 20)

find_package(PkgConfig REQUIRED)

# This finds both libnotify and its dependency, glib-2.0
pkg_check_modules(LIBNOTIFY REQUIRED libnotify)

add_library(cpp_libnotify STATIC library.cpp)

# Add the include directories found by pkg-config
target_include_directories(cpp_libnotify PRIVATE ${LIBNOTIFY_INCLUDE_DIRS})

# Link the libraries
target_link_libraries(cpp_libnotify PRIVATE ${LIBNOTIFY_LIBRARIES})
