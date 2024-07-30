#include <stdexcept>
#include <string>

#include "Arduino.h"

String::String() : data_("") {}

String::String(const char* cdata_) : data_(cdata_) {}

String::String(const std::string& std_string) : data_(std_string) {}

String::String(const String& lhs) : data_(lhs.data_) {}

String& String::operator=(const String& lhs) {
  if (this != &lhs) {
    data_ = lhs.data_;
  }
  return *this;
}

bool String::operator==(const String& lhs) {
  return data_ == lhs.data_;
}

bool String::operator!=(const String& lhs) {
  return !(data_ == lhs.data_);
}

String String::operator+(const String& lhs) {
  return data_ + lhs.data_;
}

String String::substring(const std::size_t begin_index,
                         const std::size_t end_index) const {
  return data_.substr(begin_index, end_index - begin_index);
}

String String::substring(const std::size_t begin_index) const {
  return substring(begin_index, data_.size());
}

int String::indexOf(const char ch, const std::size_t from_index) const {
  std::size_t pos = data_.find(ch, from_index);
  return pos == std::string::npos ? -1 : static_cast<int>(pos);
}

int String::indexOf(const char ch) const {
  return indexOf(ch, 0);
}

int String::toInt() const {
  try {
    return std::stoi(data_);
  } catch (std::invalid_argument&) {
    return 0;
  }
}

unsigned int String::length() const {
  return data_.size();
}

const char* String::c_data() const {
  return data_.data();
}
