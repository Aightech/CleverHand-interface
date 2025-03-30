if(BUILD_EXAMPLES AND DIR_DEPTH EQUAL 0)
    #for each main file
    foreach(MAIN_FILE ${MAIN_FILES})
      #the name of the executable is the name of the project + the main file (without the "main" and extension)
      get_filename_component(EXEC_NAME ${MAIN_FILE} NAME_WE)
      #replace main with the project name
      string(REPLACE "main" demo_${PROJECT_NAME} EXEC_NAME ${EXEC_NAME})
      message("${PREFIX_MSG}-- ${ColourBold}${Esc}[${color_code}m${EXEC_NAME}${ColourReset} will be built")
      add_executable(${EXEC_NAME} ${MAIN_FILE})
      target_link_libraries(${EXEC_NAME}
        PUBLIC ${LIB_NAME}
        PUBLIC ${TOOL_LIBS})
      target_include_directories(${EXEC_NAME}
        PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    endforeach()
  endif()