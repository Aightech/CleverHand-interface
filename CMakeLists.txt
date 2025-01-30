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
include(CMakeConfig.txt)
project(${EXEC_NAME} VERSION ${PROJECT_VERSION})
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_PYTHON "Build python wrapper" OFF)

###### LIBRARY NAME ######
set(LIB_NAME ${EXEC_NAME}.${PROJECT_VERSION})

if(NOT DEFINED DIR_DEPTH)
  set(DIR_DEPTH 0)
else()
  MATH(EXPR DIR_DEPTH "${DIR_DEPTH}+1")
endif()
string(REPEAT "\t" ${DIR_DEPTH} PREFIX_MSG)
MATH(EXPR color_code "${DIR_DEPTH}+33")
MATH(EXPR color_code2 "${DIR_DEPTH}+34")

###### include helper functions ######
include(CMakeFunctions.txt)

if(TARGET ${EXEC_NAME})
  # Do something when target found
  message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset} is already built")
elseif(TARGET ${LIB_NAME})
  message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${LIB_NAME}${ColourReset} is already built")
else()

  ###### C++ standard ######
  set(CMAKE_CXX_STANDARD 14)
  set(CMAKE_CXX_STANDARD_REQUIRED True)
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
  if(NOT DEFINED NO_CONNECTION)
    execute_process(COMMAND printf "-- Check internet connection: ${ColourSlowBlink}${Yellow}...${ColourReset}\\b\\b\\b")
    execute_process(
      COMMAND ping www.google.com -c 2 -w 1
      ERROR_QUIET
      OUTPUT_QUIET
      RESULT_VARIABLE NO_CONNECTION
    )
    if(NO_CONNECTION EQUAL 1)
      message(" ${Green}Online${ColourReset}")
    else()
      message(" ${Red}Offline${ColourReset}")
    endif()
  endif()


  ###### Search for Subdirectories/Libraries ######
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib")
    execute_process(COMMAND git submodule status
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE SUBMODULES_LIST)
    subdirlist(LIBS "${CMAKE_CURRENT_SOURCE_DIR}/lib/")
    foreach(subdir ${LIBS})
      #check if the library is a unused library
      set(IS_UNUSED 0)
      foreach(lib ${UNUSED_LIBS})
        string(FIND "${lib}" "${subdir}" FOUND)
        if(${FOUND} GREATER -1)
          set(IS_UNUSED 1)
        endif()
      endforeach()
      if(IS_UNUSED EQUAL 1)
        message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset}: ${ColourBold}${Esc}[${color_code2}m${subdir}${ColourReset} is an unused library")
        continue()
      endif()

      string(FIND "${SUBMODULES_LIST}" "${subdir}" FOUND)
      if(${FOUND} GREATER -1 AND NO_CONNECTION EQUAL 1)
        #git submodules init
        execute_process(COMMAND git submodule update --remote --merge --init -- "lib/${subdir}"
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE OUTPUT)
      endif()
      subproject_version("lib/${subdir}" subdir_version) #get the version
      message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset}: Adding library ${ColourBold}${Esc}[${color_code2}m${subdir}${ColourReset} version: ${ColourBold}${subdir_version}${ColourReset}")
      add_subdirectory("lib/${subdir}")

      get_directory_property(libname
        DIRECTORY "lib/${subdir}"
        DEFINITION LIB_NAME)
      list(APPEND EXTRA_LIBS "${libname}")
      link_directories(${CMAKE_SOURCE_DIR}/lib/${subdir}/bin)
    endforeach()
  endif()

  ###### Search for Subdirectories/Libraries ######
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tool_lib" AND BUILD_EXAMPLES AND DIR_DEPTH EQUAL 0)
    execute_process(COMMAND git submodule status
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      OUTPUT_VARIABLE SUBMODULES_LIST)
    subdirlist(LIBS "${CMAKE_CURRENT_SOURCE_DIR}/tool_lib/")
    foreach(subdir ${LIBS})
      #check if the library is a unused library
      set(IS_UNUSED 0)
      foreach(lib ${UNUSED_LIBS})
        string(FIND "${lib}" "${subdir}" FOUND)
        if(${FOUND} GREATER -1)
          set(IS_UNUSED 1)
        endif()
      endforeach()
      if(IS_UNUSED EQUAL 1)
        message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset}: ${ColourBold}${Esc}[${color_code2}m${subdir}${ColourReset} is an unused library")
        continue()
      endif()

      string(FIND "${SUBMODULES_LIST}" "${subdir}" FOUND)
      if(${FOUND} GREATER -1 AND NO_CONNECTION EQUAL 1)
        #git submodules init
        execute_process(COMMAND git submodule update --remote --merge --init -- "tool_lib/${subdir}"
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE OUTPUT)
      endif()
      subproject_version("tool_lib/${subdir}" subdir_version) #get the version
      message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset}: Adding ${ColourBold}tool${ColourReset} library ${ColourBold}${Esc}[${color_code2}m${subdir}${ColourReset} version: ${ColourBold}${subdir_version}${ColourReset}")
      add_subdirectory("tool_lib/${subdir}")

      get_directory_property(libname
        DIRECTORY "tool_lib/${subdir}"
        DEFINITION LIB_NAME)
      list(APPEND TOOL_LIBS "${libname}")
      link_directories(${CMAKE_SOURCE_DIR}/tool_lib/${subdir}/bin)
    endforeach()
  endif()

  #create the list of sources for the python wrapper
  set(LIB_SRCS_PY ${LIB_SRCS})

  #remove the python files from the list
  list(FILTER LIB_SRCS EXCLUDE REGEX "_py.*")

  ###### Create the library ######
  add_library(${LIB_NAME} SHARED ${LIB_SRCS})

  target_link_libraries(${LIB_NAME}
    PUBLIC ${EXTRA_LIBS})
  target_include_directories(${LIB_NAME}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  target_compile_options(${LIB_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)

  #check if there are _py.* files in the src folder 
  set(IS_PYTHON 0)
  foreach(file ${LIB_SRCS_PY})
    string(FIND "${file}" "_py" FOUND)
    if(${FOUND} GREATER -1)
      set(IS_PYTHON 1)
      message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset}: Python wrapper found: ${ColourBold}${Esc}[${Green}${file}${ColourReset}")
      # break()
    endif()
  endforeach()

  if(BUILD_PYTHON AND IS_PYTHON)
    # check if there is an extern folder and if pybind11 is present
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/extern/pybind11")
      #check if pybind11 submodule is initialized/updated (if the folder is empty)
      # Add pybind11 subdirectory
      execute_process(COMMAND git submodule update --remote --merge --init -- "extern/pybind11"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_VARIABLE OUTPUT)

      add_subdirectory(extern/pybind11)
      include_directories(${pybind11_INCLUDE_DIRS})

      # Find Python 3 libraries and development headers
      find_package(Python3 REQUIRED COMPONENTS Development)

      #test if the python is found
      if(NOT Python3_FOUND)
        message(FATAL_ERROR "Python 3 is required to build the python wrapper")
      endif()
      ###### LIBRARY NAME ######
      set(PYTHON_LIB_NAME pyclvhd)

      # Add pybind11 module
      pybind11_add_module(${PYTHON_LIB_NAME} ${LIB_SRCS_PY})

      # Link the necessary libraries
      target_link_libraries(${PYTHON_LIB_NAME}
        PUBLIC ${LIB_NAME}
        PRIVATE Python3::Python
      )
      target_include_directories(${PYTHON_LIB_NAME} # 
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include
      )
      #copy the resulting .so file to the script folder
      add_custom_command(TARGET ${PYTHON_LIB_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PYTHON_LIB_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}/script/${PYTHON_LIB_NAME}.so
        #add so in bin/ with os specific name
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PYTHON_LIB_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}/bin/
        COMMAND ${CMAKE_COMMAND} -E echo "       Python wrapper built in ${CMAKE_CURRENT_SOURCE_DIR}/scripts/"
      )
    else()
      # say that its ok but we can't build the python wrapper
      message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset}: ${ColourBold}${Red}pybind11${ColourReset} not found in extern folder")
    endif()
  endif()


  ###### Create the executable only if examples are enabled and buildind from the root ######
  if(BUILD_EXAMPLES AND DIR_DEPTH EQUAL 0)
    #for each main file
    foreach(MAIN_FILE ${MAIN_FILES})
      #the name of the executable is the name of the project + the main file (without the "main" and extension)
      get_filename_component(EXEC_NAME ${MAIN_FILE} NAME_WE)
      #replace main with the project name
      string(REPLACE "main" ${PROJECT_NAME} EXEC_NAME ${EXEC_NAME})
      message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset} will be built")
      add_executable(${EXEC_NAME} ${MAIN_FILE})
      target_link_libraries(${EXEC_NAME}
        PUBLIC ${LIB_NAME}
        PUBLIC ${TOOL_LIBS})
      target_include_directories(${EXEC_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    endforeach()
  endif()

  ###### Create the library ######
  add_library(${PROJECT_NAME} STATIC ${LIB_SRCS})

  # fill EXTRA_LIBS_STATIC with the EXTRA_LIBS without the version lib.major.minor
  foreach(lib ${EXTRA_LIBS})
    string(REGEX REPLACE "\\.[0-9]+\\.[0-9]+" "" lib ${lib})
    list(APPEND EXTRA_LIBS_STATIC ${lib})
  endforeach()

  target_link_libraries(${PROJECT_NAME}
    PRIVATE ${EXTRA_LIBS_STATIC})
  target_include_directories(${PROJECT_NAME}
    PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  )
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)


  ###### Create package ######
  # Install the library
  install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
  )

  # Install the headers
  install(DIRECTORY include/ DESTINATION include)

  # Export the targets to a script
  install(EXPORT ${PROJECT_NAME}
    FILE ${PROJECT_NAME}.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION lib/cmake/${PROJECT_NAME}
  )

  include(CMakePackageConfigHelpers)
  # Create a ConfigVersion.cmake file
  write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion # AnyNewerVersion, SameMajorVersion, SameMinorVersion, ExactVersion
  )

  set(PACKAGE_DEPENDENCIES "")
  foreach(pkg ${EXTRA_LIBS})
      string(REGEX REPLACE "\\.[0-9]+\\.[0-9]+" "" pkg ${pkg})
      string(APPEND PACKAGE_DEPENDENCIES "find_dependency(${pkg})\n")
  endforeach()

  # Create a Config.cmake file
  configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in" # input file
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" # output file
    INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
  )

  # Install the Config.cmake file
  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
    DESTINATION lib/cmake/${PROJECT_NAME}
  )




endif()



