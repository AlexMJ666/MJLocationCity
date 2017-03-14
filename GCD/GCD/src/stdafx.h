// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC 
	#include <stdlib.h> 
	#include <crtdbg.h>
#else
	#include <stdlib.h> 
#endif

#ifdef WIN32
    #include <SDKDDKVer.h>
    #define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
    // Windows Header Files:
    #include <windows.h>
    #include <process.h>
#else
	#include <pthread.h>
    #include <unistd.h>
#endif

// TODO: reference additional headers your program requires here
//#include <tchar.h>

#include <string.h>
#include <errno.h>
#include <signal.h>

#ifndef WIN32
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
#else
	#include <winsock2.h>
#endif

#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/util.h>
#include <event2/event.h>
#include <event2/thread.h>
