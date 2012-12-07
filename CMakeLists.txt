cmake_minimum_required(VERSION 2.8)

project(AOFlagger)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-2.4>=2.18)
pkg_check_modules(SIGCXX sigc++-2.0)

find_library(CASA_MS_LIB casa_ms REQUIRED)
find_library(CASA_TABLES_LIB casa_tables REQUIRED)
find_library(CASA_CASA_LIB casa_casa REQUIRED)
find_library(CASA_MEASURES_LIB casa_measures REQUIRED)
set(CASA_LIBS ${CASA_MS_LIB} ${CASA_TABLES_LIB} ${CASA_CASA_LIB})
find_path(CASA_INCLUDE_DIR NAMES ms/MeasurementSets/MeasurementSet.h PATHS /usr/local/include/casacore PATH_SUFFIXES casacore)
find_path(FITSIO_INCLUDE_DIR NAMES fitsio.h)

#find_package(GSL)

find_package(LibXml2 REQUIRED)
find_package(PNG REQUIRED)
find_package(Boost REQUIRED COMPONENTS date_time thread filesystem system)
find_library(FFTW3_LIB fftw3 REQUIRED)
find_library(FITSIO_LIB cfitsio REQUIRED)
enable_language(Fortran OPTIONAL)
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  find_library(RT_LIBRARY rt)
#  list(APPEND LOFAR_EXTRA_LIBRARIES ${RT_LIBRARY})
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

include_directories(${CASA_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${FITSIO_INCLUDE_DIR})

add_subdirectory(src)

# add target to generate API documentation with Doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
add_custom_target(doc
	${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating API documentation with Doxygen" VERBATIM)

endif(DOXYGEN_FOUND)
