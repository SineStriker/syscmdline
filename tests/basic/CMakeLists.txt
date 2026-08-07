project(tst_basic)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

target_link_libraries(${PROJECT_NAME} PRIVATE syscmdline)

# The library asks for its standard privately, so the test has to ask for its own.
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# The suite reports its own failures and returns their count, so it means the
# same thing in a release build as it does in a debug one.
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_NONSTDC_NO_WARNINGS)
endif()
