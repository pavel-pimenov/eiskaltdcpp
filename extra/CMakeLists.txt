project (extra)
cmake_minimum_required (VERSION 2.6)
include_directories (${PROJECT_SOURCE_DIR} ${GETTEXT_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${MINIUPNP_INCLUDE_DIR} ${LUA_INCLUDE_DIR})
aux_source_directory (${PROJECT_SOURCE_DIR} extra_SRCS)
if (FREE_SPACE_BAR_C)
    if (WIN32)
        list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/fsusage.c)
    endif (WIN32)
else (FREE_SPACE_BAR_C)
    list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/fsusage.c)
    list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/freespace.cpp)
endif (FREE_SPACE_BAR_C)

if (NOT USE_MINIUPNP)
    list (REMOVE_ITEM extra_SRCS ${PROJECT_SOURCE_DIR}/upnpc.cpp)
endif (NOT USE_MINIUPNP)

add_library (extra STATIC ${extra_SRCS})

if (USE_MINIUPNP AND WIN32)
    set (WIN32_LIBS "ws2_32" "iphlpapi")
endif (USE_MINIUPNP AND WIN32)

if (DEBUG_IPFILTER)
    add_definitions (-D_DEBUG_IPFILTER)
    message (STATUS "Enabled debug output from filter of IP addresses!")
endif (DEBUG_IPFILTER)

if (WIN32)
    set_property(TARGET extra PROPERTY COMPILE_FLAGS)
else (WIN32)
    set_property(TARGET extra PROPERTY COMPILE_FLAGS -fPIC)
endif (WIN32)

target_link_libraries (extra ${MINIUPNP_LIBRARY} ${WIN32_LIBS} ${GETTEXT_LIBRARIES} ${ICONV_LIBRARIES})
