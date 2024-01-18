cmake_minimum_required(VERSION 3.28)

project( BING )

find_package( OpenCV REQUIRED )
find_package( ZeroMQ REQUIRED )
find_package( cppzmq REQUIRED )

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    "/Applications/MATLAB/MATLAB_Runtime/R2023b/extern/include")

set(HEADERS
    FES.h
    facedetection/facedetectcnn.h
)

set(SOURCES
    FES.cpp
    main.cpp
    facedetection/facedetectcnn.cpp
    facedetection/facedetectcnn-data.cpp
    facedetection/facedetectcnn-model.cpp
)

add_executable( BING ${HEADERS} ${SOURCES} )

target_link_libraries(
    BING
    cppzmq
    ${OpenCV_LIBS})