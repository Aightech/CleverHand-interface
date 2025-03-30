set(LIB_SRCS_PY ${LIB_SRCS}) #copy all the library sources for the python wrapper
  list(FILTER LIB_SRCS EXCLUDE REGEX "_py.*")#remove the python files from the library sources

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