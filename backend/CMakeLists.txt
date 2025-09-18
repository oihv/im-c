project(lws-minimal-ws-server C)
cmake_minimum_required(VERSION 3.10)

# Cross-platform static linking configuration
option(STATIC_BUILD "Build with static linking" ON)
option(CROSS_COMPILE_WINDOWS "Cross-compile for Windows" OFF)

if(STATIC_BUILD)
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
    if(WIN32 OR CROSS_COMPILE_WINDOWS)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
        # Windows-specific libraries for static linking
        set(PLATFORM_LIBS ws2_32 winmm kernel32 user32 gdi32 winspool shell32 ole32 oleaut32 uuid comdlg32 advapi32)
    else()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

# Cross-platform dependency finding
if(WIN32 OR CROSS_COMPILE_WINDOWS)
    # For Windows cross-compilation, find libwebsockets
    find_library(WEBSOCKETS_LIB 
        NAMES libwebsockets_static libwebsockets websockets
        PATHS /usr/x86_64-w64-mingw32/lib
        NO_DEFAULT_PATH
    )
    
    if(WEBSOCKETS_LIB)
        set(LIBWEBSOCKETS_LIBRARIES ${WEBSOCKETS_LIB})
        set(LIBWEBSOCKETS_INCLUDE_DIRS /usr/x86_64-w64-mingw32/include)
        set(LIBWEBSOCKETS_DEP_LIBS ${PLATFORM_LIBS})
        message(STATUS "Found Windows libwebsockets: ${WEBSOCKETS_LIB}")
    else()
        message(FATAL_ERROR "libwebsockets not found for Windows. Please install libwebsockets development files for MinGW.")
    endif()
    
    # Skip the complex libwebsockets config for cross-compilation
    set(requirements 1)
else()
    # Linux - use standard libwebsockets config
    find_package(libwebsockets CONFIG REQUIRED)
    list(APPEND CMAKE_MODULE_PATH ${LWS_CMAKE_DIR})
    include(CheckCSourceCompiles)
    include(LwsCheckRequirements)
    
    set(requirements 1)
    require_lws_config(LWS_ROLE_WS 1 requirements)
    require_lws_config(LWS_WITH_SERVER 1 requirements)
endif()

set(SAMP lws-minimal-ws-server)
set(SRCS minimal-ws-server.c)

if (requirements)
	add_executable(${SAMP} ${SRCS})

	if(WIN32 OR CROSS_COMPILE_WINDOWS)
		# Windows cross-compilation - use found libraries
		target_include_directories(${SAMP} PUBLIC ${LIBWEBSOCKETS_INCLUDE_DIRS})
		target_link_libraries(${SAMP} ${LIBWEBSOCKETS_LIBRARIES} ${LIBWEBSOCKETS_DEP_LIBS})
	else()
		# Linux - use standard libwebsockets linking
		if (websockets_shared)
			target_link_libraries(${SAMP} websockets_shared ${LIBWEBSOCKETS_DEP_LIBS})
			add_dependencies(${SAMP} websockets_shared)
		else()
			target_link_libraries(${SAMP} websockets ${LIBWEBSOCKETS_DEP_LIBS})
		endif()
	endif()
endif()