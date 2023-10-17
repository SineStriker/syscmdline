file(GLOB_RECURSE _src ../include/syscmdline/*.h *.h *.cpp)

# Add library
add_library(${PROJECT_NAME} STATIC ${_src})
target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_include_directories(${PROJECT_NAME} PRIVATE . ../include/syscmdline)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Install
install(TARGETS ${PROJECT_NAME}
    EXPORT ${SYSCMDLINE_INSTALL_NAME}Targets
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
)
