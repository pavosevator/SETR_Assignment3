cmake_minimum_required(VERSION 3.10)

# Set project
project(CommandProcessing_Project VERSION 1.0 LANGUAGES C)

# Set C complier
set(CMAKE_C_COMPILER "gcc")

# Add CmdProc and Unity folder to the build
add_subdirectory(CmdProc)
add_subdirectory(Unity)

# Generate variables with set of libs and include folders
# Dependencies are automatically set
# Divide the libraries for testing and app purpose
set (APP_LIBS CmdProcLib)
set (TEST_LIBS CmdProcLib UnityLib)

list(APPEND EXTRA_INCLUDES "${CMAKE_SOURCE_DIR}/CmdProc")
list(APPEND EXTRA_INCLUDES "${CMAKE_SOURCE_DIR}/Unity")

# set source folder variable and add it in directories 
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
include_directories(${SRC_DIR})

# collect all source files from from source folder 
set(SOURCES ${SRC_DIR}/main.c)

#set the executable
add_executable(cmdproc ${SOURCES})

##############################################################################
# Add test executable
set(TEST_DIR ${CMAKE_SOURCE_DIR}/test)
set(TEST_SOURCES ${TEST_DIR}/test_1.c)

add_executable(runTests ${TEST_SOURCES})

# Link the test executable with the necessary libraries
target_link_libraries(runTests PUBLIC ${TEST_LIBS})

# Specifies include directories to use when compiling a given target
target_include_directories(runTests PUBLIC ${EXTRA_INCLUDES})

# Add a test target to run the tests
enable_testing()
add_test(NAME AllTests COMMAND runTests)
##############################################################################

# Specify libraries or flags to use when linking a given target and/or its dependents
target_link_libraries(cmdproc PUBLIC ${APP_LIBS})

# Specifies include directories to use when compiling a given target
target_include_directories(cmdproc PUBLIC ${EXTRA_INCLUDES})

#set target compile options
target_compile_options(cmdproc PRIVATE -Wall -Wextra -Wpedantic)
