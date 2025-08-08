include(ExternalProject)
set(BITWUZLA_GIT_TAG "main" CACHE STRING "Bitwuzla git commit hash or tag to checkout")
ExternalProject_Add(
  bitwuzla
  GIT_REPOSITORY https://github.com/bitwuzla/bitwuzla.git
  GIT_TAG ${BITWUZLA_GIT_TAG}
  BUILD_IN_SOURCE 1
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND meson setup build -Dbuildtype=release
  BUILD_COMMAND meson compile -C build
  INSTALL_COMMAND cp build/src/main/bitwuzla ${CMAKE_CURRENT_BINARY_DIR})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bitwuzla TYPE BIN)
