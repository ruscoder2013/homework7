cmake_minimum_required(VERSION 3.5)

set(BUILD_NUMBER 0)
project(bayan  VERSION 7.0.${BUILD_NUMBER})
FIND_PACKAGE( Boost COMPONENTS program_options filesystem REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

add_executable(bayan main.cpp)

set_target_properties(bayan PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)

target_link_libraries(bayan
${Boost_LIBRARIES}
  )

if (MSVC)
    target_compile_options(
            bayan PRIVATE
            /W4 -lboost_system
    )
else ()
    target_compile_options(
            bayan PRIVATE
            -Wall -Wextra -pedantic -lboost_system#-Werror
    )
endif()

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT example@example.com)

include(CPack)
