#include "utils/xr_godot_utils.h"

#include <iomanip>
#include <sstream>
#include <string>

using namespace godot;

bool XrGodotUtils::isUuidValid(const XrUuidEXT& uuid) {
  for (int i = 0; i < XR_UUID_SIZE_EXT; ++i) {
    if (uuid.data[i] > 0) {
      return true;
    }
  }
  return false;
}

std::string XrGodotUtils::bytesToHexStr(const uint8_t *data, int len) {
  std::stringstream ss;
  ss << std::hex;
  for(int i = 0; i < len; i++) {
    ss << std::setw(2) << std::setfill('0') << (int)data[i];
  }
  return ss.str();
}

std::vector<uint8_t> XrGodotUtils::hexStrToBytes(std::string data) {
  std::stringstream ss;
  ss << data;

  std::vector<uint8_t> resBytes;
  size_t count = 0;
  const auto len = data.size();
  while(ss.good() && count < len) {
      unsigned short num;
      char hexNum[2];
      ss.read(hexNum, 2);
      sscanf(hexNum, "%2hX", &num);
      resBytes.push_back(static_cast<uint8_t>(num));
      count += 2;
  }
  return resBytes;

}

std::string XrGodotUtils::uuidToString(XrUuidEXT uuid) {
  return bytesToHexStr(uuid.data, XR_UUID_SIZE_EXT);
}

XrUuidEXT XrGodotUtils::uuidFromString(std::string uuidStr) {
  XrUuidEXT uuid;
  auto bytes = hexStrToBytes(uuidStr);
  for (size_t i = 0; i < XR_UUID_SIZE_EXT; i++) {
    uuid.data[i] = bytes[i];
  }
  return uuid;
}

XrPosef XrGodotUtils::transformToPose(const Transform3D& transform) {
  auto quat = transform.basis.get_rotation_quaternion();
  return {
    { quat.x, quat.y, quat.z, quat.w }, // orientation
    { transform.origin.x, transform.origin.y, transform.origin.z } // position
  };
}

Transform3D XrGodotUtils::poseToTransform(const XrPosef& pose) {
	Quaternion q(pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w);
	Basis basis(q);
	Vector3 origin(pose.position.x, pose.position.y, pose.position.z);

	return Transform3D(basis, origin);
}
