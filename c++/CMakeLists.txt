cmake_minimum_required(VERSION 3.14)
project(html_case_corrector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(GTest REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GUMBO REQUIRED gumbo)

# Add library
add_library(html_case_corrector
    src/HtmlTestCorrector.cpp
)

target_include_directories(html_case_corrector
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${GUMBO_INCLUDE_DIRS}
)

target_link_libraries(html_case_corrector
    PUBLIC
        ${GUMBO_LIBRARIES}
)

# Add executable
add_executable(html_case_corrector_app
    src/main.cpp
)

target_link_libraries(html_case_corrector_app
    PRIVATE
        html_case_corrector
)

# Add tests
enable_testing()

add_executable(html_case_corrector_test
    tests/HtmlCaseCorrectorTest.cpp
)

target_link_libraries(html_case_corrector_test
    PRIVATE
        html_case_corrector
        GTest::GTest
        GTest::Main
)

include(GoogleTest)
gtest_discover_tests(html_case_corrector_test)
