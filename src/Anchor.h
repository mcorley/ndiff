//===--- Anchor.h - A Common Substring of Tokens---------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file defines the Anchor interface.
//
//===----------------------------------------------------------------------===

#ifndef ANCHOR_H
#define ANCHOR_H

class Anchor {
  /// Index marking the start of this Anchor in the source.
  int source;

  /// Index marking the start of this Anchor in the target.
  int target;
  
  /// Number of tokens identified in the run.
  int len;
public:
  /// Anchor constructor - Create a new Anchor object.
  Anchor(int idx0, int idx1, int len) 
    : source(idx0), target(idx1), len(len) {
  }

  bool operator==(const Anchor &rhs) const { 
    return (len == rhs.len && source == rhs.source && target == rhs.target);
  }
  
  bool operator<(const Anchor &rhs) const { return len < rhs.len; }
  bool operator<=(const Anchor &rhs) const { return len <= rhs.len; }
  bool operator!=(const Anchor &rhs) const { return !(*this == rhs); }
  bool operator>(const Anchor &rhs) const { return rhs < *this; }
  bool operator>=(const Anchor &rhs) const { return rhs <= *this; }

  /// sourceIdx - Returns the index designating the start of this Anchor 
  ///             in the source token datastream.
  int sourceIdx() const { return source; }

  /// targetIdx - Returns the index designating the start of this Anchor 
  ///             in the target token datastream.
  int targetIdx() const { return target; }

  /// sourceIdxEnd - Returns the index designating the end of this Anchor 
  ///                in the source token datastream.
  int sourceIdxEnd() const { return source + len; }

  /// targetIdxEnd - Returns the index designating the end of this Anchor 
  ///                in the target token datastream.
  int targetIdxEnd() const { return target + len; }

  /// length - Returns the number of tokens identified in this Anchor.
  int length() const { return len; } 
};

/// compareSourceIndex - Returns true if the sequence identified by a1 occurs
/// before the sequence identified by a2 in the source token stream.
static bool compareSourceIndex(const Anchor &a1, const Anchor &a2) {
  return (a1.sourceIdx() < a2.sourceIdx());
}
  
/// compareTargetIndex - Returns true if the sequence identified by a1 occurs
/// before the sequence identified by a2 in the target token stream
static bool compareTargetIndex(const Anchor &a1, const Anchor &a2) {
  return (a1.targetIdx() < a2.targetIdx());
}

#endif // ANCHOR_H
