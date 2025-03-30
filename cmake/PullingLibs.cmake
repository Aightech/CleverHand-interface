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