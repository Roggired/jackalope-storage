project(tests)
include(../commons.cmake)

set(page_test_source page_tests.cpp)
add_executable(page_unit_test ${page_test_source})
target_link_libraries(page_unit_test memory;model;${CONAN_LIBS})
add_test(NAME page_unit_test COMMAND $<TARGET_FILE:page_unit_test>)