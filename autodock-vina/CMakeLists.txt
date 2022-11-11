cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(vina VERSION 1.1.2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "g++")

add_library(vina STATIC vina.cxx)
#add_executable(vina src/main/main.cpp)
#target_compile_options(vina PRIVATE -O3 -DNDEBUG)

target_include_directories(vina PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/lib)
target_sources(vina PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/pdb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/monte_carlo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/current_weights.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/everything.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/coords.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/parse_pdbqt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/random.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/quaternion.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/my_pid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/mutate.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/parallel_progress.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/cache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/quasi_newton.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/non_cache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/terms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/manifold.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/ssd.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/naive_non_cache.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/szv_grid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/model.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/grid.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/weighted_terms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/parallel_mc.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/std-out.cc
)

#add_definitions(-Dcout=vina_std_out -Dcerr=vina_std_err)

find_package(Boost 1.65.0 COMPONENTS system  thread  serialization  filesystem  program_options) 
if(Boost_FOUND)
    target_include_directories(vina PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(vina ${Boost_LIBRARIES})
endif()

target_link_libraries(vina pthread)
