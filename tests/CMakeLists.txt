set(PROJECT_NAME easyobserver-tests)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/framework)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/framework)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(${PROJECT_NAME}
    tests_doubleendedlinkedlist.cpp
    tests_single_thread_notifier.cpp
    tests_multi_thread_notifier.cpp
)

target_link_libraries(${PROJECT_NAME}
    easyobserver
    catch2
)
