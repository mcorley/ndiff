//===--- SuffixArray.cpp - SuffixArray data structure ---------------------===//
//
//                     The NDiff File Comparison Utility
//
//===----------------------------------------------------------------------===//
//
//  This file implements the SuffixArray interface.
//
//===----------------------------------------------------------------------===//

#include "SuffixArray.h"
#include "Token.h"
#include <cstdio>

void SuffixArray::init(const std::vector<Token> sourceTokenStream,
                       const std::vector<Token> targetTokenStream) {
  // Assign index points to the tokens. Index points are assigned 
  // token by token and hence we can search with the suffix array 
  // at any positions later.
  const int size = sourceTokenStream.size() + targetTokenStream.size() + 2;
  std::vector<int> indexPoints;
  indexPoints.reserve(size);
  for (int i = 0; i < 2; ++i) {
    std::vector<Token> tokStream = (i==0) ? sourceTokenStream : targetTokenStream;
    for (int j = 0, e = tokStream.size(); j != e; ++j)
      indexPoints.push_back(tokStream[j].getHashValue());
    indexPoints.push_back(i); // Sentinel.
  }
  // Run DC3 and compute the lcp array.
  // DC3 requires at least 3 elements of padding at the end.
  indexPoints.resize(size + 3, 0);  
  orderedIdxPoints = DC3(indexPoints);

  // Drop padding.
  indexPoints.resize(size);

  // Compute the lcps
  lcps = computeLCPs(indexPoints, orderedIdxPoints);
  orderedlcps = orderLCPs(lcps);
}

std::vector<int> SuffixArray::DC3(std::vector<int> indexPoints) {
  int n = indexPoints.size() - 3;
  int max = *std::max_element(indexPoints.begin(), indexPoints.end());
  std::vector<int> result;
  result.resize(n);
  DC3(&indexPoints[0], &result[0], n, max);
  return result;
}

void SuffixArray::DC3(int* s, int* SA, int n, int K) {
	int n0=(n+2)/3, n1=(n+1)/3, n2=n/3, n02=n0+n2; 
	int* s12  = new int[n02 + 3];  s12[n02]= s12[n02+1]= s12[n02+2]=0; 
	int* SA12 = new int[n02 + 3]; SA12[n02]=SA12[n02+1]=SA12[n02+2]=0;
	int* s0   = new int[n0];
	int* SA0  = new int[n0];

	// generate positions of mod 1 and mod  2 suffixes
	// the "+(n0-n1)" adds a dummy mod 1 suffix if n%3 == 1
	for (int i=0, j=0; i < n + (n0 - n1);  i++) 
    if (i%3 != 0) 
      s12[j++] = i;

	// lsb radix sort the mod 1 and mod 2 triples
	radixPass(s12 , SA12, s+2, n02, K);
	radixPass(SA12, s12 , s+1, n02, K);  
	radixPass(s12 , SA12, s  , n02, K);

	// find lexicographic names of triples
	int name = 0, c0 = -1, c1 = -1, c2 = -1;
	for (int i = 0;  i < n02;  i++) {
		if (s[SA12[i]] != c0 || s[SA12[i]+1] != c1 || s[SA12[i]+2] != c2) { 
			name++;  
      c0 = s[SA12[i]];  
      c1 = s[SA12[i]+1];  
      c2 = s[SA12[i]+2];
		}
		if (SA12[i] % 3 == 1) { // left half
      s12[SA12[i]/3] = name; 
    } else { // right half
      s12[SA12[i]/3 + n0] = name; 
    } 
	}

	// recurse if names are not yet unique
	if (name < n02) {
		DC3(s12, SA12, n02, name);
		// store unique names in s12 using the suffix array 
		for (int i = 0;  i < n02;  i++) s12[SA12[i]] = i + 1;
	} else // generate the suffix array of s12 directly
		for (int i = 0;  i < n02;  i++) SA12[s12[i] - 1] = i; 

	// stably sort the mod 0 suffixes from SA12 by their first character
	for (int i=0, j=0;  i < n02;  i++) if (SA12[i] < n0) s0[j++] = 3*SA12[i];
	radixPass(s0, SA0, s, n0, K);

	// merge sorted SA0 suffixes and sorted SA12 suffixes
	for (int p=0,  t=n0-n1,  k=0;  k < n;  k++) {
#define GetI() (SA12[t] < n0 ? SA12[t] * 3 + 1 : (SA12[t] - n0) * 3 + 2)
		int i = GetI(); // pos of current offset 12 suffix
		int j = SA0[p]; // pos of current offset 0  suffix
		if (SA12[t] < n0 ? 
				leq(s[i],       s12[SA12[t] + n0], s[j],       s12[j/3]) :
				leq(s[i],s[i+1],s12[SA12[t]-n0+1], s[j],s[j+1],s12[j/3+n0]))
		{ // suffix from SA12 is smaller
			SA[k] = i;  t++;
			if (t == n02) { // done --- only SA0 suffixes left
				for (k++;  p < n0;  p++, k++) SA[k] = SA0[p];
			}
		} else { 
			SA[k] = j;  p++; 
			if (p == n0)  { // done --- only SA12 suffixes left
				for (k++;  t < n02;  t++, k++) SA[k] = GetI(); 
			}
		}  
	} 
	delete [] s12; delete [] SA12; delete [] SA0; delete [] s0; 
}

void SuffixArray::radixPass(int* a, int* b, int* r, int n, int K) {
  int* c = new int[K + 1];                          // counter array
  for (int i = 0;  i <= K;  i++) c[i] = 0;         // reset counters
  for (int i = 0;  i < n;  i++) c[r[a[i]]]++;    // count occurences
  for (int i = 0, sum = 0;  i <= K;  i++) { // exclusive prefix sums
     int t = c[i];  c[i] = sum;  sum += t;
  }
  for (int i = 0;  i < n;  i++) b[c[r[a[i]]]++] = a[i];      // sort
  delete [] c;
}

std::vector<int> SuffixArray::computeLCPs(const std::vector<int> &indexPoints,
                                          const std::vector<int> &orderedIdxPoints) {
  const int n = indexPoints.size();
  std::vector<int> LCPs(n);

  // Initilaze the rank array. 
	std::vector<int> rank(n);
	for (int i = 0, e = n; i < e; ++i)
		rank[orderedIdxPoints[i]] = i;

	int h = 0;
	for (int i = 0, e = n; i < e; ++i) {
		int k = rank[i];
		if (k == 0) {
			LCPs[k] = 0;
    } else {
			int j = orderedIdxPoints[k - 1];
			while (i+h <= n && 
             j+h <= n && 
             indexPoints[i+h] == indexPoints[j+h]) 
        h++;
			LCPs[k] = h;
		}
		if (h > 0) 
      h--;
	}
  return LCPs;
}

std::vector<int> SuffixArray::orderLCPs(const std::vector<int> &LCPs) {
  const int n = LCPs.size();
  std::vector<int> longestFirstLCPs;
  longestFirstLCPs.reserve(n);

  // We can reduce the overhead of sorting the entire vector of lcp values by 
  // eliminating small values below some threshold. Also, in practice we notice
  // that a bulk of the values are 0, which are worthless to keep anyway.
  const int cutoff = 1;
  for (int i = 0; i < n; ++i)
    if (LCPs[i] > cutoff) 
      longestFirstLCPs.push_back(LCPs[i]);
	std::sort(longestFirstLCPs.begin(), longestFirstLCPs.end());
  return longestFirstLCPs;
}

