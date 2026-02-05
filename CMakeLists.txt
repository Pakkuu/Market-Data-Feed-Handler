cmake_minimum_required(VERSION 3.14)
project(MarketDataFeedHandler VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for optimization and warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/tick_generator.cpp
)

# Executable
add_executable(market_feed_handler ${SOURCES})

# Link pthread library (required for std::thread)
find_package(Threads REQUIRED)
target_link_libraries(market_feed_handler Threads::Threads)

# Enable testing
enable_testing()
