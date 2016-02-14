#pragma once

#include <stdio.h>
#include <time.h>
#include <string>

#include "Platform.h"

#ifdef WIN32

int     gettimeofday(struct timeval *tp, void *tzp);
#define snprintf    _snprintf

#endif

int         GetCurThreadID();

int         GetErrorCode();
std::string GetErrorText(int errcode);


