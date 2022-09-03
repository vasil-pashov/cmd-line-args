project(tests)

enable_testing()

set(SRC
    cpp/main.cpp
)

add_executable(${PROJECT_NAME} ${SRC})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} doctest::doctest cmd_line_args)
target_include_directories(${PROJECT_NAME} PRIVATE include)
doctest_discover_tests(${PROJECT_NAME})