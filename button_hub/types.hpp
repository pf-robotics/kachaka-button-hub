#pragma once

#include <Arduino.h>
#include <vector>

struct Shelf {
  String id;
  String name;
};

enum class LocationType : uint8_t { kOther = 0, kCharger = 1, kShelfHome = 2 };

String GetLocationTypeString(LocationType type);

struct Location {
  String id;
  String name;
  LocationType type;
};

struct Shortcut {
  String id;
  String name;
};

struct RobotInfoHolder {
  bool has_robot_version = false;
  String robot_version;
  bool has_shelves = false;
  std::vector<Shelf> shelves;
  bool has_locations = false;
  std::vector<Location> locations;
  bool has_shortcuts = false;
  std::vector<Shortcut> shortcuts;
};

struct LockOnEnd {
  bool enabled = false;
  double duration_sec;
};
