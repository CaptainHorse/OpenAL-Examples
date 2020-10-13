project(OpenAL-Examples)

# Earlier versions should work just fine
cmake_minimum_required(VERSION 3.15)

# Using C++17 but earlier standards should work just fine
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add OpenAL-Soft as a submodule
add_subdirectory("Submodules/openal-soft")

# Create executable of wav_play example
add_executable(wav_play "Examples/wav_play.cpp" "Examples/common.hpp")

# Include submodule and source headers
target_include_directories(wav_play PRIVATE "Examples/" "Submodules/openal-soft/include/" "Submodules/dr_libs/")

# Link OpenAL-Soft
target_link_libraries(wav_play OpenAL)