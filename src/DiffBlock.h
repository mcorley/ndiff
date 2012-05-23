//===--- DiffBlock.h - DiffBlock interface --------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
// 
//
//===--------------------------------------------------------------------===//
//
// This file defines the DiffBlock interface.
//
//===----------------------------------------------------------------------===

#ifndef DIFFBLOCK_H
#define DIFFBLOCK_H
#include <vector>
#include "Token.h"

//class Token;

/// Various operations supported by diff blocks.
enum Operation {
  ERROR, // Should not be used.
  DELETE, // Deletes only: lines taken from just the first file.
  INSERT, // Inserts only: lines taken from just the second file.
  EQUAL, // No changes: lines common to both files.
  SUBST
};

/// Class representing one DiffBlock.
class DiffBlock {
  Operation operation;
  std::vector<Token> tokenVec;
public:
  /// DiffBlock constructor - Create a new DiffBlock object.
  DiffBlock(Operation op, std::vector<Token> tokVec)
    : operation(op), tokenVec(tokVec) {
  }

  bool operator==(const DiffBlock &rhs) const { 
    return operation == rhs.operation &&
           tokenVec == rhs.tokenVec; 
  }

  bool operator<(const DiffBlock &rhs) const { 
    return tokenVec.front().lexedOffset() < rhs.tokenVec.front().lexedOffset(); 
  }

  bool operator<=(const DiffBlock &rhs) const { 
    return tokenVec.front().lexedOffset() <= rhs.tokenVec.front().lexedOffset(); 
  }

  bool operator!=(const DiffBlock &rhs) const { return !(*this == rhs); }
  bool operator>(const DiffBlock &rhs) const { return rhs < *this; }
  bool operator>=(const DiffBlock &rhs) const { return rhs <= *this; }

  /// getOperation - Returns the operation supported by this diff block.
  const Operation getOperation() const { return operation; }

  /// getTokens - Returns a read-only vector containing the tokens associated 
  /// with this diff block.
  const std::vector<Token> getTokens() const { return tokenVec; }

  /// tokenVector - Returns the tokenVector.
  std::vector<Token>& tokens() { return tokenVec; }
};

#endif // DIFFBLOCK_H
