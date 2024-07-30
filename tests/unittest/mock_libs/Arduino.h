#pragma once

#include <cstdint>
#include <string>

class String {
 public:
  String();
  String(const char* cdata);
  String(const std::string& std_string);
  String(const String& lhs);
  String& operator=(const String& lhs);

  bool operator==(const String& lhs);
  bool operator!=(const String& lhs);
  String operator+(const String& lhs);

  char operator[](std::size_t index) const { return data_[index]; }

  String substring(std::size_t begin_index, size_t end_index) const;
  String substring(std::size_t begin_index) const;
  int indexOf(char ch, std::size_t from_index) const;
  int indexOf(char ch) const;
  int toInt() const;
  unsigned int length() const;
  const char* c_data() const;

 private:
  std::string data_;
};
