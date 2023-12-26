cmake_minimum_required(VERSION 3.5)

project(bridge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(sqlite3 SHARED src/sqlite/sqlite3.c src/sqlite/sqlite3.h)

find_package(Boost COMPONENTS system program_options REQUIRED)
find_package(Threads)

include_directories(${Boost_INCLUDE_DIR})
include_directories(src)
include_directories(src/serial)
include_directories(src/logger)


file(GLOB HRDS
    src/*.h    
    src/logger/*.h
)

file(GLOB SRCS
    src/*.cpp
    src/serial/*.cpp
    src/logger/*.cpp
)

add_executable(${PROJECT_NAME}
    ${SRCS}
    ${HRDS}
)

target_link_libraries(${PROJECT_NAME}
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        sqlite3
        )


add_executable( device_emulator dev_emulator/emulator.cpp src/serial/TimeoutSerial.cpp src/metra_protocol.cpp src/dyn_array.cpp)

target_link_libraries(device_emulator
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )

