/******************************************************************************
* (C) Copyright 2020 Darwin Tech, LLC, http://www.darwintechnologiesllc.com
*******************************************************************************
* This file is licensed under the Darwin Tech Embedded Software License Agreement.
* See the file "Darwin Tech - Embedded Software License Agreement.pdf" for 
* details. Read the terms of that agreement carefully.
*
* Using or distributing any product utilizing this software for any purpose
* constitutes acceptance of the terms of that agreement.
******************************************************************************/

#ifndef _DARWIN_LOG_H_
#define _DARWIN_LOG_H_

// The ALOG macro always prints
#ifndef ALOG
#define ALOG(format, ...) printf(format,## __VA_ARGS__)
#endif

#ifdef DARWIN_DEBUG
void DumpHex(void *AdrIn,int Len);

// The ELOG macro always prints and also calls ErrorBreakPoint.
#ifndef ELOG
#define ELOG(format, ...) printf("%s#%d: " format,__FUNCTION__,__LINE__,## __VA_ARGS__); \
        ErrorBreakPoint(0,__LINE__)
#endif
      
// The LOG macro only prints when the DEBUG define is set
// This macro adds the function name in from of the log message
#ifndef LOG
#define LOG(format, ...) printf("%s: " format,__FUNCTION__,## __VA_ARGS__)
#endif

// The LOG_RAW macro only prints when the DEBUG define is set
// This macro is the same as LOG but without adding the function name
#ifndef LOG_RAW
#define LOG_RAW(format, ...) printf(format,## __VA_ARGS__)
#endif

void LogGeckoEvent(void *Pkt,const char *Function);
#define LOG_GECKO_EVENT(x) LogGeckoEvent(x,__FUNCTION__)

#else    // DEBUG

#ifndef ELOG
#define ELOG(format, ...) ErrorBreakPoint(__FUNCTION__,__LINE__)
#endif
      
#ifndef LOG
#define LOG(format, ...)
#endif

#ifndef LOG_RAW
#define LOG_RAW(format, ...)
#endif

#define DumpHex(x,y)

void LOG_GECKO_EVENT(x)

#endif   // DARWIN_DEBUG

#endif   // _DARWIN_LOG_H_

