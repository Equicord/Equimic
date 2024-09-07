cmake_minimum_required(VERSION 3.21)
project(equimic LANGUAGES CXX VERSION 6.1.0)

# --------------------------------------------------------------------------------------------------------
# Library options
# --------------------------------------------------------------------------------------------------------

option(equimic_addon         "Build as addon"                            OFF)
option(equimic_server        "Build as rest server"                       ON)
option(equimic_prefer_remote "Prefer remote packages over local packages" ON)
option(equimic_static_cxx    "Static link libstdc++ and libgcc"          OFF)

# --------------------------------------------------------------------------------------------------------
# Addon and Rest-Server are mutually exclusive
# --------------------------------------------------------------------------------------------------------

if (equimic_addon)
  set(equimic_server OFF)
endif()

if (equimic_server)
  set(equimic_addon OFF)
endif()

# --------------------------------------------------------------------------------------------------------
# Sync `CPM_DOWNLOAD_ALL` with `equimic_prefer_remote`
# --------------------------------------------------------------------------------------------------------

if (equimic_prefer_remote)
    message(STATUS "[equimic] Avoiding local packages as 'equimic_prefer_remote' is ON")
endif()

set(CPM_DOWNLOAD_ALL ${equimic_prefer_remote})

# --------------------------------------------------------------------------------------------------------
# CMake options
# --------------------------------------------------------------------------------------------------------

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --------------------------------------------------------------------------------------------------------
# Create library
# --------------------------------------------------------------------------------------------------------

add_library(${PROJECT_NAME})
add_library(equicord::equimic ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF CXX_STANDARD_REQUIRED ON)

if (PROJECT_IS_TOP_LEVEL)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror -pedantic -pedantic-errors -Wfatal-errors)
endif()

if (equimic_static_cxx)
  target_link_libraries(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++)
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  target_compile_options(${PROJECT_NAME} PUBLIC -Wno-attributes=vc::)
else()
  target_compile_options(${PROJECT_NAME} PUBLIC -Wno-unknown-attributes)
endif()

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-missing-field-initializers -Wno-cast-function-type)

# --------------------------------------------------------------------------------------------------------
# Add source files
# --------------------------------------------------------------------------------------------------------

file(GLOB src "src/*.cpp")
target_sources(${PROJECT_NAME} PRIVATE ${src})

# --------------------------------------------------------------------------------------------------------
# Include "include" folder
# --------------------------------------------------------------------------------------------------------

target_include_directories(${PROJECT_NAME} PUBLIC  "include")
target_include_directories(${PROJECT_NAME} PRIVATE "include/equicord" "private")

# --------------------------------------------------------------------------------------------------------
# Setup compile definitions
# --------------------------------------------------------------------------------------------------------

target_compile_definitions(${PROJECT_NAME} PUBLIC equimic_VERSION="${PROJECT_VERSION}")

# --------------------------------------------------------------------------------------------------------
# Setup Dependencies
# --------------------------------------------------------------------------------------------------------

include("cmake/cpm.cmake")

CPMFindPackage(
  NAME           rohrkabel
  VERSION        7.0
  GIT_REPOSITORY "https://github.com/Curve/rohrkabel"
)

CPMFindPackage(
  NAME           tl-expected
  VERSION        1.1.0
  GIT_REPOSITORY "https://github.com/TartanLlama/expected"
)

CPMFindPackage(
  NAME           channel
  VERSION        2.3
  GIT_REPOSITORY "https://github.com/Curve/channel"
)

CPMFindPackage(
  NAME           range-v3
  GIT_TAG        0.12.0
  GIT_REPOSITORY "https://github.com/ericniebler/range-v3"
)

CPMFindPackage(
  NAME           glaze
  VERSION        2.6.8
  GIT_REPOSITORY "https://github.com/stephenberry/glaze"
)

CPMFindPackage(
  NAME           spdlog
  VERSION        1.14.1
  GIT_REPOSITORY "https://github.com/gabime/spdlog"
)

target_link_libraries(${PROJECT_NAME} PUBLIC cr::rohrkabel tl::expected cr::channel glaze::glaze range-v3::meta spdlog::spdlog)

# --------------------------------------------------------------------------------------------------------
# Custom Find-Package configurations
# --------------------------------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# --------------------------------------------------------------------------------------------------------
# Link PulseAudio
# --------------------------------------------------------------------------------------------------------

find_package(PulseAudio)
target_link_libraries(${PROJECT_NAME} PUBLIC PulseAudio::PulseAudio)

# --------------------------------------------------------------------------------------------------------
# Setup Rest Server
# --------------------------------------------------------------------------------------------------------

if (equimic_server)
  add_subdirectory(server)
endif()

# --------------------------------------------------------------------------------------------------------
# Setup Node Addon
# --------------------------------------------------------------------------------------------------------

if (equimic_addon AND NOT CMAKE_JS_VERSION)
  message(FATAL_ERROR "[equimic] Please build the addon using CMake.js")
endif()

if (equimic_addon)
  add_subdirectory(addon)
endif()
