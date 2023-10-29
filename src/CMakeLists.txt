file(GLOB_RECURSE _src ../include/syscmdline/*.h *.h *.c *.hpp *.cpp)

# Add library
if(SYSCMDLINE_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SYSCMDLINE_STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${_src})
target_compile_definitions(${PROJECT_NAME} PRIVATE SYSCMDLINE_LIBRARY)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
)
target_include_directories(${PROJECT_NAME} PRIVATE . ../include/syscmdline ../include/syscmdline/private)

if(WIN32)
    set(RC_DESCRIPTION "C++ Advanced Command Line Parser")
    set(RC_COPYRIGHT "Copyright 2023 SineStriker")
    include("../cmake/winrc.cmake")
endif()

# if(MSVC)
#     if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
#         target_compile_options(${PROJECT_NAME} PRIVATE /Gw /Gy /Zc:inline)
#         target_link_options(${PROJECT_NAME} PRIVATE /OPT:REF /OPT:ICF /OPT:LBR)
#     endif()
# endif()

if(SYSCMDLINE_INSTALL)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    # Install
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${SYSCMDLINE_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
    )
endif()