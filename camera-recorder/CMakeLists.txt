cmake_minimum_required(VERSION 3.28)

project(Recorder )

find_package( OpenCV REQUIRED )
find_package( ZeroMQ REQUIRED )
find_package( cppzmq REQUIRED )

include_directories(
    ${OpenCV_INCLUDE_DIRS})

set(HEADERS
)

set(SOURCES
    main.cpp
)

add_executable( Recorder ${HEADERS} ${SOURCES} )

target_link_libraries(
    Recorder
    cppzmq
    ${OpenCV_LIBS})