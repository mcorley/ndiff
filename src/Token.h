//===--- Token.h - Token interface ----------------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file defines the Token interface.
//
//===----------------------------------------------------------------------===

#ifndef TOKEN_H
#define TOKEN_H

#include <string>

/// Token - This structure provides full information about a lexed token.
class Token {
  std::string charData;
  int hashValue;
  int offset;
  int line, column;

  /// flags - Bits we track about this token, members of the TokenFlags enum.
  unsigned char flags;
public:
  // Various flags set per token:
  enum TokenFlags {
    startOfLine = 0x01,  // At start of line or only after whitespace.
    leadingSpace = 0x02,  // Whitespace exists before this token.
    whitespace = 0x04,  // This token is an identifier of whitspace.
    unused = 0x08
  };

  /// Token constructor - Create a new Token object.
  Token(std::string chardata, int hval, int off, int lin, int col)
    : charData(chardata), hashValue(hval), offset(off), line(lin), column(col) {
      clearFlag(startOfLine);
      clearFlag(leadingSpace);
      clearFlag(whitespace);
  }

  bool operator==(const Token &rhs) const { return hashValue == rhs.hashValue; }
  bool operator<(const Token &rhs) const { return offset < rhs.offset; }
  bool operator<=(const Token &rhs) const { return offset <= rhs.offset; }
  bool operator!=(const Token &rhs) const { return !(*this == rhs); }
  bool operator>(const Token &rhs) const { return rhs < *this; }
  bool operator>=(const Token &rhs) const { return rhs <= *this; }

  /// getCharacterData - Return the character data identified by this token.
  const std::string getCharData() const { return charData; }

  /// getColumn - Return the presumed column number of this location.  This can
  /// not be affected by #line, but is packaged here for convenience.
  int getHashValue() const { return hashValue; }

  /// getLine - Return the line number in the file this token was identified on.
  int getLine() const { return line; }

  /// getColumn - Return the presumed column number of this location.  This can
  /// not be affected by #line, but is packaged here for convenience.
  int getColumn() const { return column; }

  /// lexedOffset - Return a value for mapping virtual token indexes (not 
  ///                counting discarded tokens) to real ones (counting those 
  ///                tokens).
  int lexedOffset() const { return offset; }

  /// setFlag - Set the specified flag.
  void setFlag(TokenFlags flag) {
    flags |= flag;
  }

  /// clearFlag - Unset the specified flag.
  void clearFlag(TokenFlags flag) {
    flags &= ~flag;
  }

  /// setFlagValue - Set a flag to either true or false.
  void setFlagValue(TokenFlags flag, bool val) {
    if (val)
      setFlag(flag);
    else
      clearFlag(flag);
  }

  /// isAtStartOfLine - Return true if this token is at the start of a line.
  bool isAtStartOfLine() const { return (flags & startOfLine) ? true : false; }

  /// hasLeadingSpace - Return true if this token has whitespace before it.
  bool hasLeadingSpace() const { return (flags & leadingSpace) ? true : false; }

  /// isWhitespace - Return true if this token contains whitespace.
  bool isWhitespace() const { return (flags & whitespace) ? true : false; }
};

#endif // TOKEN_H
