#include "logging.hpp"

#include <FS.h>
#include <SPIFFS.h>
#include <cstdio>
#include <deque>
#include <vector>

#include "mutex.hpp"

namespace logging {

constexpr int kMaxLogFileCount = 30;

static String g_filename;

static kb::Mutex g_mutex;
static std::deque<String> g_queue;  // guard by g_mutex

static String GenDateTime() {
  time_t now = time(nullptr);
  struct tm* ptm = gmtime(&now);
  char out[20];
  sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900,
          ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min,
          ptm->tm_sec);
  return out;
}

void RemoveOldLogsKeepingLastNItems(int size) {
  constexpr char kDir[] = "/";
  File root = SPIFFS.open(kDir);
  if (!root || !root.isDirectory()) {
    Serial.println("Failed to open directory");
    return;
  }

  std::vector<String> file_names;
  for (File entry = root.openNextFile(); entry; entry = root.openNextFile()) {
    String name = entry.name();
    if (name.startsWith("log") && name.endsWith(".txt")) {
      file_names.push_back(std::move(name));
    }
  }

  std::sort(file_names.begin(), file_names.end());
  while (file_names.size() > size) {
    const String& name = file_names.front();
    Serial.printf("Removing %s%s\n", kDir, name.c_str());
    if (!SPIFFS.remove(kDir + name)) {
      Serial.printf("Failed to remove %s%s\n", kDir, name.c_str());
    }
    file_names.erase(file_names.begin());
  }
}

void Begin(int log_unique_id) {
  RemoveOldLogsKeepingLastNItems(kMaxLogFileCount);

  char buf[16];
  snprintf(buf, sizeof(buf), "/log%05d.txt", log_unique_id);
  g_filename = buf;

  Serial.printf("Logging to %s\n", g_filename.c_str());
}

void Log(const char* format, ...) {
  // Minimize memory allocation as much as possible
  int needed = 0;
  {
    va_list args;
    va_start(args, format);
    needed = vsnprintf(nullptr, 0, format, args);
    va_end(args);
  }
  if (needed <= 0) {
    return;
  }

  String line;
  int header_size = 0;
  {
    const String date = GenDateTime();
    header_size = static_cast<int>(date.length()) + 1;  // +1 for space

    // Pre-allocate buffer (+2 for \n and \0)
    line.reserve(header_size + needed + 2);
    line += date;
    line += ' ';
    for (int i = 0; i < needed; i++) {
      line += '_';  // this will be overwritten
    }
  }

  {
    va_list args;
    va_start(args, format);
    vsnprintf(&line[header_size], needed + 1, format, args);
    va_end(args);
  }
  line += '\n';

  Serial.print(line);
  if (kb::LockGuard lock(g_mutex); lock) {
    g_queue.push_back(std::move(line));
  }
}

void Update() {
  while (true) {
    String line;
    {
      kb::LockGuard lock(g_mutex);
      if (!lock) {
        break;
      }
      if (g_queue.empty()) {
        break;
      }
      line = std::move(g_queue.front());
      g_queue.pop_front();
    }

    File file = SPIFFS.open(g_filename, "a");
    if (file) {
      file.write(reinterpret_cast<const uint8_t*>(line.c_str()), line.length());
      file.flush();
      file.close();
    } else {
      Serial.println("logging: File is not opened");
    }
  }
}

}  // namespace logging
