cmake_minimum_required(VERSION 3.28)

project(Recorder )

find_package( OpenCV REQUIRED )

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
    ${OpenCV_LIBS})