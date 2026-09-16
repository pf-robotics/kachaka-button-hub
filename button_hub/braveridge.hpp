#pragma once

#include <cstdint>
#include <cstring>

#include "command_table.hpp"

namespace braveridge {

constexpr int kUuidLength = 16;

static const uint8_t kBraveridgeUuid[kUuidLength] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static const uint8_t kBraveridgePlusUuid1[kUuidLength] = {
    0x58, 0x1e, 0x31, 0xd6, 0xe7, 0xba, 0x40, 0x7a,
    0xb1, 0x2e, 0x94, 0x9a, 0xce, 0x47, 0x54, 0x85};
static const uint8_t kBraveridgePlusUuid2[kUuidLength] = {
    0xaa, 0x82, 0xce, 0x42, 0xbf, 0xc7, 0x41, 0x82,
    0xb7, 0x60, 0x1c, 0xca, 0x10, 0x11, 0x68, 0x76};

constexpr uint16_t kPlusLongPressMajorBit = 0x4000;

inline bool IsBraveridgeUuid(const uint8_t uuid[kUuidLength]) {
  return std::memcmp(uuid, kBraveridgeUuid, kUuidLength) == 0;
}

inline bool IsPlusUuid(const uint8_t uuid[kUuidLength]) {
  return std::memcmp(uuid, kBraveridgePlusUuid1, kUuidLength) == 0 ||
         std::memcmp(uuid, kBraveridgePlusUuid2, kUuidLength) == 0;
}

// ignore the minor and the lower 14 bits of major for Braveridge Plus buttons
inline void NormalizeAppleIBeacon(AppleIBeacon& beacon) {
  if (!IsPlusUuid(beacon.uuid)) {
    return;
  }
  beacon.major &= kPlusLongPressMajorBit;
  beacon.minor = 0;
}

}  // namespace braveridge
