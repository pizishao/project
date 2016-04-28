#pragma once

#include <stdio.h>
#include <time.h>
#include <string>
#include <assert.h>

#include "Platform.h"

#ifdef WIN32

#define snprintf    _snprintf
int     gettimeofday(struct timeval *tp, void *tzp);

#endif

int         GetCurThreadID();

int         GetLastErrorCode();
std::string GetErrorText(int errcode);
std::string GetLastErrorText();


