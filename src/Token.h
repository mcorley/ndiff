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

/// Token - This structure provides full information about a lexed token.
/// It is not intended to be space efficient, it is intended to return as much
/// information as possible about each returned token.  This is expected to be
/// compressed into a smaller form if memory footprint is important.
class Token {
  /// Character sequence identified for this token.
  std::string Data;
  
  /// Line number in the file this token was identified on.
  unsigned Line;
  
  /// An equivalence code for this token.
  unsigned Equiv;
  
  /// True if this token is at the start of a line.
  bool StartOfLine;

  /// true if this token represents whitespace.
  bool Whitespace
  
  /// Offset position in the overall file.
  unsigned FilePos;

  /// Offset position in the overall file.
  unsigned LinePos;

public:
  /// Token constructor - Create a new Token object.
  Token(std::string Data, 
        unsigned Line, 
        unsigned Equiv, 
        unsigned FilePos, 
        unsigned LinePos) 
  : Data(Data), Line(Line), Equiv(Equiv), FilePos(FilePos), LinePos(LinePos) 
  {}

  bool isWhitespace() const {
    return equiv == -1;
  }

  bool operator==(const Token &t) const {
    return (t.equiv == this->equiv);
  }

  bool operator!=(const Token &t) const {
    return !(operator == (t));
  }
};

#endif // TOKEN_H
