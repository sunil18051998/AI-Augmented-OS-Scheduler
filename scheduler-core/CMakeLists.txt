cmake_minimum_required(VERSION 3.20)
project(AI_Augmented_OS_Scheduler LANGUAGES CXX)

# Require C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ============================
# Add source files
# ============================
set(SRC_FILES
    src/main.cpp
    src/onnx_predictor.cpp
    src/process.cpp
    src/scheduler.cpp
)

# ============================
# Create executable
# ============================
add_executable(test_predictor ${SRC_FILES})

# ============================
# Include directories
# ============================
target_include_directories(test_predictor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/onnxruntime/include  # ONNX Runtime headers
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/rapidjson/
)

# ============================
# Link ONNX Runtime
# ============================
target_link_libraries(test_predictor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/onnxruntime/lib/onnxruntime.lib
)

# ============================
# Preprocessor definitions
# ============================
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

# ============================
# Optional: output directory
# ============================
set_target_properties(test_predictor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
