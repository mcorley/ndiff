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
#include "AnchorAnalysis.h"
#include "SuffixArray.h"
#include "Token.h"

#include <algorithm>

std::vector<Anchor> AnchorAnalysis::findAnchors(
    const std::vector<Token> sourceTokenStream,
    const std::vector<Token> targetTokenStream) {

  const int sourceTokenStreamSize = sourceTokenStream.size();
  const int targetTokenStreamSize = targetTokenStream.size();

  SuffixArray sa(sourceTokenStream, targetTokenStream);
  std::vector<int> indexPoints = sa.orderedIndexPoints();
  std::vector<int> LCPs        = sa.LCPs();
  std::vector<int> orderedLCPs = sa.orderedLCPs();

  std::vector<Anchor> crossAnchors, sourceAnchors, targetAnchors;

  std::vector<int>::reverse_iterator 
    i(orderedLCPs.rbegin()), e(orderedLCPs.rend());
  for (; i != e; ++i) {
    const int LCP = *i;
    const int idx = distance(LCPs.begin(), find(LCPs.begin(), LCPs.end(), LCP));
    const int x = indexPoints[idx];
    const int y = indexPoints[idx - 1];
    const int len = LCPs[idx];

    // First check for self-anchors. Self-anchors represent the common 
    // substrings found when considering a file with itself. They give us a 
    // way to express self similarity by providing a measure on the 
    // distribution of common substrings for that file. By getting an idea of 
    // how similar two files are with themselves, we can get a better feel for 
    // the statistical significance of common substrings between two files to
    // later set some threshold level.
    //
    // We say an anchor is a self anchor from the source stream when both
    // indexes x and y are found in the source stream.
    //
    // We say an anchor is a self anchor from the target stream when both
    // indexes x and y are found in the target stream.
    if ((x < sourceTokenStreamSize) && (y < sourceTokenStreamSize)) {
      Anchor srcAnch(x, y, len);
      if (isMaximal(srcAnch, sourceAnchors)) 
        sourceAnchors.push_back(srcAnch);
    } else if ((sourceTokenStreamSize < x) && (sourceTokenStreamSize < y)) {
      Anchor tgtAnch(x - sourceTokenStreamSize - 1, y - sourceTokenStreamSize - 1, len);
      if (isMaximal(tgtAnch, targetAnchors)) 
        targetAnchors.push_back(tgtAnch);
    } else {
      // Check for a maximal cross anchor. Cross anchors represent common 
      // substrings between two files. We need to consider two cases here:
      //  1) When x is in the source stream and y is in the target stream
      //  2) When y is in the source stream and x is in the target stream
      if ((x < sourceTokenStreamSize) && (sourceTokenStreamSize < y)) {
        Anchor anch(x, y - sourceTokenStreamSize - 1, len);
        if (isMaximal(anch, crossAnchors))
          crossAnchors.push_back(anch);
      }
      else if ((y < sourceTokenStreamSize) && (sourceTokenStreamSize < x)) {
        Anchor anch(y, x - sourceTokenStreamSize - 1, len);
        if (isMaximal(anch, crossAnchors))
          crossAnchors.push_back(anch);
      }
    }
    // Invalidate the current LCP entry.
		LCPs[idx] = -1;
  }

  // Some anchors might have been identifed because we were looking at such 
  // small blocks that the probability they will be common to both files is 
  // high, rather than because these are actually two common blocks preserved
  // across. Detect them now and avoid considering them for the rest of the 
  // comparison algorithm.
  discardConfusingAnchors(sourceAnchors, targetAnchors, crossAnchors);  
	return crossAnchors;
}

void AnchorAnalysis::discardConfusingAnchors(const std::vector<Anchor> &sourceAnchors, 
                                             const std::vector<Anchor> &targetAnchors, 
                                             std::vector<Anchor> &crossAnchors) {
  discardConfusingAnchorsI(sourceAnchors, targetAnchors, crossAnchors);
  std::vector<Anchor> perm0(crossAnchors), perm1(crossAnchors);
  std::sort(perm0.begin(), perm0.end(), compareSourceIndex);
  std::sort(perm1.begin(), perm1.end(), compareTargetIndex);
  crossAnchors = (perm0 == perm1) ? perm0 : alignAnchors(perm0, perm1);
}

void AnchorAnalysis::discardConfusingAnchorsI(const std::vector<Anchor> &sourceAnchors, 
                                              const std::vector<Anchor> &targetAnchors, 
                                              std::vector<Anchor> &crossAnchors) {
  const int maxself0 = !sourceAnchors.empty() ? sourceAnchors.front().length() : 0;
  const int maxself1 = !targetAnchors.empty() ? targetAnchors.front().length() : 0;
  int thresh = (maxself0 > maxself1) ? maxself0 : maxself1; 

  // Discard anchors falling below the computed threshold.
	while (!crossAnchors.empty() && crossAnchors.back().length() < thresh)
		crossAnchors.pop_back();
}

void AnchorAnalysis::discardConfusingAnchorsII(const std::vector<Anchor> &sourceAnchors, 
                                               const std::vector<Anchor> &targetAnchors, 
                                               std::vector<Anchor> &crossAnchors) {
  std::vector<Anchor> anchs;
  anchs.insert(anchs.end(), sourceAnchors.begin(), sourceAnchors.end());
  anchs.insert(anchs.end(), targetAnchors.begin(), targetAnchors.end());
  anchs.insert(anchs.end(), crossAnchors.begin(), crossAnchors.end());

	int thresh = crossAnchors.front().length(), savedThresh = 0;
	do {		
		// Partition the anchors into two sets by comparing 
    // their lengths to the current threshold level.
    savedThresh = thresh;
		int m1 = 0, m2 = 0, nBelow = 0, nAbove = 0;
		std::vector<Anchor>::iterator i(anchs.begin()), e(anchs.end());
		for (; i != e; ++i) {
      const int len = (*i).length();
			if (len > thresh) { ++nAbove;  m2 += len; } 
      else { ++nBelow;  m1 += len; }
		}
		m1 = (nBelow > 0) ? m1 / nBelow : 0;
		m2 = (nAbove > 0) ? m2 / nAbove : 0;
		thresh = (m1+m2) / 2; 
	} while (thresh != savedThresh);

  // Discard anchors falling below the computed threshold.
	while (!crossAnchors.empty() && crossAnchors.back().length() < thresh)
		crossAnchors.pop_back();
}

std::vector<Anchor> AnchorAnalysis::alignAnchors(const std::vector<Anchor> &perm0, 
                                                 const std::vector<Anchor> &perm1) {
	const int nAnchs = perm0.size();
	std::vector<std::vector<int> > LCSTable(nAnchs+1);
  for (int i = 0, e = nAnchs + 1; i < e; ++i)
    LCSTable[i].resize(nAnchs+1);
	
	for (int i = nAnchs - 1; i >= 0; --i) {
		for (int j = nAnchs - 1; j >= 0; --j) {
			if (perm0[i] == perm1[j]) 
        LCSTable[i][j] = LCSTable[i+1][j+1]+1;
      else 
        LCSTable[i][j] = std::max(LCSTable[i+1][j], LCSTable[i][j+1]);
    }
	}
  std::vector<Anchor> anchList;
  anchList.reserve(nAnchs);
  for (int i=0, j=0; ((i < nAnchs) && (j < nAnchs)); ) {
    if (perm0[i] == perm1[j]) {
      anchList.push_back(perm0[i]);
      i++; j++;
    } else if (LCSTable[i][j+1] < LCSTable[i+1][j]) {
      ++i;
    } else {
      ++j;
    }
  }
  return anchList;
}

bool AnchorAnalysis::isMaximal(const Anchor &anch, 
                               const std::vector<Anchor> &anchors) {
  const int firstBegin = anch.sourceIdx(),
            firstEnd = anch.sourceIdx() + anch.length();

  const int secondBegin = anch.targetIdx(),
            secondEnd = anch.targetIdx() + anch.length();

  std::vector<Anchor>::const_iterator i(anchors.begin()), e(anchors.end());
  for (; i != e; ++i) {
    const int firstLowBound = (*i).sourceIdx(),
              firstUpBound = (*i).sourceIdx() + (*i).length();

    const int secondLowBound = (*i).targetIdx(),
              secondUpBound = (*i).targetIdx() + (*i).length();

		if ((firstLowBound <= firstBegin) && 
        (firstBegin < firstUpBound)) 
      return false;
    if ((firstLowBound <= firstEnd) && 
        (firstEnd < firstUpBound)) 
      return false;
		if ((secondLowBound <= secondBegin) && 
        (secondBegin < secondUpBound)) 
      return false;
    if ((secondLowBound <= secondEnd) && 
        (secondEnd < secondUpBound)) 
      return false;
  }
	return true;
}
