cmake_minimum_required(VERSION 3.5)

project(bulk)
find_package(Threads)
add_executable(bulk main.cpp)

set_target_properties(bulk PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

target_link_libraries(bulk
  ${CMAKE_THREAD_LIBS_INIT}
  )

if (MSVC)
    target_compile_options(
            bulk PRIVATE
            /W4
    )
else ()
    target_compile_options(
            bulk PRIVATE
            -Wall -Wextra -pedantic #-Werror
    )
endif()

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)
