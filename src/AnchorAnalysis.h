//===--- AnchorAnalysis.h - AnchorAnalysis interface ----------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file defines the AnchorAnalysis interface.
//
//===----------------------------------------------------------------------===

#ifndef ANCHORANALYSIS_H
#define ANCHORANALYSIS_H

class Anchor;
class Token;

class AnchorAnalysis {
public:
  std::vector<Anchor> findAnchors(const std::vector<Token> &tokArray0,
                                  const std::vector<Token> &tokArray1);

  /// Computes a global threshold level used to eliminate anchors which have 
  /// a length below this value. The goal of this is to remove anchors that 
  /// might have been identified because they are so small the probability 
  /// the blocks are common to both files is close to 1.
  void discardConfusingAnchors(const std::vector<Anchor> &selfAnchs0, 
                               const std::vector<Anchor> &selfAnchs1, 
                               std::vector<Anchor> &crossAnchs);

  /// Given two permutations of a set of Anchors, finds the longest subsequence 
  /// common to both arrangements and returns the subset of Anchors comprising it.
  std::vector<Anchor> alignAnchors(const std::vector<Anchor> &perm0, 
                                   const std::vector<Anchor> &perm1);

  /// firstOccurenceFirst - Returns true if the index of the first occurence of 
  ///                       A1 is before the index of the first occurence of A2.
  bool firstOccurenceFirst(const Anchor &a1, const Anchor &a2) {
    return (a1.index0 < a2.index0);
  }
  
  /// secondOccurenceFirst - Returns true if the index of the second occurence of 
  ///                        A1 is before the index of the second occurence of A2.
  bool secondOccurenceFirst(const Anchor &a1, const Anchor &a2) {
    return (a1.index1 < a2.index1);
  }
  
  /// isMaximal - Returns true if the Anchor Anch is not in AnchList, or not
  ///             contained within any other Anchor in AnchList
  bool isMaximal(const Anchor &anch, const std::vector<Anchor> &anchList);
private:
  /// Computes a global threshold level by taking the length of the first 
  /// cross-anchor that is larger than all self-anchors as the minimum 
  /// requirement. A more liberal threshold is set with this function.
  void discardConfusingAnchorsI(const std::vector<Anchor> &selfAnchs0, 
                                const std::vector<Anchor> &selfAnchs1, 
                                std::vector<Anchor> &crossAnchs);

  /// Computes a global threshold level by running Lloyds algorithm on the set 
  /// of all self and cross anchors discovered. This iterative algorithm is a 
  /// special one-dimensional case of the k-means clustering algorithm, which 
  /// has been proven to converge at a local minimum—meaning that a different 
  /// initial threshold may give a different final result. A more conservative 
  /// threshold is set with this function.
  void discardConfusingAnchorsII(const std::vector<Anchor> &selfAnchs0, 
                                 const std::vector<Anchor> &selfAnchs1, 
                                 std::vector<Anchor> &crossAnchs);
};

#endif // ANCHORANALYSIS_H
