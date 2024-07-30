#include "ip_resolver.hpp"

#include <M5Unified.h>
#include <WiFi.h>
#include <cctype>

#include "logging.hpp"

namespace ip_resolver {

static String s_hostname_cached;
static String s_ip_cached;

void Begin() {}

static bool IsKachakaSerialNumber(const String& name) {
  return (name.length() == 9 && std::toupper(name[0]) == 'B' &&
          std::toupper(name[1]) == 'K') ||
         (name.length() == 10 && std::isdigit(name[0]) &&
          std::isdigit(name[1]) && std::isdigit(name[2]) &&
          std::isdigit(name[3]) && std::isdigit(name[4]) &&
          std::isdigit(name[5]) && std::isdigit(name[6]) &&
          std::isdigit(name[7]) && std::isdigit(name[8]) &&
          std::isdigit(name[9])) ||
         (name.length() == 3 && std::isdigit(name[0]) &&
          std::isdigit(name[1]) && std::isdigit(name[2]));
}

String GetIpAddressIfPossible(const String& hostname, const bool debug_print) {
  String hostname_for_query = hostname;
  if (IsKachakaSerialNumber(hostname)) {
    hostname_for_query = "kachaka-" + hostname + ".local";
  }

  if (debug_print) {
    logging::Log("IpResolver: Resolving %s (%s)", hostname.c_str(),
                 hostname_for_query.c_str());
  }
  IPAddress ip;
  if (WiFi.hostByName(hostname_for_query.c_str(), ip)) {
    if (debug_print) {
      logging::Log("IpResolver: Resolved %s to %s", hostname_for_query.c_str(),
                   ip.toString().c_str());
    }
    s_hostname_cached = hostname;
    s_ip_cached = ip.toString();
    return s_ip_cached;
  }
  if (hostname == s_hostname_cached) {
    if (debug_print) {
      logging::Log("IpResolver: Use cached IP %s for %s", s_ip_cached.c_str(),
                   hostname.c_str());
    }
    return s_ip_cached;
  }
  if (debug_print) {
    logging::Log("IpResolver: Use hostname %s as-is",
                 hostname_for_query.c_str());
  }
  return hostname_for_query;
}

String GetCachedIpAddressOrPassthrough(const String& hostname) {
  if (hostname == s_hostname_cached) {
    return s_ip_cached;
  }
  return hostname;
}

}  // namespace ip_resolver
