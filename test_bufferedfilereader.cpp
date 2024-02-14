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

#include <errno.h>
#include <sys/select.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <string>
#include "./BufferChecker.hpp"
#include "./BufferedFileReader.hpp"
#include "catch.hpp"

using namespace std;
static constexpr const char* kHelloFileName = "./test_files/Hello.txt";
static constexpr const char* kByeFileName = "./test_files/Bye.txt";
static constexpr const char* kLongFileName = "./test_files/war_and_peace.txt";
static constexpr const char* kGreatFileName = "./test_files/mutual_aid.txt";

// helper functions

static bool verify_token(const string& actual,
                         const string& expected_contents,
                         const string& delims,
                         off_t* offset) {
  off_t off = *offset;
  string expected = expected_contents.substr(off, actual.length());
  // std::cout << "actual:" << actual << endl;
  // std::cout << "expected:" << expected << endl;
  if (actual != expected) {
    *offset = off;
    return false;
  }

  off += actual.length();
  if (off >= static_cast<off_t>(expected_contents.length())) {
    // eof reached
    *offset = off;
    return true;
  }

  if (delims.find(expected_contents[off]) == string::npos) {
    *offset = off + 1;
    return false;
  }

  off++;

  *offset = off;
  return true;
}

// Ensures that the last token has a new line character after it
static bool verify_tokens(const vector<string> actual,
                          const string& expected_contents,
                          const string& delims,
                          off_t* offset) {
  // std::cout << "actual:" << endl;
  // for (size_t i = 0; i < actual.size(); ++i) {
  //   std::cout << actual[i] << std::endl;
  // }
  // std::cout << "expected:" << expected_contents << endl;
  if (actual.size() <= 0) {
    if (static_cast<size_t>(*offset) < expected_contents.length() &&
        expected_contents[*offset] == '\n') {
      *offset += 1;  // for \n since this is a blank line
    }
    return true;
  }

  for (size_t i = 0; i < actual.size() - 1; i++) {
    if (!verify_token(actual.at(i), expected_contents, delims, offset)) {
      return false;
    }
  }

  if (!verify_token(actual.at(actual.size() - 1), expected_contents,
                    string("\n"), offset)) {
    return false;
  }

  return true;
}

TEST_CASE("Basic", "[Test_BufferedFileReader]") {
  BufferedFileReader* bf = new BufferedFileReader(kHelloFileName);
  char c = bf->get_char();
  REQUIRE('H' == c);

  // Check that the buffer has 'h' in it
  BufferChecker bc(*bf);
  string expected("H");
  REQUIRE_FALSE(bc.check_token_errors(expected, 0));

  // Delete SF to make sure destructor works, then award points
  // if it doesn't crash
  delete bf;
}

TEST_CASE("open_close", "[Test_BufferedFileReader]") {
  // close when already closed
  BufferedFileReader* bf = new BufferedFileReader(kHelloFileName);
  REQUIRE(bf->good());
  bf->close_file();
  REQUIRE_FALSE(bf->good());
  REQUIRE(bf->tell() == -1);
  bf->close_file();
  REQUIRE_FALSE(bf->good());

  // open when already opened
  bf->open_file(kByeFileName);
  REQUIRE(bf->good());
  bf->open_file(kByeFileName);
  REQUIRE(bf->good());
  bf->open_file(kByeFileName);
  REQUIRE(bf->good());

  // open and close the same file over and over again
  for (size_t i = 0; i < 10; i++) {
    bf->open_file(kByeFileName);
    REQUIRE(bf->good());
    bf->close_file();
    REQUIRE_FALSE(bf->good());
    bf->close_file();
    REQUIRE_FALSE(bf->good());
    bf->open_file(kByeFileName);
    REQUIRE(bf->good());
  }

  // destructor on an already closed file
  delete bf;
}

TEST_CASE("get_char", "[Test_BufferedFileReader]") {
  // file contents
  string kHelloContents{};
  string kByeContents{};
  string kLongContents{};
  string kGreatContents{};

  // flil the above strings with the actual contents of the file
  ifstream hello_ifs(kHelloFileName);
  kHelloContents.assign((std::istreambuf_iterator<char>(hello_ifs)),
                        (std::istreambuf_iterator<char>()));
  ifstream bye_ifs(kByeFileName);
  kByeContents.assign((std::istreambuf_iterator<char>(bye_ifs)),
                      (std::istreambuf_iterator<char>()));
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));
  ifstream great_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(great_ifs)),
                        (std::istreambuf_iterator<char>()));

  // Hello test case
  BufferedFileReader bf(kHelloFileName);
  BufferChecker bc(bf);
  string contents;
  char c;
  for (size_t i = 0; i < kHelloContents.length(); i++) {
    REQUIRE(bf.tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.tell()));
    c = bf.get_char();
    contents += c;
    REQUIRE(bf.good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kHelloContents == contents);
  c = bf.get_char();
  REQUIRE(EOF == c);
  REQUIRE_FALSE(bf.good());

  // Goodbye test case
  bf.close_file();
  bf.open_file(kByeFileName);
  contents.clear();
  for (size_t i = 0; i < kByeContents.length(); i++) {
    REQUIRE(bf.tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.tell()));
    c = bf.get_char();
    contents += c;
    REQUIRE(bf.good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kByeContents == contents);
  c = bf.get_char();
  REQUIRE(EOF == c);
  REQUIRE_FALSE(bf.good());

  // Long file test case
  contents.clear();
  bf.close_file();
  REQUIRE(bf.tell() == -1);
  bf.open_file(kLongFileName);
  contents.reserve(kLongContents.length());
  for (size_t i = 0; i < kLongContents.length(); i++) {
    REQUIRE(bf.tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.tell()));
    c = bf.get_char();
    contents += c;
    REQUIRE(bf.good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kLongContents == contents);
  c = bf.get_char();
  REQUIRE(EOF == c);
  REQUIRE_FALSE(bf.good());

  // "Great" file test case
  contents.clear();
  bf.close_file();
  REQUIRE(bf.tell() == -1);
  bf.open_file(kGreatFileName);
  contents.reserve(kGreatContents.length());
  for (size_t i = 0; i < kGreatContents.length(); i++) {
    REQUIRE(bf.tell() >= 0);
    REQUIRE(i == static_cast<size_t>(bf.tell()));
    c = bf.get_char();
    contents += c;
    REQUIRE(bf.good());
    REQUIRE_FALSE(bc.check_char_errors(c, i));
  }
  REQUIRE(kGreatContents == contents);
  c = bf.get_char();
  REQUIRE(EOF == c);
  REQUIRE_FALSE(bf.good());
}

TEST_CASE("get_token", "[Test_BufferedFileReader]") {
  string token{};
  optional<string> opt{};
  string delims{"\t "};
  off_t offset{0};
  // file contents
  string kHelloContents{};
  string kLongContents{};

  // flil the above strings with the actual contents of the file
  ifstream hello_ifs(kHelloFileName);
  kHelloContents.assign((std::istreambuf_iterator<char>(hello_ifs)),
                        (std::istreambuf_iterator<char>()));
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));
  BufferedFileReader bf(kHelloFileName, delims);
  BufferChecker bc(bf);

  while (bf.good()) {
    opt = bf.get_token();
    REQUIRE(opt.has_value());
    token = opt.value();
    REQUIRE_FALSE(bc.check_token_errors(token, offset));
    REQUIRE(verify_token(token, kHelloContents, delims, &offset));
    REQUIRE(offset == static_cast<off_t>(bf.tell()));
  }

  REQUIRE(static_cast<off_t>(kHelloContents.length()) == offset);

  offset = 0;
  bf.open_file(kLongFileName);
  while (bf.good()) {
    opt = bf.get_token();
    REQUIRE(opt.has_value());
    token = opt.value();
    REQUIRE_FALSE(bc.check_token_errors(token, offset));
    REQUIRE(verify_token(token, kLongContents, delims, &offset));
    REQUIRE(offset == static_cast<off_t>(bf.tell()));
  }

  REQUIRE(static_cast<off_t>(kLongContents.length()) == offset);
  opt = bf.get_token();
  REQUIRE_FALSE(opt.has_value());
}

TEST_CASE("get_line", "[Test_BufferedFileReader]") {
  vector<string> tokens{};
  vector<string> tokens2{};
  optional<vector<string>> opt{};
  string delims = ",\t ";
  off_t offset = 0;
  // file contents
  string kByeContents{};
  string kGreatContents{};

  // flil the above strings with the actual contents of the file
  ifstream bye_ifs(kByeFileName);
  kByeContents.assign((std::istreambuf_iterator<char>(bye_ifs)),
                      (std::istreambuf_iterator<char>()));
  ifstream great_ifs(kGreatFileName);
  kGreatContents.assign((std::istreambuf_iterator<char>(great_ifs)),
                        (std::istreambuf_iterator<char>()));

  BufferedFileReader bf(kByeFileName, delims);
  BufferChecker bc(bf);

  while (bf.good()) {
    opt = bf.get_line();
    REQUIRE(opt.has_value());
    tokens = opt.value();
    REQUIRE(verify_tokens(tokens, kByeContents, delims, &offset));
    if (tokens.size() > 0) {
      REQUIRE_FALSE(bc.check_token_errors(
          tokens.back(), offset - (tokens.back().length() + 1)));
    }
    REQUIRE(offset == static_cast<off_t>(bf.tell()));
  }
  REQUIRE(static_cast<off_t>(kByeContents.length()) == offset);

  offset = 0;
  bf.open_file(kGreatFileName);

  while (bf.good()) {
    opt = bf.get_line();
    REQUIRE(opt.has_value());
    tokens = opt.value();
    REQUIRE(verify_tokens(tokens, kGreatContents, delims, &offset));
    if (tokens.size() > 0) {
      REQUIRE_FALSE(bc.check_token_errors(
          tokens.back(), offset - (tokens.back().length() + 1)));
    }
    REQUIRE(offset == static_cast<off_t>(bf.tell()));
  }
  REQUIRE(static_cast<off_t>(kGreatContents.length()) == offset);
  opt = bf.get_line();
  // tokens2 = opt.value();
  //  for (int i = 0; i < tokens2.size(); i++) {
  //    cout << "123:" << tokens2[i] << endl;
  //  }
  REQUIRE_FALSE(opt.has_value());
}

TEST_CASE("Complex", "[Test_BufferedFileReader]") {
  vector<string> tokens{};
  optional<vector<string>> tok_opt{};
  optional<string> opt{};
  string token;
  string delims = ",\n ";
  int len = 0;
  off_t offset = 0;
  bool read_line = false;
  string kLongContents{};
  ifstream long_ifs(kLongFileName);
  kLongContents.assign((std::istreambuf_iterator<char>(long_ifs)),
                       (std::istreambuf_iterator<char>()));

  BufferedFileReader bf(kLongFileName, delims);
  BufferChecker bc(bf);

  for (int i = 0; i < 3; i++) {
    while (bf.good()) {
      if (read_line) {
        tok_opt = bf.get_line();
        REQUIRE(tok_opt.has_value());
        tokens = tok_opt.value();
        REQUIRE(verify_tokens(tokens, kLongContents, delims, &offset));
        if (len > 0) {
          REQUIRE_FALSE(bc.check_token_errors(
              tokens[len - 1], offset - (tokens[len - 1].length() + 1)));
        }
        REQUIRE(offset == static_cast<off_t>(bf.tell()));
      } else {
        opt = bf.get_token();
        REQUIRE(opt.has_value());
        token = opt.value();
        REQUIRE_FALSE(bc.check_token_errors(token, offset));
        REQUIRE(verify_token(token, kLongContents, delims, &offset));
        REQUIRE(offset == static_cast<off_t>(bf.tell()));
      }
      read_line = !read_line;
    }

    REQUIRE(static_cast<off_t>(kLongContents.length()) == offset);
    opt = bf.get_token();
    REQUIRE_FALSE(opt.has_value());
    tok_opt = bf.get_line();
    REQUIRE_FALSE(tok_opt.has_value());
    offset = 0;
    bf.rewind();
  }
}