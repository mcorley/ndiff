//===--- DiffAlgorithm.cpp - ----------------------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file implements the DiffAlgorithm interface.
//
//===----------------------------------------------------------------------===

#include "diffblock.h"
#include "token.h"
#include <cstdio>
#include <vector>

std::vector<DiffBlock> DiffAlgorithm::computeDifference(const std::vector<Token> &tokArray0, 
                                                        const std::vector<Token> &tokArray1) {
  string tmpfiles[2];
  for (int f = 0; f < 2; ++f) {
    char tmpname[] = "/tmp/ndiff.XXXXXX";
    FILE *fpt = fdopen(mkstemp(tmpname), "w");
    tmpfiles[f] = tmpname;
    for (int i = 0; i < files[f].size(); ++i) {
      fputs(files[f][i].chars.c_str(), fpt);
      fputc('\n', fpt);
    }
    // mkstemp sets the O_EXCL flag preventing us from opening the
    // file again with popen, so we need to first close the file.
    fclose(fpt);
  }

	const string command = "diff -a " + tmpfiles[0] + " " + tmpfiles[1];
  FILE *fpipe = popen(command.c_str(), "r");
  if (fpipe == NULL) {
    perror(command.c_str());
  }
 	char changecmd[2048];
  vector<DiffBlock> block_list;
	while (fgets(changecmd, 2048, fpipe)) {
    if ((changecmd[0] != '>') && (changecmd[0] != '<') && (changecmd[0] != '-')) 
      processDiff(changecmd, files, block_list);
  }
  // Cleanup and return the list of DiffBlocks.
  pclose(fpipe);
  for (int f = 0; f < 2; ++f)
    remove(tmpfiles[f].c_str());
  return block_list;
}

void DiffAlgorithm::processDiff(const std::string &changecmd, 
                                const std::vector<Token> &tokArray0,
                                const std::vector<Token> &tokArray1, 
                                std::vector<DiffBlock> &block_list) {
  DiffBlock db;
  processDiffControl(changecmd, db);
  switch (db.operation) {
    case INSERT: {
      const int index1 = db.ranges[FILE1][RANGE_START];
      const int inserted = db.ranges[FILE1][RANGE_END];
      for (int i = index1 - 1; i < inserted; ++i)
        db.inserted.push_back(filevec[FILE1][i]);
      break;
    }
    case SUBST: {
      const int index0 = db.ranges[FILE0][RANGE_START];
      const int deleted = db.ranges[FILE0][RANGE_END];
      for (int i = index0 - 1; i < deleted; ++i)
        db.deleted.push_back(filevec[FILE0][i]);
      const int index1 = db.ranges[FILE1][RANGE_START];
      const int inserted = db.ranges[FILE1][RANGE_END];
      for (int i = index1 - 1; i < inserted; ++i)
        db.inserted.push_back(filevec[FILE1][i]);
      break;
    }
    case DELETE: {
      const int index0 = db.ranges[FILE0][RANGE_START];
      const int deleted = db.ranges[FILE0][RANGE_END];
      for (int i = index0 - 1; i < deleted; ++i)
        db.deleted.push_back(filevec[FILE0][i]);
      break;
    }
    default:
      return; // Bad format 
  }
  block_list.push_back(db);
}

void DiffAlgorithm::processDiffControl(const std::string &changecmd, 
                                       DiffBlock &db) {
  const char *s = changecmd.c_str();
  // Read first set of digits 
  s = readnum(skipwhite(s), &db.ranges[FILE0][RANGE_START]);
  if (!s)
    return;

  // Was that the only digit? 
  s = skipwhite(s);
  if (*s == ',') {
    s = readnum(s + 1, &db.ranges[FILE0][RANGE_END]);
    if (!s)
      return;
  } else {
    db.ranges[FILE0][RANGE_END] = db.ranges[FILE0][RANGE_START];
  }

  // Get the letter 
  s = skipwhite(s);
  switch (*s) {
    case 'a':
      db.operation = INSERT;
      break;
    case 'c':
      db.operation = SUBST;
      break;
    case 'd':
      db.operation = DELETE;
      break;
    default:
      return; // Bad format 
  }
  s++; // Past letter 

  // Read second set of digits 
  s = readnum(skipwhite(s), &db.ranges[FILE1][RANGE_START]);
  if (!s)
    return;

  // Was that the only digit?
  s = skipwhite(s);
  if (*s == ',') {
    s = readnum(s + 1, &db.ranges[FILE1][RANGE_END]);
    if (!s)
      return;
    s = skipwhite(s); // To move to end
  } else {
    db.ranges[FILE1][RANGE_END] = db.ranges[FILE1][RANGE_START];
  }
}

static inline const char DiffAlgorithm::*skipwhite(const char *s) {
  while (*s == ' ' || *s == '\t') 
    s++;
  return s;
}
  
static inline const char DiffAlgorithm::*readnum(const char *s, int *pnum) {
  unsigned char c = *s;
  int num = 0;

  if (!isdigit(c))
    return 0;

  do {
    num = c - '0' + num * 10;
    c = *++s;
  } while (isdigit(c));

  *pnum = num;
  return s;
}
