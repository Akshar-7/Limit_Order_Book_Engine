# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Name your project and optionally set a version
project(Limit_Order_Book_Engine VERSION 1.0)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")

# Tell CMake to find the OS threading library (Required for std::thread)
find_package(Threads REQUIRED)

# Tell CMake to create an executable named "my_engine" 
# and list the source files required to build it.
add_executable(Limit_Order_Book_Engine
    main.cpp
    benchmark.cpp
)

# Tell the compiler where to find all your .h files
target_link_directories(Limit_Order_Book_Engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Wire the threading library into your executable
target_link_libraries(Limit_Order_Book_Engine PRIVATE Threads::Threads)

target_link_options(Limit_Order_Book_Engine PRIVATE -Wl,--stack,16777216)