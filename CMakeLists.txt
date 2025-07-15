cmake_minimum_required(VERSION 3.14)
project(ArtemisExercise)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCE_FILES
        src/WordleGame.cpp
        src/WordleSolver.cpp
        src/FeedbackStrategy.cpp
)
# Library
add_library(assignment ${SOURCE_FILES})
target_include_directories(assignment PUBLIC include data)

include_directories(include)

# Main executable
add_executable(assignment_main src/Main.cpp)
target_link_libraries(assignment_main assignment)



##########################################################
### Um die Tests lokal auszuführen, Kommentare vor den folgenden Zeilen entfernen
##########################################################

#[[
include(FetchContent)
FetchContent_Declare(
        catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.4
)
FetchContent_MakeAvailable(catch2)


add_executable(assignment_tests ${SOURCE_FILES}
        tests/WordleSolverTests.cpp
        tests/WordleGameTests.cpp
)
target_link_libraries(assignment_tests Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(assignment_tests)
enable_testing()
include(CTest)
]]