#pragma once

#include <stdio.h>
#include <time.h>
#include <string>
#include <assert.h>

#include "base/define.h"

#ifdef WIN32

#define snprintf    _snprintf
int     gettimeofday(struct timeval *tp, void *tzp);

#endif

int         GetCurrThreadID();

int         GetLastErrorCode();
std::string GetErrorText(int errcode);
std::string GetLastErrorText();


