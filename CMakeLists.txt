
cmake_minimum_required(VERSION 3.20)
project(ArchiveManager)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBZIP REQUIRED libzip)

# wxWidgets configuration
execute_process(
        COMMAND wx-config --cxxflags
        OUTPUT_VARIABLE wxWidgets_CXX_FLAGS
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND wx-config --libs
        OUTPUT_VARIABLE wxWidgets_LIBRARIES
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Add the wxWidgets flags to the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${wxWidgets_CXX_FLAGS}")

# Add executable
add_executable(ArchiveManager
        main.cpp
        EnhancedZipPanel.cpp
        EnhancedZipPanel.h
        EnhancedUnZipPanel.cpp
        EnhancedUnZipPanel.h
        PathOptimizer.h
)

# Include directories
target_include_directories(ArchiveManager PRIVATE
        ${LIBZIP_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}  # Add this to find local header files
)

# Link libraries
target_link_libraries(ArchiveManager PRIVATE
        ${wxWidgets_LIBRARIES}
        ${LIBZIP_LIBRARIES}
)

# Set output directories
set_target_properties(ArchiveManager PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)