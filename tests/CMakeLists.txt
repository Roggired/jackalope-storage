project(tests)
include(../commons.cmake)

set(test_example_unit_test_source test-example.cpp)
add_executable(test_example_unit_test ${test_example_unit_test_source})
target_link_libraries(test_example_unit_test example;${CONAN_LIBS})
add_test(NAME test_example_unit_test COMMAND $<TARGET_FILE:test_example_unit_test>)