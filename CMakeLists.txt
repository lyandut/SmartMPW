cmake_minimum_required(VERSION 3.17)
project(SmartMPW)

set(CMAKE_CXX_STANDARD 11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")

set(Data
        SmartMPW/Data.hpp
        SmartMPW/Instance.hpp)

set(Algorithm
        SmartMPW/AdaptSelect.hpp
        SmartMPW/MpwBinPack.hpp)

set(Utils
        SmartMPW/Config.hpp
        SmartMPW/Utils.hpp)

add_executable(SmartMPW
        SmartMPW/Main.cpp
        ${Data} ${Algorithm} ${Utils})
