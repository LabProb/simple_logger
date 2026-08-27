cmake_minimum_required(VERSION 3.16)
project(simple_logger VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------- Sources ----------------

set(LOG_SOURCES
    src/log.cpp
)

# ---------------- Library ----------------

# Тип бібліотеки визначається через BUILD_SHARED_LIBS
add_library(simple_logger ${LOG_SOURCES})

# Для shared бібліотек (і безпечно для static)
set_target_properties(simple_logger PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Include directories
target_include_directories(simple_logger
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(simple_logger PUBLIC cxx_std_20)
add_library(simple_logger::simple_logger ALIAS simple_logger)

# Dependencies
find_package(Threads REQUIRED)
target_link_libraries(simple_logger PUBLIC Threads::Threads)

# ---------------- Example executable ----------------

add_executable(Logger examples/main.cpp)
target_link_libraries(Logger PRIVATE simple_logger)

# Install executable
install(TARGETS Logger
        RUNTIME DESTINATION bin
        COMPONENT runtime)

# ---------------- Installation / Export ----------------

# Install library + headers
install(TARGETS simple_logger
        EXPORT simple_loggerTargets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(DIRECTORY include/ DESTINATION include)

# Export targets for find_package
install(EXPORT simple_loggerTargets
        FILE simple_loggerTargets.cmake
        NAMESPACE simple_logger::
        DESTINATION lib/cmake/simple_logger)

# ---------------- Package config ----------------

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/simple_loggerConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/simple_loggerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/simple_loggerConfig.cmake"
    INSTALL_DESTINATION lib/cmake/simple_logger
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/simple_loggerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/simple_loggerConfigVersion.cmake"
    DESTINATION lib/cmake/simple_logger)

include(CTest)
if(BUILD_TESTING)
    add_executable(simple_logger_tests tests/logger_tests.cpp)
    target_link_libraries(simple_logger_tests PRIVATE simple_logger)
    add_test(NAME simple_logger_tests COMMAND simple_logger_tests)
endif()
