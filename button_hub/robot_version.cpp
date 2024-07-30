#include "robot_version.hpp"

#include <Arduino.h>
#include <cctype>
#include <tuple>

namespace detail {
std::tuple<bool, int, int, int> ParseRobotVersion(const String& version) {
  const int dot1 = version.indexOf('.');
  if (dot1 < 0) {
    return {false, 0, 0, 0};
  }
  const int dot2 = version.indexOf('.', dot1 + 1);
  if (dot2 < 0) {
    return {false, 0, 0, 0};
  }
  // Check if each string piece is really a number
  for (int i = 0; i < static_cast<int>(version.length()); ++i) {
    if (i == dot1 || i == dot2) {
      continue;
    }
    if (!isdigit(version[i])) {
      return {false, 0, 0, 0};
    }
  }
  if (dot1 == 0 || dot2 == 0 || dot1 + 1 == dot2 ||
      dot2 + 1 == static_cast<int>(version.length())) {
    return {false, 0, 0, 0};
  }
  const int major = version.substring(0, dot1).toInt();
  const int minor = version.substring(dot1 + 1, dot2).toInt();
  const int patch = version.substring(dot2 + 1).toInt();
  return {true, major, minor, patch};
}
}  // namespace detail

bool IsReleaseVersionVersionFormat(const String& version) {
  return std::get<0>(detail::ParseRobotVersion(version));
}

RobotVersion::RobotVersion(const String& version) {
  auto result = detail::ParseRobotVersion(version);
  if (std::get<0>(result)) {
    major_ = std::get<1>(result);
    minor_ = std::get<2>(result);
    patch_ = std::get<3>(result);
  } else {
    major_ = minor_ = patch_ = -1;  // Invalid version
  }
}

RobotVersion::RobotVersion(int major, int minor, int patch)
    : major_(major), minor_(minor), patch_(patch) {}

bool RobotVersion::operator<(const RobotVersion& other) const {
  const bool is_release_version = IsReleaseVersion();
  const bool other_is_release_version = other.IsReleaseVersion();

  if (is_release_version != other_is_release_version) {
    // realease version < develop version
    return is_release_version;
  }

  if (not is_release_version and not other_is_release_version) {
    return false;
  }
  if (major_ != other.major_) {
    return major_ < other.major_;
  }
  if (minor_ != other.minor_) {
    return minor_ < other.minor_;
  }
  return patch_ < other.patch_;
}

bool RobotVersion::operator>(const RobotVersion& other) const {
  return other < *this;
}

bool RobotVersion::operator==(const RobotVersion& other) const {
  return major_ == other.major_ && minor_ == other.minor_ &&
         patch_ == other.patch_;
}

bool RobotVersion::operator!=(const RobotVersion& other) const {
  return !(*this == other);
}

bool RobotVersion::operator>=(const RobotVersion& other) const {
  return !(*this < other);
}

bool RobotVersion::IsReleaseVersion() const {
  return major_ != -1 && minor_ != -1 && patch_ != -1;
}
