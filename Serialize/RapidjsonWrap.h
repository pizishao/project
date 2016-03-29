#pragma once

#include "common.h"

#include "rapidjson/document.h"
#include "rapidjson/rapidjson.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


namespace Serialization
{
    typedef Node<rapidjson::Value *>    JsonSeriNode;
}