#include <Arduino.h>

class RobotVersion {
 public:
  RobotVersion(const String& version);
  RobotVersion(int major, int minor, int patch);

  bool operator<(const RobotVersion& other) const;
  bool operator>(const RobotVersion& other) const;
  bool operator==(const RobotVersion& other) const;
  bool operator!=(const RobotVersion& other) const;
  bool operator>=(const RobotVersion& other) const;

  bool IsReleaseVersion() const;

 private:
  int major_;
  int minor_;
  int patch_;
};

namespace detail {
std::tuple<bool, int, int, int> ParseRobotVersion(const String& version);
}
