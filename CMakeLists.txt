cmake_minimum_required(VERSION 3.3...3.5)
###### Author: Alexis Devillard
###### Date:   25/05/2022
###### Brief: This CmakeLists.txt file enable to build generic project with the following structure:
###### ┌─────────┐                        ┌────────────────────┐
###### │ project │                        │ CMakeList.txt      │
###### └─┬───────┘                     ┌──┤ CMakeConfig.txt    │
######   │                             │  │ CmakeFunctions.txt │
######   ├───────────┬───────┬──────┬──┘  └────────────────────┘
######   │           │       │      │
######   │      ┌────┴────┐  │   ┌──┴──┐     ┌─────────────┐
######   │      │ include │  │   │ lib ├──┬──┤ subproject1 │
######   │      └────┬────┘  │   └─────┘  │  └──────┬──────┘
######   │           │       │            │         │
######   │      ┌────┴────┐  │    modules │     ┌───┴──────────────┐
######   │      │ lib.hpp │  │    for the │     │                  │
######   │      └─────────┘  │    library │  ┌──┴──┐          ┌────┴────┐
######   │                   │    target  │  │ src │          │ include │
######  ┌┴────┐              │            │  └─┬───┘          └────┬────┘
######  │ src │              │            │    │                   │
######  └─┬───┘              │            │    │ ┌──────────┐ ┌────┴────┐
######    │                  │            │    ├─┤ main.cpp │ │ lib.hpp │
######    │ ┌──────────┐     │            │    │ └──────────┘ └─────────┘
######    ├─┤ main.cpp │     │            │    │
######    │ └──────────┘     │            │    │ ┌─────────┐
######    │                  │            │    └─┤ lib.cpp │
######    │ ┌─────────┐      │            │      └─────────┘
######    └─┤ lib.cpp │      │            │
######      └─────────┘      │            │  ┌───────────────┐
######                       │            └──┤ subproject ...│
######                       │               └───────────────┘
######                       │
######                      ┌┴─────────┐     ┌─────────────┐
######                      │ tool_lib ├──┬──┤ subproject1 │
######                      └──────────┘  │  └──────┬──────┘
######                                    │         │
######                          modules   │     ┌───┴──────────────┐
######                          for the   │     │                  │
######                          exe file  │  ┌──┴──┐          ┌────┴────┐
######                          target    │  │ src │          │ include │
######                                    │  └─┬───┘          └────┬────┘
######                                    │    │                   │
######                                    │    │ ┌──────────┐ ┌────┴────┐
######                                    │    ├─┤ main.cpp │ │ lib.hpp │
######                                    │    │ └──────────┘ └─────────┘
######                                    │    │
######                                    │    │ ┌─────────┐
######                                    │    └─┤ lib.cpp │
######                                    │      └─────────┘
######                                    │
######                                    │  ┌───────────────┐
######                                    └──┤ subproject ...│
######                                       └───────────────┘

########### PROJECT NAME AND VERSION ######
include(CMakeConfig.cmake)
project(${EXEC_NAME} VERSION ${PROJECT_VERSION})
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_PYTHON "Build python wrapper" OFF)

###### LIBRARY NAME ######
set(LIB_NAME ${EXEC_NAME}.${PROJECT_VERSION})


###### include helper functions ######
include(cmake/ColorLogs.cmake)
include(cmake/CMakeFunctions.cmake)


###### Check if the project is already built ######
if(TARGET ${EXEC_NAME}) #check if the target is already built
  message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset} is already built")
elseif(TARGET ${LIB_NAME})
  message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${LIB_NAME}${ColourReset} is already built")
else() #if not, build the target

  #display the compiler user and its version
  message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset}: Compiler: ${ColourBold}${CMAKE_CXX_COMPILER_ID}${ColourReset} ${CMAKE_CXX_COMPILER_VERSION}")
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version OUTPUT_VARIABLE COMPILER_VERSION)

  ###### Get the sources and headers ######
  ###### Exec main src file ######
  file(GLOB MAIN_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/main*")
  ###### Exec-only header file ######
  set(EXTRA_HEADERS "include/gnuplot-iostream.h")

  ###### Library src and headezr files (everything else) ######
  file(GLOB LIB_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "src/*.c")
  list(REMOVE_ITEM LIB_SRCS ${MAIN_FILES})
  file(GLOB LIB_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h" "include/*.hpp")
  list(REMOVE_ITEM LIB_HEADERS ${EXTRA_HEADERS})


  ###### check for internet ######
  include(cmake/ConnectionTest.cmake)

  ###### Search for Subdirectories/Libraries ######
  include(cmake/PullingLibs.cmake)

  ###### Check if the python wrapper is needed ########
  # Note: the python wrapper is built based on the _py.* files in the src folder
  include(cmake/PythonWrapper.cmake)

  ###### Create the shared library ######
  add_library(${LIB_NAME} SHARED ${LIB_SRCS}) #using version number
  target_link_libraries(${LIB_NAME}
    PUBLIC ${EXTRA_LIBS})
  target_include_directories(${LIB_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

  ###### Create the staticlibrary ######
  add_library(${PROJECT_NAME} STATIC ${LIB_SRCS})
  foreach(lib ${EXTRA_LIBS}) # link the subprojects static libraries
    string(REGEX REPLACE "\\.[0-9]+\\.[0-9]+" "" lib ${lib}) #remove the version number
    list(APPEND EXTRA_LIBS_STATIC ${lib})
  endforeach()
  target_link_libraries(${PROJECT_NAME}
    PRIVATE ${EXTRA_LIBS_STATIC})
  target_include_directories(${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)


  ###### Create the example only if examples are enabled and buildind from the root ######
  # Note: the examples are built based on the main_*.cpp files in the src folder
  include(cmake/BuildExamples.cmake)

  ###### Installing the library ########
  include(cmake/InstallingGeneral.cmake)

  ###### Create package ######
  set(CPACK_PACKAGE_NAME "adev-${PROJECT_NAME}"
    CACHE STRING "The resulting package name"
  )
  include(cmake/Packing.cmake)

endif()



