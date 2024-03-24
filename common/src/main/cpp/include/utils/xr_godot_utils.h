#pragma once

#include <string>
#include <vector>

#include <godot_cpp/variant/transform3d.hpp>
#include <openxr/openxr.h>

namespace godot {

class XrGodotUtils {
  public:
    static bool isUuidValid(const XrUuidEXT& uuid);
    static std::string bytesToHexStr(const uint8_t *data, int len);
    static std::vector<uint8_t> hexStrToBytes(std::string hexStr);
    static std::string uuidToString(XrUuidEXT uuid);
    static XrUuidEXT uuidFromString(std::string uuidStr);
    static XrPosef transformToPose(const Transform3D& transform);
    static Transform3D poseToTransform(const XrPosef& pose);
};

struct XrUuidExtCmp {
  bool operator()(const XrUuidEXT& a, const XrUuidEXT& b) const {
      return XrGodotUtils::uuidToString(a) < XrGodotUtils::uuidToString(b);
  }
};

} // namespace godot
