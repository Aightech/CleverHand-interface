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