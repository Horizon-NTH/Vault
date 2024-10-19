cmake_minimum_required(VERSION 3.12)

if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan_provider.cmake")
	message(STATUS "Downloading conan_provider.cmake from Conan repository")
	file(DOWNLOAD
		"https://raw.githubusercontent.com/conan-io/cmake-conan/develop2/conan_provider.cmake"
		${CMAKE_BINARY_DIR}/conan_provider.cmake)
endif ()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CONAN_BUILD_PROFILE "default;auto-cmake")
set(CONAN_HOST_PROFILE "default;auto-cmake")
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif ()
set(CMAKE_PROJECT_TOP_LEVEL_INCLUDES "${CMAKE_BINARY_DIR}/conan_provider.cmake")

project(vault VERSION 3.1)

option(ENABLE_TESTS "Enable testing" OFF)

set(HEADER_FILES
	include/Node.h
	include/File.h
	include/Directory.h
	include/Vault.h
	include/Application.h
	include/EncryptionManager.h
	include/VaultManager.h
	include/Utils.h
	include/CompressionManager.h
)

set(SOURCE_FILES
	src/Node.cpp
	src/File.cpp
	src/Directory.cpp
	src/Vault.cpp
	src/Application.cpp
	src/EncryptionManager.cpp
	src/VaultManager.cpp
	src/Utils.cpp
	src/CompressionManager.cpp
)

find_package(Botan 3.5.0 REQUIRED)
find_package(CLI11 2.4.2 REQUIRED)
find_package(pugixml 1.12.1 REQUIRED)
find_package(ZLIB 1.3.1 REQUIRED)
set(DEPS CLI11::CLI11 botan::botan pugixml::pugixml ZLIB::ZLIB)

set(PROJECT_LIB ${PROJECT_NAME}_lib)

add_library(${PROJECT_LIB} STATIC ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${PROJECT_LIB} PUBLIC include)
target_compile_features(${PROJECT_LIB} PUBLIC cxx_std_20)
target_compile_definitions(${PROJECT_LIB} PUBLIC PROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_LIB} ${DEPS})

add_executable(${PROJECT_NAME} src/Main.cpp)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} ${PROJECT_LIB} ${DEPS})

if (ENABLE_TESTS)
	add_subdirectory(tests)
endif ()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)