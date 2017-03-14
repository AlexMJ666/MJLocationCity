// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the BMTP_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// BMTP_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef __BMTP_H
#define __BMTP_H

#ifdef WIN32
    #ifdef BMTP_EXPORTS
        #define BMTP_API __declspec(dllexport)
    #else
        #define BMTP_API __declspec(dllimport)
    #endif
#else
    #define BMTP_API
#endif

// This class is exported from the BMTP.dll
//class BMTP_API CBMTP {
//public:
//	CBMTP(void);
//	// TODO: add your methods here.
//};

//extern BMTP_API int nBMTP;

//BMTP_API int fnBMTP(void);

#define BMTP_OK              0
#define BMTP_OUT_OF_MEMORY   1

#define BMTP_AT_MOST_ONCE    0
#define BMTP_AT_LEAST_ONCE   1
#define BMTP_EXACTLY_ONCE    2

typedef void BMTP;

BMTP_API int bmtp_init();
BMTP_API void bmtp_cleanup();

BMTP_API BMTP* bmtp_new( const char *ip, int port );

BMTP_API int bmtp_set_on_open( BMTP*, void( *on_open )( BMTP* ) );
BMTP_API int bmtp_set_on_close( BMTP*, void( *on_close )( BMTP* ) );
BMTP_API int bmtp_set_on_error( BMTP*, void( *on_error )( BMTP*, int err_no ) );
BMTP_API int bmtp_set_on_message( BMTP*, void( *on_message )( BMTP*, const char *data, int len ) );

BMTP_API int bmtp_open(BMTP*);
BMTP_API int bmtp_pub( BMTP*, const char *data, int len, int qos );
BMTP_API int bmtp_sub(BMTP*, unsigned long long int channel_id);

BMTP_API int bmtp_debug( BMTP* );

#endif /* __BMTP_H */