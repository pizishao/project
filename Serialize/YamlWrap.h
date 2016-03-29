#pragma once

#include "yaml-cpp/yaml.h"
#include "common.h"

namespace Serialization
{
    typedef YAML::Node          YamlNode;
    typedef Node<YamlNode>      YamlSeriNode;
}
