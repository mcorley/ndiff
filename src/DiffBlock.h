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

/// The data structure representing a diff is a Linked list of DiffBlock objects:
/// {DiffBlock(Operation.DELETE, "Hello"), DiffBlock(Operation.INSERT, "Goodbye"),
/// DiffBlock(Operation.EQUAL, " world.")}
/// which means: delete "Hello", add "Goodbye" and keep " world."
enum Operation {
  ERROR, // Should not be used.
  DELETE, // Deletes only: lines taken from just the first file.
  EQUAL, // No changes: lines common to both files.
  INSERT, // Inserts only: lines taken from just the second file.
  SUBST // Both deletes and inserts: a hunk containing both old and new lines.
};

/// Class representing one DiffBlock.
class DiffBlock {
  /// Marks the ranges in the orginal files this DiffBlock was created from.
  int ranges[2][2];

  /// The kind of operation this DiffBlock is for.
  Operation operation_;

  /// The text corresponding to the operation.
  std::vector<Token> text_;

  /// DiffBlock constructor - Create a new DiffBlock object.
  DiffBlock(Operation operation, const std::vector<Token>& text)
    : operation_(operation), text_(text)
  {}

  /// operation - Returns the start index of the first occurence
  Operation operation() const {
    return operation_;
  }

  /// text - Returns the text corresponding to the operation.
  std::vector<Token> text() const {
    return text_;
  }

  bool operator==(const DiffBlock& db) const;
  bool operator!=(const DiffBlock& db) const;
  static std::string opStr(Operation op) const;
};

#endif // DIFFBLOCK_H
