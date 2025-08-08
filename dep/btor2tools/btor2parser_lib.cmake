set(patch "${CMAKE_CURRENT_LIST_DIR}/btor2tools.patch")
include(FetchContent)
FetchContent_Declare(
  btor2tools
  GIT_REPOSITORY https://github.com/hwmcc/btor2tools.git
  PATCH_COMMAND git apply --reverse ${patch} || true
        COMMAND git apply ${patch}
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build as shared library")
FetchContent_MakeAvailable(btor2tools)
get_target_property(btor2parser_include_dirs btor2parser INCLUDE_DIRECTORIES)
target_include_directories(btor2parser PUBLIC ${btor2parser_include_dirs}/btor2parser)
