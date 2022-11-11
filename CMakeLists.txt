cmake_minimum_required(VERSION 3.10)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

project(vina-mpi-batch VERSION 1.1.2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(vina-mpi-batch vina-mpi.cc)
add_subdirectory(autodock-vina)
install(TARGETS vina-mpi-batch DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)

target_include_directories(vina-mpi-batch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vina-mpi)
target_sources(vina-mpi-batch PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/vina-mpi/serializers/string_serializer.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vina-mpi/arguments/arguments.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vina-mpi/util/util.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vina-mpi/io/io.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vina-mpi/io/logger.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/vina-mpi/vina_util.cc
)

find_package(Boost 1.65.0 COMPONENTS program_options) 
if(Boost_FOUND)
    target_include_directories(vina-mpi-batch PRIVATE ${Boost_INCLUDE_DIRS}) 
    target_link_libraries(vina-mpi-batch ${Boost_LIBRARIES})
endif()

find_package(MPI REQUIRED)
if(MPI_FOUND)
    set(CMAKE_CXX_COMPILER ${MPI_CXX_COMPILER})
    target_link_libraries(vina-mpi-batch ${MPI_C_LIBRARIES})
endif()

target_include_directories(vina-mpi-batch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/autodock-vina)
target_include_directories(vina-mpi-batch PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/autodock-vina/src/lib)
target_link_libraries(vina-mpi-batch vina)
