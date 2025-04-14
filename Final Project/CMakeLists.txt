cmake_minimum_required(VERSION 3.15)

project(Final_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)

# Find Boost libraries
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(Threads REQUIRED)

# Add Crow (header-only) - assumes crow is in the project folder or installed via vcpkg
# If you downloaded Crow manually, place crow_all.h or crow/ in your source tree
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/crow)  # adjust if crow is in a different path

# Add your source files here
add_executable(Final_Project
    main.cpp
)

# Link libraries
target_link_libraries(Final_Project
    ${Boost_LIBRARIES}
    Threads::Threads
)