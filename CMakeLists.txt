cmake_minimum_required(VERSION 3.12)

project(vault VERSION 0.0)

option(ENABLE_TESTS "Enable testing" OFF)

set(HEADER_FILES
	include/Base64.h
	include/Node.h
	include/File.h
	include/Directory.h
	include/Vault.h
	include/ArgumentsParser.h
	include/Application.h
)

set(SOURCE_FILES
	src/Base64.cpp
	src/Node.cpp
	src/File.cpp
	src/Directory.cpp
	src/Vault.cpp
	src/ArgumentsParser.cpp
	src/Application.cpp
)

set(PROJECT_LIB ${PROJECT_NAME}_lib)
include_directories(include)

add_library(${PROJECT_LIB} STATIC ${HEADER_FILES} ${SOURCE_FILES})

target_include_directories(${PROJECT_LIB} PUBLIC include)
target_compile_features(${PROJECT_LIB} PUBLIC cxx_std_20)

add_executable(${PROJECT_NAME} src/Main.cpp)

target_link_libraries(${PROJECT_NAME} ${PROJECT_LIB})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

if (ENABLE_TESTS)
	add_subdirectory(tests)
endif ()

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}
)