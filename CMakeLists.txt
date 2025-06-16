cmake_minimum_required(VERSION 3.5)
project(ticket_bot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ensure Homebrew prefix is searched (adjust if using different prefix)
if(DEFINED ENV{HOMEBREW_PREFIX})
    set(HBREW_PREFIX $ENV{HOMEBREW_PREFIX})
else()
    set(HBREW_PREFIX "/opt/homebrew")  # default for Apple Silicon
endif()
list(APPEND CMAKE_PREFIX_PATH "${HBREW_PREFIX}/lib/cmake/yaml-cpp")

# Find libcurl for HTTP communication
find_package(CURL REQUIRED)

# Find yaml-cpp via its config file
find_package(yaml-cpp REQUIRED CONFIG)

configure_file(
        ${CMAKE_SOURCE_DIR}/config.yaml
        ${CMAKE_BINARY_DIR}/config.yaml
        COPYONLY
)

# Define our executable
add_executable(ticket_bot main.cpp)

# Add include paths for header-only libraries
# webdriverxx headers (installed via `make install`) live under /usr/local/include or Homebrew prefix
target_include_directories(ticket_bot PRIVATE
        /usr/local/include/webdriverxx       # default for manual installs
        /opt/homebrew/include     # default for Homebrew on Apple Silicon
)

# Link against libcurl and yaml-cpp
target_link_libraries(ticket_bot PRIVATE
        CURL::libcurl
        yaml-cpp::yaml-cpp
)