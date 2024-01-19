cmake_minimum_required(VERSION 3.28)

project( ROI-Detector )

find_package( OpenCV REQUIRED )
find_package( ZeroMQ REQUIRED )
find_package( cppzmq REQUIRED )

include_directories(
    ${OpenCV_INCLUDE_DIRS})

set(HEADERS
    fes/FES.h
    facedetection/facedetectcnn.h
    facedetection/facedetection.h
    cvsaliency/CVSaliency.h
    ROIDetector.h
)

set(SOURCES
    main.cpp
    fes/FES.cpp
    facedetection/facedetectcnn.cpp
    facedetection/facedetectcnn-data.cpp
    facedetection/facedetectcnn-model.cpp
    facedetection/facedetection.cpp
    cvsaliency/CVSaliency.cpp
)

add_executable( ROI-Detector ${HEADERS} ${SOURCES} )

target_link_libraries(
    ROI-Detector
    cppzmq
    ${OpenCV_LIBS})