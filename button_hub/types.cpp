#include "types.hpp"

String GetLocationTypeString(LocationType type) {
  switch (type) {
    case LocationType::kOther:
      return "other";
    case LocationType::kCharger:
      return "charger";
    case LocationType::kShelfHome:
      return "shelf_home";
  }
  return String(static_cast<int>(type));
}
