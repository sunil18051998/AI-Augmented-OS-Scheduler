cmake_minimum_required(VERSION 3.10)
project(ai_scheduler_core)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES main.cpp scheduler.cpp)

add_executable(ai_scheduler ${SOURCES})

# No external deps required; if you later add ONNX Runtime or JSON, link here.
# Example to link onnxruntime (illustrative only):
# find_library(ONNXRUNTIME_LIB onnxruntime HINTS /usr/local/lib)
# target_link_libraries(ai_scheduler PRIVATE ${ONNXRUNTIME_LIB})
