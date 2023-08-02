set (RIOT_DAEMON RIoTDaemon)

find_path(RTMESSAGE_API_HEADER REQUIRED NAMES rtMessage.h HINTS ~/work/rbus/install/usr/include PATH_SUFFIXES rtmessage)

if (NOT ${RTMESSAGE_API_HEADER} STREQUAL "RTMESSAGE_API_HEADER-NOTFOUND")
        message("Found rtMessage.h ${RTMESSAGE_API_HEADER}")
        set(RTMESSAGE_API_HEADERS ${RTMESSAGE_API_HEADER})
endif()

find_library(RTMESSAGE_LIBRARY REQUIRED HINTS ~/work/rbus/install/usr/lib NAMES rtMessage)
set(RTMESSAGE_LIBRARIES "")
if (NOT ${RTMESSAGE_LIBRARY} STREQUAL "RTMESSAGE_LIBRARY-NOTFOUND")
        set(RTMESSAGE_LIBRARIES ${RTMESSAGE_LIBRARY})
endif()

add_executable(${RIOT_DAEMON} RIoTDaemon.cpp)
target_include_directories(${RIOT_DAEMON} PRIVATE  ${RTMESSAGE_API_HEADERS})
target_link_libraries(${RIOT_DAEMON} PRIVATE  ${RTMESSAGE_LIBRARIES} )

set_source_files_properties(RIoTDaemon.cpp PROPERTIES COMPILE_FLAGS "-fpermissive")

add_executable(sender sample_req.c)
target_include_directories(sender PRIVATE  ${RTMESSAGE_API_HEADERS})
target_link_libraries(sender PRIVATE  ${RTMESSAGE_LIBRARIES} )

add_executable(receiver sample_recv.c)
target_include_directories(receiver PRIVATE  ${RTMESSAGE_API_HEADERS})
target_link_libraries(receiver PRIVATE  ${RTMESSAGE_LIBRARIES} )

install(TARGETS ${RIOT_DAEMON} sender receiver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDR})