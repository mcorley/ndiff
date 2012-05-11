//===--- SuffixArray.h - SuffixArray interface ------------------*- C++ -*-===
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file defines the SuffixArray interface.
//
//===----------------------------------------------------------------------===

#ifndef SUFFIXARRAY_H
#define SUFFIXARRAY_H

class Token;

/// The SuffixArray data structure is the sorted order of suffixes with pairwise 
/// LCPs of neighboring suffixes. Suffix arrays help lookup of any substring of 
/// a text and indentification of repeated substrings. And it is more compact 
/// than a suffix tree and suitable for storing in secondary memory.
class SuffixArray {
  /// sortedIndexPoints_ - This is the list of sorted indexes of suffixes.
  ///
  std::vector<int> sortedIndexPoints_;
    
  /// lcpArray_ - This is the list of pairwise longest common prefixes 
  ///             of neighboring suffixes.
  ///
  std::vector<int> lcpArray_;

  /// sortedLcpArray_ - This is the list of pairwise longest common prefixes 
  ///                   of neighboring suffixes after they have been sorted.
  ///
  std::vector<int> sortedLcpArray_;
public:
  /// Create a SuffixArray for the specified token streams.
  SuffixArray(const std::vector<Token> &tokArray0, 
              const std::vector<Token> &tokArray1) {
    init(tokArray0, tokArray1);
  }

  /// Initialize this SuffixArray with the specified token streams.
  void init(const std::vector<Token> &tokArray0, 
            const std::vector<Token> &tokArray1);

  /// indexPoints - Return the list of sorted index points.
  std::vector<int> indexPoints() const {
    return sortedIndexPoints_;
  }

  /// lcpArray - Return the list of longest common prefixes.
  std::vector<int> lcpArray() const {
    return lcpArray_;
  }

  /// sortedLcpArray - Return the sorted list of longest common prefixes.
  std::vector<int> sortedLcpArray() const {
    return sortedLcpArray_;
  }

private:
  /// Builds the siffix array with the DC3 (Difference Cover 3) divide and 
  /// conquer algorithm. We closely follow the exposition of the paper by 
  /// Karkkainen-Sanders-Burkhardt that originally proposed this algorithm
  /// in their paper Linear Work Suffix Array Construction published in the 
  /// Journal of the ACM Volume 53 Issue 6, November 2006. Implementation 
  /// provided by the authors at 
  ///       http://www.mpi-inf.mpg.de/~sanders/programs/suffix/
  void build(int* s, int* SA, int n, int K);

  /// Computes the length of the longest common prefix between neighboring 
  /// entries of the intermediate array. We use the algorithm of Kasai et al. 
  /// for the linear time computation. The algorithm used comes from the paper 
  /// "Linear-Time Longest-Common-Prefix Computation in Suffix Arrays and Its 
  /// Applications" by: Toru Kasai, Gunho Lee, Hiroki Arimura, Setsuo Arikawa, 
  /// and Kunsoo Park in Combinatorial Pattern Matching (2001), pp. 181-192.
  std::vector<int> computeLCPs(const std::vector<int> &tokArray, 
                               const std::vector<int> &sortedIndexes);  
  
  /// Stably sort src[0..n-1] to dst[0..n-1] with keys in 0..K from r.
  void radixPass(int *a, int *b, int *r, int n, int K);

  /// Lexicographic order for pairs.
  static inline bool leq(int a1, int a2, int b1, int b2) {
    return(a1 < b1 || a1 == b1 && a2 <= b2); 
  }

  /// Lexicographic order for triples.
  static inline bool leq(int a1, int a2, int a3, int b1, int b2, int b3) {
    return(a1 < b1 || a1 == b1 && leq(a2,a3, b2,b3)); 
  }
};

#endif // SUFFIXARRAY_H
