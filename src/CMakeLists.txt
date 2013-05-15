link_libraries(${Boost_THREAD_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY} ${Boost_DATE_TIME_LIBRARY} ${Boost_SIGNALS_LIBRARY})
link_libraries(${FFTW3_LIB})
link_libraries(${CASA_LIBS} ${CASA_MEASURES_LIB})
link_libraries(${LAPACK_lapack_LIBRARY})
link_libraries(${FITSIO_LIB})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -DNDEBUG -funroll-loops -O3")

include(CheckCXXCompilerFlag)

CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
CHECK_CXX_COMPILER_FLAG("-pedantic -Wno-long-long -Werror=vla" COMPILER_SUPPORTS_PEDANTIC)

# -march=native speeds up the flagger with about 6% - 7%, but is not supported prior gcc 4.2.
# Hence, only enable it if it is supported.
if(COMPILER_SUPPORTS_MARCH_NATIVE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
else()
	# We add -msse2 because it needs the sse2 instruction set to compile some files, and
	# this will force its inclusion in case the below -march=natve is not set.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2")
  message(STATUS " CXX compiler does not support -march=native : your code will not be optimized with -march=native.")
  message(STATUS " This probably means your gcc is old ( < 4.2).")
endif(COMPILER_SUPPORTS_MARCH_NATIVE)

if(COMPILER_SUPPORTS_PEDANTIC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wno-long-long -Werror=vla")
endif(COMPILER_SUPPORTS_PEDANTIC)

include(CheckIncludeFileCXX)

FIND_PATH(BOOST_ASIO_H_FOUND "boost/asio.hpp")
if(BOOST_ASIO_H_FOUND)
  message(STATUS "Boost ASIO library found.")
else()
  message(STATUS "Boost ASIO library not found, remote access of measurement sets disabled.")
endif(BOOST_ASIO_H_FOUND)

# To enable assembly .s files:
#enable_language(ASM-ATT)

if(GTKMM_FOUND)
  add_definitions(-DHAVE_GTKMM)
endif(GTKMM_FOUND)

if(GSL_FOUND)
  add_definitions(-DHAVE_GSL)
  link_libraries(${GSL_LIBRARIES})
endif(GSL_FOUND)

add_executable(rficonsole rficonsole.cpp)

add_executable(rfihistory rfihistory.cpp)

add_executable(aorefscript aorefscript.cpp)

set(GUI_QUALITY_FILES
  gui/quality/aoqplotwindow.cpp
  gui/quality/baselineplotpage.cpp
  gui/quality/datawindow.cpp
  gui/quality/grayscaleplotpage.cpp
  gui/quality/histogrampage.cpp
  gui/quality/timefrequencyplotpage.cpp
  gui/quality/twodimensionalplotpage.cpp)

set(GUI_PLOT_FILES
  gui/plot/colorscale.cpp
  gui/plot/horizontalplotscale.cpp
  gui/plot/plot2d.cpp
  gui/plot/plotpropertieswindow.cpp
  gui/plot/plotwidget.cpp
  gui/plot/title.cpp
  gui/plot/verticalplotscale.cpp)

set(GUI_FILES
  gui/controllers/rfiguicontroller.cpp
  gui/application.cpp
  gui/complexplaneplotwindow.cpp
  gui/editstrategywindow.cpp
  gui/gotowindow
  gui/highlightwindow.cpp
  gui/imagecomparisonwidget.cpp
  gui/imageplanewindow
  gui/imagepropertieswindow.cpp
  gui/imagewidget.cpp
  gui/msoptionwindow.cpp
  gui/noisestatoptionwindow.cpp
  gui/plotframe.cpp
  gui/plotwindow.cpp
  gui/progresswindow.cpp
  gui/rawoptionwindow.cpp
  gui/rfiguiwindow.cpp
  gui/strategywizardwindow.cpp
  gui/tfstatoptionwindow.cpp
  ${GUI_PLOT_FILES} ${GUI_QUALITY_FILES})

set(IMAGING_FILES
  imaging/uvimager.cpp
  imaging/model.cpp
  imaging/fourproductcorrelatortester.cpp)

set(INTERFACE_FILES
  interface/aoflagger.cpp)
  
set(MSIO_FILES
  msio/baselinematrixloader.cpp
  msio/baselinereader.cpp
  msio/colormap.cpp
  msio/directbaselinereader.cpp
  msio/fitsfile.cpp
  msio/image2d.cpp
  msio/indirectbaselinereader.cpp
  msio/mask2d.cpp
  msio/measurementset.cpp
  msio/memorybaselinereader.cpp
  msio/pngfile.cpp
  msio/rspreader.cpp
  msio/samplerow.cpp
  msio/segmentedimage.cpp
  msio/sortedtimestepaccessor.cpp
  msio/spatialtimeloader.cpp
  msio/stokesimager.cpp
  msio/timefrequencydata.cpp
  msio/timestepaccessor.cpp)
  
set(QUALITY_FILES
  quality/histogramcollection.cpp
  quality/histogramtablesformatter.cpp
	quality/rayleighfitter.cpp
  quality/qualitytablesformatter.cpp)

set(REMOTEAO_FILES
  remote/server.cpp)

set(STRATEGY_ACTION_FILES
  strategy/actions/baselineselectionaction.cpp
  strategy/actions/calibratepassbandaction.cpp
  strategy/actions/changeresolutionaction.cpp
  strategy/actions/foreachbaselineaction.cpp
  strategy/actions/foreachmsaction.cpp
  strategy/actions/frequencyselectionaction.cpp
  strategy/actions/fringestopaction.cpp
	strategy/actions/highpassfilteraction.cpp
  strategy/actions/imageraction.cpp
  strategy/actions/normalizevarianceaction.cpp
  strategy/actions/plotaction.cpp
  strategy/actions/slidingwindowfitaction.cpp
  strategy/actions/spatialcompositionaction.cpp
  strategy/actions/statisticalflagaction.cpp
  strategy/actions/strategyaction.cpp
  strategy/actions/svdaction.cpp
  strategy/actions/timeconvolutionaction.cpp
  strategy/actions/timeselectionaction.cpp
  strategy/actions/writeflagsaction.cpp)

set(STRATEGY_ALGORITHMS_FILES
  strategy/algorithms/baselineselector.cpp
	strategy/algorithms/baselinetimeplaneimager.cpp
  strategy/algorithms/eigenvalue.cpp
  strategy/algorithms/fringestoppingfitter.cpp
  strategy/algorithms/fringetestcreater.cpp
	strategy/algorithms/highpassfilter.cpp
  strategy/algorithms/localfitmethod.cpp
  strategy/algorithms/mitigationtester.cpp
  strategy/algorithms/morphology.cpp
  strategy/algorithms/rfistatistics.cpp
  strategy/algorithms/sinusfitter.cpp
  strategy/algorithms/statisticalflagger.cpp
  strategy/algorithms/sumthreshold.cpp
  strategy/algorithms/svdmitigater.cpp
  strategy/algorithms/thresholdconfig.cpp
  strategy/algorithms/thresholdmitigater.cpp
  strategy/algorithms/thresholdtools.cpp
  strategy/algorithms/timefrequencystatistics.cpp
  strategy/plots/antennaflagcountplot.cpp
  strategy/plots/frequencyflagcountplot.cpp)

if(GTKMM_FOUND)
set(STRATEGY_PLOTS_FILES
  strategy/plots/frequencypowerplot.cpp
  strategy/plots/iterationsplot.cpp
  strategy/plots/rfiplots.cpp
  strategy/plots/timeflagcountplot.cpp
  util/multiplot.cpp)
else()
 set(STRATEGY_PLOTS_FILES)
endif(GTKMM_FOUND)

set(STRATEGY_CONTROL_FILES
  strategy/control/actionblock.cpp
  strategy/control/actioncontainer.cpp
  strategy/control/actionfactory.cpp
  strategy/control/defaultstrategy.cpp
  strategy/control/strategyreader.cpp
  strategy/control/strategywriter.cpp)

set(STRATEGY_IMAGESETS_FILES
  strategy/imagesets/fitsimageset.cpp
  strategy/imagesets/imageset.cpp
  strategy/imagesets/msimageset.cpp
  strategy/imagesets/parmimageset.cpp
	strategy/imagesets/pngreader.cpp)

set(STRATEGY_FILES
  ${STRATEGY_ACTION_FILES}
  ${STRATEGY_ALGORITHMS_FILES}
  ${STRATEGY_CONTROL_FILES}
  ${STRATEGY_IMAGESETS_FILES}
  ${STRATEGY_PLOTS_FILES})

set(UTIL_FILES
  util/aologger.cpp
  util/compress.cpp
  util/ffttools.cpp
  util/integerdomain.cpp
  util/plot.cpp
  util/rng.cpp
  util/stopwatch.cpp)

if(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
	set(REMOTE_FILES
		remote/client.cpp
		remote/server.cpp
		remote/serverconnection.cpp
		remote/processcommander.cpp
		remote/clusteredobservation.cpp
		remote/vdsfile.cpp)
else()
	set(REMOTE_FILES
		remote/clusteredobservation.cpp)
endif(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)

add_library(aoflagger ${IMAGING_FILES} ${INTERFACE_FILES} ${MSIO_FILES} ${QUALITY_FILES} ${STRATEGY_FILES} ${UTIL_FILES})
target_link_libraries(aoflagger ${PNG_LIBRARIES} ${LIBXML2_LIBRARIES})
link_libraries(aoflagger)

if(GTKMM_FOUND)
  include_directories(${GTKMM_INCLUDE_DIRS})

	add_library(aoflaggergui ${GUI_FILES})
	add_library(aoflaggerremote ${REMOTE_FILES})

  link_directories(${GTKMM_LIBDIR})
  
  add_executable(rfigui rfigui.cpp)
  target_link_libraries(rfigui aoflaggergui ${GTKMM_LIBRARIES})
  
	if(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
		add_executable(aoqplot aoqplot.cpp)
		target_link_libraries(aoqplot aoflaggergui aoflaggerremote ${GTKMM_LIBRARIES})
		add_executable(badstations badstations.cpp)
		target_link_libraries(badstations aoflaggerremote ${GTKMM_LIBRARIES})
		add_executable(aofrequencyfilter aofrequencyfilter.cpp)
		target_link_libraries(aofrequencyfilter aoflaggerremote ${GTKMM_LIBRARIES})
	endif(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
else()
  message(WARNING " The graphical user interface library GTKMM was not found; rfigui and aoqplot will not be compiled.")
endif(GTKMM_FOUND)

add_executable(aoflagger-bin aoflagger.cpp)
set_target_properties(aoflagger-bin
  PROPERTIES OUTPUT_NAME aoflagger)

add_executable(rfistrategy rfistrategy.cpp)

add_executable(msinfo msinfo.cpp)

add_executable(colormapper colormapper.cpp)
# add_executable(imgzenith imgzenith.cpp imaging/zenithimager.cpp)

if(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND AND GTKMM_FOUND)
  add_executable(aoquality aoquality.cpp)
  target_link_libraries(aoquality aoflaggerremote ${SIGCXX_LIBRARIES})

	add_executable(aoremoteclient aoremoteclient.cpp)
  target_link_libraries(aoremoteclient aoflaggerremote ${SIGCXX_LIBRARIES})
endif(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)

add_executable(aotest EXCLUDE_FROM_ALL aotest.cpp)
add_test(aotest aotest)
add_custom_target(check COMMAND aotest DEPENDS aotest)

install (TARGETS rficonsole aoflagger-bin DESTINATION bin)
if(GTKMM_FOUND)
	install (TARGETS rfigui aoqplot DESTINATION bin)
endif(GTKMM_FOUND)

if(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)
	install (TARGETS aoquality aoremoteclient DESTINATION bin)
endif(BOOST_ASIO_H_FOUND AND SIGCXX_FOUND)

install (TARGETS aoflagger DESTINATION lib) 
install (FILES interface/aoflagger.h DESTINATION include)
