# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 2.8.11)
project (RiotClient)

set(MODULE_NAME RiotClient)
set(CMAKE_VERBOSE_MAKEFILE ON)


find_package(PkgConfig)

find_library(AVAHI_CLIENT_LIBRARIES NAMES avahi-client)
find_library(AVAHI_COMMON_LIBRARIES NAMES avahi-common)
mark_as_advanced(AVAHI_CLIENT_LIBRARIES)
mark_as_advanced(AVAHI_COMMON_LIBRARIES)



add_executable(${MODULE_NAME} 
        RIoTControl.cpp
        RIoTConnector.cpp
        AvahiClient.cpp)

find_path(RTMESSAGE_API_HEADER NAMES rtMessage.h HINTS ~/work/rbus/install/usr/include PATH_SUFFIXES rtmessage)

set(RTMESSAGE_API_HEADERS "")
if (NOT ${RTMESSAGE_API_HEADER} STREQUAL "RTMESSAGE_API_HEADER-NOTFOUND")
        message("Found rtMessage.h ${RTMESSAGE_API_HEADER}")
        set(RTMESSAGE_API_HEADERS ${RTMESSAGE_API_HEADER})

endif()

set(RTMESSAGE_LIBRARIES "")
find_library(RTMESSAGE_LIBRARY REQUIRED HINTS ~/work/rbus/install/usr/lib NAMES rtMessage)
if (NOT ${RTMESSAGE_LIBRARY} STREQUAL "RTMESSAGE_LIBRARY-NOTFOUND")
        set(RTMESSAGE_LIBRARIES ${RTMESSAGE_LIBRARY})
endif()


set_source_files_properties(
	RIoTControl.cpp
        PROPERTIES COMPILE_FLAGS "-fexceptions")

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_include_directories(${MODULE_NAME} PRIVATE ../helpers ${RTMESSAGE_API_HEADERS})
target_link_libraries(${MODULE_NAME} PRIVATE ${RTMESSAGE_LIBRARIES} ${AVAHI_CLIENT_LIBRARIES} ${AVAHI_COMMON_LIBRARIES})

install(TARGETS ${MODULE_NAME}
        DESTINATION bin)

