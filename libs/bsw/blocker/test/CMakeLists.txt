add_executable(blockerTest BlockerTest.cpp)

target_include_directories(blockerTest PRIVATE include)

target_link_libraries(
    blockerTest
    PRIVATE blocker
            asyncMockImpl
            gmock_main
            utilMock)

gtest_discover_tests(blockerTest PROPERTIES LABELS "blockerTest")
