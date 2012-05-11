//===--- AnchorAnalysis.cpp  ----------------------------------------------===//
//
//                     The NDiff File Comparison Utility
//
//===----------------------------------------------------------------------===//
//
//  This file implements the AnchorAnalysis interface.
//
//===----------------------------------------------------------------------===//

#include "Anchor.h"
#include "SuffixArray.h"
#include "Token.h"
#include <std::vector>
#include <algorithm>

std::vector<Anchor> AnchorAnalysis::findAnchors(const std::vector<Token> &tokArray0,
                                                const std::vector<Token> &tokArray1) {
  const int leftFileLength = tokArray0.size();
  const int rightFileLength = tokArray1.size();

  // Build the suffix array data structure.
  SuffixArray sa;
  sa.init(tokArray0, tokArray1);
  const std::vector<int> indexpoints(sa.indexPoints());
  const std::vector<int> lcpArray(sa.lcpArray());
  const std::vector<int> sortedLcpArray(sa.sortedLcpArray());

	std::vector<Anchor> crossanchs, selfanchs0, selfanchs1;
  std::vector<int>::reverse_iterator lcp(sortedLcpArray.rbegin()), 
                                end(sortedLcpArray.rend());
  for (; lcp != end; ++lcp) {
    // Lookup the index of the max valued lcp in the lcpArray. Using this we can
    // find the two start indexes of the runs, and their length.
		const int k = distance(lcpArray.begin(), 
                           find(lcpArray.begin(), lcpArray.end(), *lcp));
    const int runBegin0 = indexpoints[k]; // Begining of first instance
    const int runBegin1 = indexpoints[k-1]; // Begining of second instance
    const int runLength = lcpArray[k]; // Length of the run.

    // If both runs begin in the first file, construct a self-anchor 
    // for file0 and insert its length into the list if it's maximal.
    if ((runBegin0 < leftFileLength) && (runBegin1 < leftFileLength)) {
      Anchor anchToAdd(runBegin0, runBegin1, run_length);
      if (isMaximal(anchToAdd, selfanchs0)) 
        selfanchs0.push_back(anchToAdd);

      // If both runs begin in the second file, construct a self-anchor 
      // for file1 and insert its length into the list if it's maximal.
    } else if ((leftFileLength < runBegin0) && (leftFileLength < runBegin1)) {
      Anchor anchToAdd(runBegin0 - leftFileLength - 1, 
                       runBegin1 - leftFileLength - 1, 
                       runLength);
      if (isMaximal(anchToAdd, selfanchs1))
        selfanchs1.push_back(anchToAdd);
    } else { 
      // Construct a cross-anchor and insert if it's maximal. It is not
      // always true that the first instance of a cross-anchor will begin
      // in the first file, so we need to check which file the first 
      // instance begins in to setup our anchor correctly.
      if ((runBegin0 < leftFileLength) && (leftFileLength < runBegin1)) {
        Anchor anchToAdd(runBegin0, runBegin1 - leftFileLength - 1, runLength);
        if (isMaximal(anchToAdd, crossanchs))
          crossanchs.push_back(anchToAdd);
      } else { // First instance begins in the second file!!
        Anchor anchToAdd(runBegin1, runBegin0 - leftFileLength - 1, runLength);
        if (isMaximal(anchToAdd, crossanchs))
          crossanchs.push_back(anchToAdd);
      }
    }
		// Invalidate the current sa.lcpArray() entry.
		lcpArray[k] = -1;
	}

  // Some anchors might have been identifed because we were looking at such 
  // small blocks that the probability they will be common to both files is 
  // high, rather than because these are actually two common blocks preserved
  // across. Detect them now and avoid considering them for the rest of the 
  // comparison algorithm.
  discardConfusingAnchors(selfanchs0, selfanchs1, crossanchs);  
	return crossanchs;
}

void AnchorAnalysis::discardConfusingAnchors(const std::vector<Anchor> &selfAnchs0, 
                                             const std::vector<Anchor> &selfAnchs1, 
                                             std::vector<Anchor> &crossAnchs) {
  discardConfusingAnchorsI(selfAnchs0, selfAnchs1, crossAnchs);
  std::vector<Anchor> perm0(crossAnchs), perm1(crossAnchs);
  sort(perm0.begin(), perm0.end(), compareFirstInstance);
  sort(perm1.begin(), perm1.end(), compareSecondInstance);
  crossAnchs = (perm0 == perm1) ? perm0 : alignAnchors(perm0, perm1);
}

void AnchorAnalysis::discardConfusingAnchorsI(const std::vector<Anchor> &selfAnchs0, 
                                              const std::vector<Anchor> &selfAnchs1, 
                                              std::vector<Anchor> &crossAnchs) {
  const int maxself0 = !selfAnchs0.empty() ? selfAnchs0.front().length : 0;
  const int maxself1 = !selfAnchs1.empty() ? selfAnchs1.front().length : 0;
  int thresh = (maxself0 > maxself1) ? maxself0 : maxself1; 

  // Discard anchors falling below the computed threshold.
	while (!crossAnchs.empty() && crossAnchs.back().length < thresh)
		crossAnchs.pop_back();
}

void AnchorAnalysis::discardConfusingAnchorsII(const std::vector<Anchor> &selfAnchs0, 
                                               const std::vector<Anchor> &selfAnchs1, 
                                               std::vector<Anchor> &crossAnchs) {
  std::vector<Anchor> lcps;
  lcps.insert(lcps.end(), selfAnchs0.begin(), selfAnchs0.end());
  lcps.insert(lcps.end(), selfAnchs1.begin(), selfAnchs1.end());
  lcps.insert(lcps.end(), crossAnchs.begin(), crossAnchs.end());

	// Set intitial threshold == length of longest anchor.
	int thresh = crossAnchs.front().length, saved_thresh = 0;
	
	do { // Lloyd's algorithm.
		saved_thresh = thresh; // save last computed thresh to compare
		// Partition the anchors into two sets by comparing 
    // their lengths to the current threshold level.
		int m1 = 0, m2 = 0, nBelow = 0, nAbove = 0;
		std::vector<Anchor>::iterator lcp(lcps.begin()), end(lcps.end());
		for (; lcp != end; ++lcp) {
			if ((*lcp).length > thresh) {
				++nAbove;
				m2 += (*lcp).length;
			} else { 
				++nBelow;
				m1 += (*lcp).length;
			}
		}
		// Compute the average anchor length for both sets.
		m1 = (nBelow > 0) ? m1 / nBelow : 0;
		m2 = (nAbove > 0) ? m2 / nAbove : 0;
		thresh = (m1+m2) / 2; // new thresh = the average of m1 and m2
	} while (thresh != saved_thresh);

  // Discard anchors falling below the computed threshold.
	while (!crossAnchs.empty() && crossAnchs.back().length < thresh)
		crossAnchs.pop_back();
}

std::vector<Anchor> alignAnchors(const std::vector<Anchor> &perm0, 
                                 const std::vector<Anchor> &perm1) {
	const int nAnchs = perm0.size();
	std::vector<std::vector<int> > lcs_table(nAnchs+1);
  for (int i = 0, e = nAnchs + 1; i < e; ++i)
    lcs_table[i].resize(nAnchs+1);
	
	for (int i = nAnchs - 1; i >= 0; --i) {
		for (int j = nAnchs - 1; j >= 0; --j) {
			if (perm0[i] == perm1[j]) 
        lcs_table[i][j] = lcs_table[i+1][j+1]+1;
      else 
        lcs_table[i][j] = max(lcs_table[i+1][j], lcs_table[i][j+1]);
    }
	}
  std::vector<Anchor> anchList;
  anchList.reserve(nAnchs);
  for (int i=0, j=0; ((i < nAnchs) && (j < nAnchs)); ) {
    if (perm0[i] == perm1[j]) {
      anchList.push_back(perm0[i]);
      i++; j++;
    } else if (lcs_table[i][j+1] < lcs_table[i+1][j]) {
      ++i;
    } else {
      ++j;
    }
  }
  return anchList;
}

bool AnchorAnalysis::isMaximal(const Anchor &anch, 
                               const std::vector<Anchor> &anchList) {
  const int anch_begin0 = anchToCheck.index0;
  const int anch_begin1 = anchToCheck.index1;
  const int anch_end0 = anchToCheck.index0 + anchToCheck.length;
  const int anch_end1 = anchToCheck.index1 + anchToCheck.length;

  std::vector<Anchor>::const_iterator pAnch(anchList.begin()),
                                 end(anchList.end());
  for (; pAnch != end; ++pAnch) {
    const int base_water_mark0 = (*pAnch).index0;
    const int base_water_mark1 = (*pAnch).index1;
    const int high_water_mark0 = (*pAnch).index0 + (*pAnch).length;
    const int high_water_mark1 = (*pAnch).index1 + (*pAnch).length;

    // Check bounds from first instance.
		if ((base_water_mark0 <= anch_begin0) && (anch_begin0 < high_water_mark0))
      return false;
    if ((base_water_mark0 <= anch_end0) && (anch_end0 < high_water_mark0))
      return false;

    // Check bounds from second instance.
		if ((base_water_mark1 <= anch_begin1) && (anch_begin1 < high_water_mark1))
      return false;
    if ((base_water_mark1 <= anch_end1) && (anch_end1 < high_water_mark1))
      return false;
  }
	return true;
}
