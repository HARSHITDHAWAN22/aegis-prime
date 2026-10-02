add_library(aegis_kv INTERFACE)
target_include_directories(aegis_kv INTERFACE include)

add_executable(engine_tests tests/smoke_test.cpp)
target_link_libraries(engine_tests PRIVATE aegis_kv gtest_main)

include(GoogleTest)
gtest_discover_tests(engine_tests)