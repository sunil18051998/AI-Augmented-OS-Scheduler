cmake_minimum_required(VERSION 3.15)
project(AI_Augmented_Scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Adjust this if you put ONNX Runtime somewhere else
include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(test_predictor 
    src/main.cpp
    src/onnx_predictor.cpp
    src/process.cpp
)

target_link_libraries(test_predictor PRIVATE onnxruntime)
target_include_directories(test_predictor PRIVATE src)

set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/third_party/onnxruntime")

target_include_directories(test_predictor PRIVATE
  ${ONNXRUNTIME_DIR}/include
)

target_link_libraries(test_predictor
  ${ONNXRUNTIME_DIR}/lib/onnxruntime.lib
)

add_custom_command(TARGET test_predictor POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${ONNXRUNTIME_DIR}/bin/onnxruntime.dll
    $<TARGET_FILE_DIR:test_predictor>
)
