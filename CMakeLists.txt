cmake_minimum_required(VERSION 3.19)
project(cerbotor VERSION 1.1.0)
set(CMAKE_CXX_STANDARD 23)
file(GLOB headers CONFIGURE_DEPENDS src/*.hpp)
file(GLOB sources CONFIGURE_DEPENDS src/*.cpp)
include(util.cmake)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(TOOLS "Build additional tools (SMT, fuzzer, scripts)" OFF)
option(STATIC "Build static binary" OFF)

add_executable(cerbotor ${sources})
target_compile_options(cerbotor PRIVATE -DGITID="${GIT_ID}")
target_compile_options(cerbotor PRIVATE -DVERSION="${PROJECT_VERSION}")
include(dep/btor2tools/btor2parser_lib.cmake)
target_link_libraries(cerbotor btor2parser)
install(TARGETS cerbotor)

if(TOOLS)
  include(dep/fuzzbtor2/fuzzbtor2_bin.cmake)
  include(dep/bitwuzla/bitwuzla_bin.cmake)
  set(BUILD_TOOLS ON)
  add_scripts(
    check_sat
    check_unsat
    check
    certified
    generate
    random
    fuzz
    fuzzer
    status)
else()
  set(BUILD_TOOLS OFF CACHE BOOL "Build btor2parser tools")
endif()

if(STATIC)
  set(BUILD_SHARED_LIBS OFF)
  set_target_properties(cerbotor PROPERTIES LINK_FLAGS "-static")
  set_target_properties(btorsim PROPERTIES LINK_FLAGS "-static")
endif()
