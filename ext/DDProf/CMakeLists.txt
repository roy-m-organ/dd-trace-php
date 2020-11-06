cmake_minimum_required(VERSION 3.10)
project(DDProf LANGUAGES CXX)

add_library(DDProf INTERFACE)
target_compile_features(DDProf INTERFACE cxx_std_17)

target_include_directories(DDProf INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

#[[ We want to be able to use the namespaced name everywhere, including in this
    project's tests; this is a pattern described in the talk Effective CMake
    that allows it.
#]]
add_library(DDProf::DDProf ALIAS DDProf)

option(BUILD_TESTING "Enable tests" ON)
include(CTest)
if (${BUILD_TESTING})
    enable_testing()
    add_subdirectory(test)
endif()

