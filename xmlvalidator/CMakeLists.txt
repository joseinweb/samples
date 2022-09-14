cmake_minimum_required(VERSION 3.3)

project (xmlvalidator)
find_package(PkgConfig)
find_package(tinyxml2)

add_executable(${PROJECT_NAME} Validator.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE
    tinyxml2 )

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)