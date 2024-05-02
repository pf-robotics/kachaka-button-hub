#pragma once

#include <Arduino.h>
#include <vector>

struct Shelf {
  String id;
  String name;
};

struct Location {
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
};
