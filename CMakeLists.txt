cmake_minimum_required(VERSION 3.14)
project(seqpatternminer CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(spm
  src/subsequence.cpp
  src/sequential_pattern_miner.cpp
)
target_include_directories(spm PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
