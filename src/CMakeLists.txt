# Add library
if(SYSCMDLINE_BUILD_STATIC)
    add_library(${PROJECT_NAME} STATIC)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SYSCMDLINE_STATIC)
else()
    add_library(${PROJECT_NAME} SHARED)
endif()

# Amalgamation
# if(SYSCMDLINE_ENABLE_AMALGAMATION)
# set(_amalgamation_file ${CMAKE_CURRENT_BINARY_DIR}/amalgamation_${PROJECT_NAME}.cpp)
# set(_amalgamation_lines)

# file(GLOB_RECURSE _src *.cpp)

# foreach(_item ${_src})
# list(APPEND _amalgamation_lines "#include \"${_item}\"")
# endforeach()

# string(JOIN "\n" _amalgamation_content ${_amalgamation_lines})
# file(WRITE ${_amalgamation_file} ${_amalgamation_content})

# file(GLOB_RECURSE _inc ../include/syscmdline/*.h *.h)
# target_sources(${PROJECT_NAME} PRIVATE ${_inc} ${_amalgamation_file})
# else()
# file(GLOB_RECURSE _src ../include/syscmdline/*.h *.h *.cpp)
# target_sources(${PROJECT_NAME} PRIVATE ${_src})
# endif()

file(GLOB_RECURSE _src ../include/syscmdline/*.h *.h *.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${_src})

target_compile_definitions(${PROJECT_NAME} PRIVATE SYSCMDLINE_LIBRARY)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>"
)
target_include_directories(${PROJECT_NAME} PRIVATE . ../include/syscmdline ../include/syscmdline/private)

if(SYSCMDLINE_FORCE_VALIDITY_CHECK OR(NOT CMAKE_BUILD_TYPE OR ${CMAKE_BUILD_TYPE} MATCHES "Deb"))
    target_compile_definitions(${PROJECT_NAME} PRIVATE SYSCMDLINE_ENABLE_VALIDITY_CHECK)
endif()

if(WIN32)
    set(RC_DESCRIPTION "C++ Advanced Command Line Parser")
    set(RC_COPYRIGHT "Copyright 2023 SineStriker")
    include("../cmake/winrc.cmake")
endif()

# if(MSVC)
# if(${CMAKE_BUILD_TYPE} STREQUAL "Release")
# target_compile_options(${PROJECT_NAME} PRIVATE /GL /Os)
# target_link_options(${PROJECT_NAME} PRIVATE /OPT:REF /OPT:ICF /OPT:LBR)
# endif()
# endif()
if(SYSCMDLINE_INSTALL)
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    install(TARGETS ${PROJECT_NAME}
        EXPORT ${SYSCMDLINE_INSTALL_NAME}Targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
    )
endif()