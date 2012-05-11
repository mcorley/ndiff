//===--- Anchor.h - Anchor interface --------------------------*- C++ -*-===//
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
  /// Start index of the first occurence of this Anchor.
  unsigned StartIndex0;

  /// Start index of the second occurence of this Anchor.
  unsigned StartIndex1;
  
  /// The number of tokens in the run.
  unsigned Length;
public:
  /// Anchor constructor - Create a new Anchor object.
  Anchor(unsigned StartIndex0, unsigned StartIndex1, unsigned Length)
    : StartIndex0(StartIndex0), StartIndex1(StartIndex1), Length(Length)
  {}

  /// startIndex0 - Returns the start index of the first occurence 
  //                of this Anchor.
  unsigned startIndex0() const {
    return StartIndex0;
  }

  /// startIndex1 - Returns the start index of the second occurence 
  //                of this Anchor.
  unsigned startIndex1 const {
    return StartIndex1;
  }

  /// length - Returns the number of tokens in the run.
  unsigned length const {
    return Length;
  }

  /// Is this Anchor equivalent to another Anchor?
  bool operator==(const Anchor& A) const {
    return (A.StartIndex0 == this->StartIndex0) && 
      (A.StartIndex1 == this->StartIndex1) && 
      (A.Length == this->Length);
  }

  /// Is this Anchor not equivalent to another Anchor?
  bool operator!=(const Anchor& A) const {
    return !(operator == (A));
  }

  /// Compare anchors by their length.
  bool operator<(const Anchor& A) const {
    return (this->Length < A.Length);
  }
};

#endif // ANCHOR_H
