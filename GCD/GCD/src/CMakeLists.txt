include_directories(${PROJECT_SOURCE_DIR}/src ${OPENSSL_INCLUDE_DIR})

set (BMTP_SRCS
	bmtp.cpp
	bmtp.h
	list.h
	stdafx.h)

if (UNIX)
#	set (BMTP_SRCS ${BMTP_SRCS}
#	os/linux/wbt_terminal.h)
endif (UNIX)

if (WIN32 OR CYGWIN)
	set (BMTP_SRCS ${BMTP_SRCS}
	dllmain.cpp)
endif (WIN32 OR CYGWIN)

add_library(
    bmtp
    SHARED
    ${BMTP_SRCS}
)

add_library(
    bmtp_static
    STATIC
    ${BMTP_SRCS}
)

set_target_properties (bmtp_static PROPERTIES OUTPUT_NAME "bmtp")
set_target_properties (bmtp_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
set_target_properties (bmtp PROPERTIES CLEAN_DIRECT_OUTPUT 1)

set_target_properties (bmtp PROPERTIES	VERSION 0.5 SOVERSION 0.5)

install (TARGETS bmtp bmtp_static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
install (FILES bmtp.h DESTINATION include/bmtp)

