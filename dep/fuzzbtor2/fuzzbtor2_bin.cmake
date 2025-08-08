include(ExternalProject)
set(FUZZBTOR2_GIT_TAG "main" CACHE STRING "FuzzBtor2 git commit hash or tag to checkout")
ExternalProject_Add(
  fuzzbtor2
  GIT_REPOSITORY https://github.com/CoriolisSP/FuzzBtor2
  GIT_TAG ${FUZZBTOR2_GIT_TAG}
  UPDATE_COMMAND ""
  PATCH_COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt .
  INSTALL_COMMAND cp fuzzbtor2 ${CMAKE_CURRENT_BINARY_DIR}
  CMAKE_ARGS -DSTATIC=${STATIC})
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/fuzzbtor2 TYPE BIN)
