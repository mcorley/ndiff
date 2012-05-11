//===--- TokenLexer.h - TokenLexer interface ------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file defines the TokenLexer interface.
//
//===----------------------------------------------------------------------===

#ifndef TOKENLEXER_H
#define TOKENLEXER_H

/// TokenLexer - This implements a lexer that returns tokens from a character
///              stream.
class TokenLexer {
  /// Equivalence classes 0 and 1 are permanently safe for sentinels.
	/// Real equivalence classes start at 2
  static int equivsIndex = 2;

  /// Each token is mapped to a unique equivilence class.
  std::map<std::string, int> equivclasses;
public:
  /// TokenLexer constructor - Create a new TokenLexer object.
  TokenLexer() {}

  /// tokenize - Convert the stream of characters corresponding to the filename
  ///            into a stream of tokens. Reduce the tokens to a string of hashes 
  ///            where each Unicode character represents one token.
  vector<Token> tokenize(const std::string &filename);
};

#endif // TOKENLEXER_H
