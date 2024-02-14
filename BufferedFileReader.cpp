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

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "BufferedFileReader.hpp"
using namespace std;

BufferedFileReader::BufferedFileReader(const std::string& fname,
                                       const std::string& delims)
    : buffer_{}, fd_(open(fname.c_str(), O_RDONLY)) {
  // fd_ = open(fname.c_str(), O_RDONLY);
  if (fd_ == -1) {
    good_ = false;
    return;
  }
  lseek(fd_, 0, SEEK_SET);

  this->good_ = true;
  this->delims_ = delims;
  this->curr_length_ = 0;
  this->curr_index_ = 0;
  fill_buffer();
}

BufferedFileReader::~BufferedFileReader() {
  if (this->fd_ >= 0) {
    close(this->fd_);
    this->good_ = false;
    this->fd_ = -1;
    this->curr_length_ = 0;
    this->curr_index_ = 0;
  }
}

void BufferedFileReader::open_file(const std::string& fname) {
  if (this->fd_ >= 0) {
    close(this->fd_);
    this->fd_ = -1;
  }
  this->fd_ = open(fname.c_str(), O_RDONLY);
  if (this->fd_ < 0) {
    this->good_ = false;
    return;
  }
  this->good_ = true;
  lseek(this->fd_, 0, SEEK_SET);
}

void BufferedFileReader::close_file() {
  if (this->fd_ < -1) {
    return;
  }
  BufferedFileReader::~BufferedFileReader();
}

char BufferedFileReader::get_char() {
  if (this->fd_ == -1) {
    this->good_ = false;
    return EOF;
  }
  if (this->fd_ >= 0) {
    if (curr_index_ >= curr_length_) {
      fill_buffer();
      if (!good_) {
        return EOF;
      }
    }
  }
  char result = buffer_.at(curr_index_++);  // buffer_[curr_index_];
  // curr_index_++;
  return result;
}

// optional<string> BufferedFileReader::get_token() {
//   if (this->fd_ == -1) {
//     this->good_ = false;
//     return nullopt;
//   }
//   string result = "";
//   char& ch = buffer_[curr_index_];
//   while (is_delim(ch)) {
//     ch = buffer_[curr_index_++];
//     if (curr_index_ == curr_length_) {
//       break;
//     }
//     result += ch;
//   }
//   return result;
// }
optional<string> BufferedFileReader::get_token() {
  if (this->fd_ == -1) {
    this->good_ = false;
    return nullopt;
  }
  string token;  //= "";
  while (good_) {
    if (curr_index_ >= curr_length_) {
      fill_buffer();
      if (curr_length_ == 0) {
        good_ = false;
        break;
      }
    }
    char& cha = buffer_.at(curr_index_++);  //[curr_index_];
    if (is_delim(cha) || cha == -1) {
      break;
    }
    token += cha;
  }
  if (token.empty() && !good_) {
    return nullopt;
  }
  return token;
}
optional<vector<string>> BufferedFileReader::get_line() {
  if (fd_ == -1 || !good_) {
    good_ = false;
    return nullopt;
  }
  vector<string> line = {};
  string token;  //= "";
  size_t totalRead = 0;

  while (good_) {
    if (curr_index_ >= curr_length_) {
      fill_buffer();
      if (curr_length_ == 0) {
        good_ = false;
        break;
      }
    }
    char cha = buffer_.at(curr_index_++);
    if (cha == '\n') {
      line.push_back(token);
      totalRead++;
      token = "";
      break;
    }
    if (is_delim(cha)) {
      line.push_back(token);
      totalRead++;
      token = "";
    } else {
      token += cha;
    }
  }
  //   if (!token.empty()) {
  //     line.push_back(token);
  //     token.clear();
  //   }
  // if (totalRead == 0) {
  // return nullopt;
  //}
  return line;
}

int BufferedFileReader::tell() const {
  if (this->fd_ == -1) {
    return -1;
  }
  int pos = (int)lseek(this->fd_, 0, SEEK_CUR);
  return pos - curr_length_ + curr_index_;
  // return curr_index_ + BUF_SIZE * (buf_num - 1);
}

void BufferedFileReader::rewind() {
  if (this->fd_ == -1) {
    this->good_ = false;
    return;
  }
  this->good_ = true;
  lseek(this->fd_, 0, SEEK_SET);
  fill_buffer();
}

bool BufferedFileReader::good() const {
  return good_;
}

// void BufferedFileReader::fill_buffer() {
//   if (this->fd_ == -1) {
//     this->good_ = false;
//   }
//   uint64_t totalRead = 0;
//   while (totalRead < BUF_SIZE) {
//     curr_length_ = read(fd_, buffer_.data(), BUF_SIZE);
//     if (curr_length_ > 0) {
//       curr_index_ = 0;
//       good_ = true;

//     } else {
//       good_ = false;  // End of file or error
//     }
//     totalRead += curr_length_;
//   }
//   buf_num++;
// }

bool BufferedFileReader::is_delim(char to_check) {
  return delims_.find(to_check) != string::npos;
}

void BufferedFileReader::fill_buffer() {
  curr_length_ = 0;
  ssize_t result = 0;

  if (fd_ == -1) {
    good_ = false;
    exit(EXIT_FAILURE);
  }

  ssize_t bytesRead = 0;
  while (static_cast<uint64_t>(bytesRead) < BUF_SIZE) {
    result = read(fd_, buffer_.data() + bytesRead, BUF_SIZE - bytesRead);
    if (result == -1) {
      if (errno != EINTR) {
        good_ = false;
        return;
      }
    } else if (result == 0) {
      good_ = false;
      break;
    }
    bytesRead += result;
  }
  curr_length_ = (int)bytesRead;
  curr_index_ = 0;
  // buf_num++;
  if (static_cast<uint64_t>(curr_length_) < BUF_SIZE) {
    good_ = false;
  }

  if (curr_length_ == 0) {
    // good_ = false;
    ;
  } else {
    good_ = true;
  }
}
