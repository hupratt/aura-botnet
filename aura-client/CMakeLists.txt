cmake_minimum_required(VERSION 3.0)
project (aura-client)

add_definitions(-DAURA_VERSION="0.1.2")

# Set default CMake build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# Download and include depedencies
set(DEPS_FOLDER "${CMAKE_CURRENT_SOURCE_DIR}/deps/")
include_directories(${DEPS_FOLDER})

# PicoSHA2
if(NOT EXISTS "${DEPS_FOLDER}/picosha2.h")
    file(DOWNLOAD "https://raw.githubusercontent.com/okdshin/PicoSHA2/master/picosha2.h" "${DEPS_FOLDER}/picosha2.h" SHOW_PROGRESS)
endif(NOT EXISTS "${DEPS_FOLDER}/picosha2.h")

# nlohmann/json
if(NOT EXISTS "${DEPS_FOLDER}/json.hpp")
    file(DOWNLOAD "https://github.com/nlohmann/json/releases/download/v3.1.2/json.hpp" "${DEPS_FOLDER}/json.hpp" SHOW_PROGRESS)
endif(NOT EXISTS "${DEPS_FOLDER}/json.hpp")

# Set libraries
set(LINUX_LIBS curl stdc++fs pthread)
set(WIN32_LIBS urlmon wininet)

# Set standard
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set executable
add_executable(${PROJECT_NAME}
    aura.cc
    request.cc
    util.cc
    bot.cc
    installer.cc
    authfile.cc
    sysinfo.cc
)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) # The commit hash for 1.9.0. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

if(UNIX AND NOT APPLE)
    add_definitions(-DCURL_STATICLIB)
    target_link_libraries(${PROJECT_NAME} ${LINUX_LIBS} cpr::cpr)
endif()

if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    target_link_libraries(${PROJECT_NAME} ${WIN32_LIBS})
endif()

