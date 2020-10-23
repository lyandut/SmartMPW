cmake_minimum_required(VERSION 3.17)
project(SmartMPW)

set(CMAKE_CXX_STANDARD 14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

set(Data
        SmartMPW/Data.hpp
        SmartMPW/Instance.hpp)

set(Algorithm
        SmartMPW/AdaptSelect.hpp
        SmartMPW/MpwBinPack.hpp)

set(Utils
        SmartMPW/Config.hpp
        SmartMPW/Utils.hpp
        SmartMPW/Visualizer.hpp)

add_executable(SmartMPW
        SmartMPW/Main.cpp
        ${Data} ${Algorithm} ${Utils})

set(BOOST_ROOT $ENV{BOOST_ROOT})

find_package(Boost REQUIRED)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})

    MESSAGE( STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}.")
    MESSAGE( STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}.")
    MESSAGE( STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}.")
endif()