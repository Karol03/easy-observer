cmake_minimum_required(VERSION 3.5)

project(easyobserver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -pedantic")

add_executable(easyobserver
    main.cpp

    eob/event.hpp
    eob/isubscription.hpp
    eob/subscriber.hpp
    eob/thread.hpp
    eob/threadwrapper.hpp
    eob/tunnel.hpp
)
