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

#ifndef BUFFEREDFILEREADER_HPP_
#define BUFFEREDFILEREADER_HPP_

#include <array>
#include <optional>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
// A BufferedFileReader is a class for reading files.
//
// This class is a moderately complex wrapper around POSIX file I/O calls
// with more functionality than SimpleFileReader. Reading from the file
// is buffered to increase performance.
///////////////////////////////////////////////////////////////////////////////
class BufferedFileReader {
 public:
  // Constructor for a BufferedFileReader. Should open the
  // file and do whatever is necesary to "set-up" the object.
  // After construction, reading from the file should start
  // at the front of the file.
  // Undefined behaviour if the file name is invalid.
  //
  // Arguments:
  // - fname: The name of the file to be read
  // - delims: a string containing all of the characters to
  //   be used as delimiters for reading tokens.
  //   NOTE: delims is an optional arguement and is by default
  //   set to white space characters
  //
  //   BufferedFileReader does NOT take ownership of either string.
  //   In other words, it is the caller's responsibility to allocate
  //   and free them. The BufferedFileReader maintains copies
  //   of the strings needed for it's functionality.
  BufferedFileReader(const std::string& fname,
                     const std::string& delims = "\r\n\t ");

  // Destructor for a BufferedFileReader. Should clean up
  // any allocated resources such as memory or open files.
  //
  // Arguments: None
  ~BufferedFileReader();

  // Sets up the BufferedFileReader to start reading from the
  // front of the specified file. Note that there could or could not
  // already be an open file managed by the BufferedFileReader.
  // This function handles both cases and if the object is already
  // managing a file, that file is closed.
  // Undefined behaviour if the file name is invalid.
  //
  // Arguments:
  // - fname: The name of the file to be opened
  void open_file(const std::string& fname);

  // Closes the file currently managed by the BufferedFileReader.
  // If there is not a file currently open, then nothing should happen.
  //
  // Arguments: None
  void close_file();

  // Gets the next singular character from the file.
  //
  // Arguments: None
  //
  // Returns:
  // - the next char in the file. If at the end of the file,
  //   or if there is no file open currently, then EOF is returned.
  char get_char();

  // The next two functions deal with the reading of "tokens".
  // A token is a sequence of characters whose end is marked by a delimiter
  // character and does not contain any delimiters in it. Note that the
  // end of file can be thought of as a delimiter.
  // Also note that a token can be the empty string.
  //
  // For example: if the file had the contents "hi,there,,aaaaa,!0 fds"
  // and the delimiters for the file was specified to only be the ','
  // character, then the tokens in the file would be:
  // - "hi"
  // - "there"
  // - ""
  // - "aaaaa"
  // - "!0 fds"

  // Reads the next token from the file.
  //
  // See above comments for what a token is.
  //
  // When reading tokens, if a token were to be read, we can mark the
  // delimiter for that token as having been read. For example,
  // if we used the example above and starting from the beginning of the file,
  // after the first call to get_token, the unread part of the file would be
  // "there,,aaaaa,!0 fds".
  //
  // Returns nullopt if already at EOF
  // or if there is no open file
  //
  // Arguments: None
  //
  // Returns:
  // - the next token in the file,
  // - nullopt if alrady at EOF or if the file is not open.
  std::optional<std::string> get_token();

  // Reads tokens until a new line is encountered and returns
  // those tokens in an array.
  // If we are already at the EOF, then return nullptr.
  // See the comments above get_token() for what a token is.
  // Undefined behaviour if there is no file open currently.
  //
  // For example, if a file had the contents
  //////////////////////////////////////////////////////////
  // token1 token2 token3
  // token5 token6 token7 token8
  // token9
  //////////////////////////////////////////////////////////
  // and we started from the beginnig of the file, then
  // calling get_line would first return an array of size 3
  // containing {token1, token2, token3}.
  // The second call to get_line would return an array of size 4
  // containing {token5, token6, token7, token8}.
  // The final call to get_line would return an array of size 1
  // containing {token9}.
  //
  // NOTE: Be sure to consider that the newline character '\n'
  // May be a delimiter character. The behaviour of this function
  // should not change whether '\n' is a delimiter or not.
  //
  // HINT: you probably can't call get_token to implement this function
  //
  // Arguments: None
  //
  // Returns:
  // - the vector of tokens. if 0 tokens were read before
  //   the newline character or EOF then return an empty vector.
  // - If the file is already at the end of the file or the file is
  //   not open, return nullopt
  std::optional<std::vector<std::string>> get_line();

  // Returns the current position the user is in to the file.
  //
  // Arguments: None
  //
  // Returns:
  // - The current position we are in the file, which is the
  //   Offset from the start of the file. (e.g, if the user is at
  //   the start of the file, returns 0. If the user has read 2
  //   characters, return 2. etc.).
  // - -1 if there is no open file
  int tell() const;

  // Resets the file to start reading from the beginning
  // of the file that is currently open.
  // Does Nothing if there is no file open currently.
  //
  // Arguments: None
  void rewind();

  // Returns whether or not the file is available for reading
  // (e.g. if the file is open and not at the end of file)
  // Note: The reader is only considered to be at the end of file
  // if it had previously tried to read, and then hit the end
  // of the file, causing it to have an "incomplete" read).
  //
  // Arguements: None
  //
  // Returns:
  // - false if the file reader is at the end of the file
  //   or if there is no file open
  // - true otherwise
  bool good() const;

  // Ignore These
  // If you want to know more, this is disabling the
  // copy constructor and the assignment operator.
  // These will be covered later in the class
  BufferedFileReader(const BufferedFileReader& other) = delete;
  BufferedFileReader& operator=(const BufferedFileReader& other) = delete;
  BufferedFileReader(const BufferedFileReader&& other) = delete;
  BufferedFileReader& operator=(const BufferedFileReader&& other) = delete;

  // Ignore this
  // This is necessary for testing and will be talked about later in the course
  friend class BufferChecker;

 private:
  // Constants
  static constexpr uint64_t BUF_SIZE = 1024;  // the size of the buffer.

  // fields
  int curr_length_;  // The current number of characters stored in the buffer
                     // To understand the purpose of this, consider when
                     // a file is less than BUF_SIZE in length.

  int curr_index_;  // The current index we are in to the buffer.
                    // necessary since we many not parse the entire
                    // buffer in one function call.

  std::array<char, BUF_SIZE> buffer_;  // The buffer we maintiain for reading
                                       // from the file.

  int fd_;              // The File Descriptor that we use to manage our file.
  std::string delims_;  // the delimiters used for reading tokens
  bool good_;           // Whether or not the reader is good to read

  // Suggested Helpers
  void fill_buffer();
  bool is_delim(char to_check);
  int buf_num = 0;
};

#endif  // BUFFEREDFILEREADER_HPP_
