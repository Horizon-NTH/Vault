enable_testing()

set(gtest_force_shared_crt ON)
include(FetchContent)
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
	DOWNLOAD_EXTRACT_TIMESTAMP 1620000000
)
FetchContent_MakeAvailable(googletest)

include_directories(${CMAKE_SOURCE_DIR}/include)

set(TEST_SOURCES
	src/Base64Test.cpp
	src/VaultTest.cpp
	src/ArgumentsParserTest.cpp
	src/ApplicationTest.cpp
)

add_executable(runTests ${TEST_SOURCES})

target_compile_features(runTests PUBLIC cxx_std_20)
target_link_libraries(runTests gtest gtest_main ${PROJECT_LIB})

include(GoogleTest)
gtest_discover_tests(runTests)