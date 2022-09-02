project(tests)

enable_testing()

set(SRC
    cpp/main.cpp
)

add_executable(${PROJECT_NAME} ${SRC})
target_link_libraries(${PROJECT_NAME} doctest::doctest cmd_line_args)
target_include_directories(${PROJECT_NAME} PRIVATE include)
doctest_discover_tests(${PROJECT_NAME})