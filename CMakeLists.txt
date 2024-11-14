cmake_minimum_required(VERSION 3.5)

project(easyobserver-main LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Wall -Werror -static -pedantic")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -fno-inline -O0 -g -ggdb -Wall -pedantic -Werror -static -Wa,-mbig-obj")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/easy)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)

add_executable(easyobserver-main
    main.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/examples/simple_example.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/advanced_example.cpp
)

target_link_libraries(${PROJECT_NAME} easyobserver)

