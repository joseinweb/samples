cmake_minimum_required(VERSION 3.3)
project(samples)

option(xmlsample "Include Tinyxml2sample" ON)

if(xmlsample)
    add_subdirectory(xmlvalidator)
endif()
