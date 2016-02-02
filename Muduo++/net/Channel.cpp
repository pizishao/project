#include "Channel.h"

static const int kNoneEvent     = 0x00;
static const int kReadEvent     = 0x01;
static const int kWriteEvent    = 0x02;
static const int kErrorEvent    = 0x04;