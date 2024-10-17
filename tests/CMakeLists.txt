enable_testing()

set(gtest_force_shared_crt ON)
find_package(GTest 1.15.0 REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(TEST_SOURCES
	src/VaultTest.cpp
	src/ApplicationTest.cpp
	src/EncryptionManagerTest.cpp
	src/CompressionManagerTest.cpp
)

add_executable(runTests ${TEST_SOURCES})

target_compile_features(runTests PUBLIC cxx_std_20)
target_link_libraries(runTests GTest::gtest GTest::gmock GTest::gmock_main ${PROJECT_LIB})

include(GoogleTest)
gtest_discover_tests(runTests)