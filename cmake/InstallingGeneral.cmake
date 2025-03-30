# Install the library
install(TARGETS ${PROJECT_NAME}
EXPORT ${PROJECT_NAME}
COMPONENT ${PROJECT_NAME}
PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME} 
INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
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
"${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in" # input file
"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake" # output file
INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}
)

# Install the Config.cmake file
install(FILES
"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
"${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
DESTINATION lib/cmake/${PROJECT_NAME}
)