/*
 * Copyright ©2024 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2024 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */
#include "./SimpleFileReader.hpp"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <array>
static constexpr uint64_t BUF_SIZE = 100000;
using namespace std;
SimpleFileReader::SimpleFileReader(const std::string& fname)
    : fd_(open(fname.c_str(), O_RDONLY)) {
  // fd_ = open(fname.c_str(), O_RDONLY);
  if (fd_ < 0) {
    this->good_ = false;
    exit(EXIT_FAILURE);
  }
  lseek(fd_, 0, SEEK_SET);
  good_ = true;
}

SimpleFileReader::~SimpleFileReader() {
  if (this->fd_ >= 0) {
    close(this->fd_);
    this->good_ = false;
    this->fd_ = -1;
  }
}

void SimpleFileReader::open_file(const std::string& fname) {
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }
  fd_ = open(fname.c_str(), O_RDONLY);
  if (fd_ < 0) {
    good_ = false;
    return;
  }
  lseek(fd_, 0, SEEK_SET);
  good_ = true;
}

void SimpleFileReader::close_file() {
  if (fd_ < -1) {
    return;
  }
  SimpleFileReader::~SimpleFileReader();
}

char SimpleFileReader::get_char() {
  if (fd_ == -1) {
    good_ = false;
    return EOF;
  }
  char temp = 0;
  if (fd_ >= 0) {
    ssize_t read_bytes = read(fd_, &temp, 1);
    if (read_bytes == 0) {  // end of file
      good_ = false;
      return EOF;
    }
  }
  good_ = true;
  return temp;
}

optional<string> SimpleFileReader::get_chars(size_t n) {
  if (fd_ < 0 || !good_) {  // file not open
    good_ = false;
    return nullopt;
  }
  string final_result;
  // char* result = new char[n + 1];
  array<char, BUF_SIZE> buf{};
  // vector<char> buf(n);
  size_t totalRead = 0;
  ssize_t bytesRead = 0;
  while (totalRead < n) {
    bytesRead = read(fd_, buf.data() + totalRead, n - totalRead);
    totalRead += bytesRead;
    if (bytesRead < 0) {
      if (errno != EINTR) {
        good_ = false;
        return nullopt;
      }
    }
    if (bytesRead == 0) {
      good_ = false;
      //   if (totalRead == 0) {
      //     return nullopt;
      //   }
      break;
    }
    good_ = true;
  }
  // result[totalRead] = '\0';
  // final_result = result;
  // delete[] result;
  // return final_result;
  return std::string(buf.begin(), buf.begin() + totalRead);
}

int SimpleFileReader::tell() const {
  if (this->fd_ == -1) {
    return -1;
  }
  int pos = (int)lseek(this->fd_, 0, SEEK_CUR);
  return pos;
}

void SimpleFileReader::rewind() {
  good_ = true;
  lseek(this->fd_, 0, SEEK_SET);
}
bool SimpleFileReader::good() const {
  return good_;
}