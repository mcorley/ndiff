//===--- DiffAlgorithm.cpp - ----------------------------------*- C++ -*-===//
//
//                     The NDiff File Comparison Utility
//
//===--------------------------------------------------------------------===//
//
// This file implements the DiffAlgorithm interface.
//
//===----------------------------------------------------------------------===

#include "DiffAlgorithm.h"
#include "DiffBlock.h"
#include "Token.h"

#include <cstdio>
#include <cstdlib>

#include <iostream>

std::list<DiffBlock> DiffAlgorithm::computeDifference(
    const std::vector<Token> &sourceTokenStream, 
    const std::vector<Token> &targetTokenStream) {
  // Create two temporary files from the token data. Tokens are interspersed 
  // with newline characters yielding diff to operate with token granularity.
  // We use the mkstemp() function which generates a unique temporary filename 
  // from template, creates and opens the file, and returns an open file 
  // descriptor for the file. The last six characters of template must be 
  // "XXXXXX" and these are replaced with a string that makes the filename 
  // unique. Since it will be modified, template must not be a string constant, 
  // but should be declared as a character array. Also, since mkstemp sets the 
  // O_EXCL flag, we need to first close the file before calling popen.
  std::string tmpfile[2];
  for (int i = 0; i < 2; ++i) {
    std::vector<Token> tokenStream = (i==0) ? sourceTokenStream : targetTokenStream;
    char tmpname[] = "/tmp/ndiff.XXXXXX";
    FILE *fpt = fdopen(mkstemp(tmpname), "w");
    tmpfile[i] = tmpname;
    for (int j = 0, e = tokenStream.size(); j < e; ++j) {
      fputs(tokenStream[j].getCharData().c_str(), fpt);
      fputc('\n', fpt);
    }
    fclose(fpt);
  }

  // Initilize the diff shell command line. The -a option tells diff to treat 
  // all files as text and compare them line-by-line, i.e. token-by-token, even 
  // if they do not seem to be text. This command is passed to /bin/sh using 
  // the -c flag; interpretation is performed by the shell.
	const std::string command = "diff -a " + tmpfile[0] + " " + tmpfile[1];

  // Create a pipe and fork invoking the shell with the above diff command. The 
  // resulting read-only stream is in the normal diff output format.
  // Normal format output looks like this:
  //  change-command
  //  < from-file-line
  //  < from-file-line...
  //  ---
  //  > to-file-line
  //  > to-file-line...
  // For our purposes, we only need the change-commands, and all other lines
  // from the output can be ignored.
  FILE *diffNormalOut = popen(command.c_str(), "r");
  if (diffNormalOut == NULL) {
    perror(command.c_str());
  }
 	char changecmd[2048];
  std::list<DiffBlock> DBs;
	while (fgets(changecmd, 2048, diffNormalOut)) {
    if ((changecmd[0] == '>') || (changecmd[0] == '<') || (changecmd[0] == '-')) 
      continue;
    processDiff(changecmd, sourceTokenStream, targetTokenStream, DBs);
  }

  // Diff blocks returned by diff don't capture the equalities so we need to
  // take care of this manually to get a more complete diff result.  
  DBs = captureEqualities(DBs, sourceTokenStream, targetTokenStream);

  // Cleanup and return the list of DiffBlocks.
  pclose(diffNormalOut);
  for (int i = 0; i < 2; ++i)
    remove(tmpfile[i].c_str());
  return DBs;
}

std::list<DiffBlock> DiffAlgorithm::captureEqualities(
      const std::list<DiffBlock> &DBs,
      const std::vector<Token> &sourceTokenStream, 
      const std::vector<Token> &targetTokenStream) {
  // Pointers into the source and target token streams that help us identify 
  // the equalities we failed to capture during the diff algorithm. At any given
  // time during the execution of this method, the pointers are at the start of
  // a pure delete, a pure insert, or a sequence of common tokens we need
  // capture followed by a deletion or insertion.
  std::vector<Token>::const_iterator srcStreamPtr(sourceTokenStream.begin());
  std::vector<Token>::const_iterator tgtStreamPtr(targetTokenStream.begin());

  std::list<DiffBlock> result; // Results go here.
  std::list<DiffBlock>::const_iterator i(DBs.begin()), e(DBs.end());
  for (; i != e; ++i) {
    // Reference to the current DiffBlock.
    const DiffBlock &DB = *i;

    // If the current DiffBlock represents deleted tokens and the srcStreamPtr
    // is at the start of these tokens, we have a pure delete. That is, there
    // are no common tokens we need to capture.
    if (DB.getOperation() == DELETE && 
        DB.getTokens().front() == *srcStreamPtr) {
      result.push_back(DB);
      srcStreamPtr += DB.getTokens().size();
      continue;
    }

    // If the current DiffBlock represents inserted tokens and the tgtStreamPtr
    // is at the start of these tokens, we have a pure insertion. That is, there
    // are no common tokens we need to capture.
    if (DB.getOperation() == INSERT && 
        DB.getTokens().front() == *tgtStreamPtr) {
      result.push_back(DB);
      tgtStreamPtr += DB.getTokens().size();
      continue;
    }

    // Before this DiffBlock, there are common tokens to both token streams we
    // need to capture and create a new DiffBlock for. Both the source and
    // target stream pointers point to the start of the equality and it runs
    // until we reach the begining of the current DiffBlock. Walk this run.
    std::vector<Token> equality; 
    while (srcStreamPtr < sourceTokenStream.end() &&
           tgtStreamPtr < targetTokenStream.end() &&
           *srcStreamPtr < DB.getTokens().front()) {
      equality.push_back(*srcStreamPtr);
      ++srcStreamPtr;
      ++tgtStreamPtr;
    }
    result.push_back(DiffBlock(EQUAL, equality));

    // Add the current DiffBlock and increment the appropirate stream pointer.
    if (DB.getOperation() == DELETE) 
      srcStreamPtr += DB.getTokens().size();
    else 
      tgtStreamPtr += DB.getTokens().size();
    result.push_back(DB);
  }

  return result;
}

void DiffAlgorithm::processDiff(const std::string &changecmd, 
                                const std::vector<Token> &sourceTokenStream, 
                                const std::vector<Token> &targetTokenStream, 
                                std::list<DiffBlock> &DBs) {
  int ranges[2][2];
  Operation op = processDiffControl(changecmd, ranges);
  std::vector<Token> deleted, inserted;

  switch (op) {
    case DELETE: {
      for (int i = ranges[0][0] - 1; i < ranges[0][1]; ++i) 
        deleted.push_back(sourceTokenStream[i]);
      DBs.push_back(DiffBlock(DELETE, deleted));
      return;
    }
    case INSERT: {
      for (int i = ranges[1][0] - 1; i < ranges[1][1]; ++i) 
        inserted.push_back(targetTokenStream[i]);
      DBs.push_back(DiffBlock(INSERT, inserted));
      return;
    }    
    case SUBST: {
      for (int i = ranges[0][0] - 1; i < ranges[0][1]; ++i) 
        deleted.push_back(sourceTokenStream[i]);
      DBs.push_back(DiffBlock(DELETE, deleted));

      for (int i = ranges[1][0] - 1; i < ranges[1][1]; ++i) 
        inserted.push_back(targetTokenStream[i]);
      DBs.push_back(DiffBlock(INSERT, inserted));
      return;
    }
    default:
      return; // Bad format 
  }
}

Operation DiffAlgorithm::processDiffControl(const std::string &changecmd, 
                                            int ranges[2][2]) {
  const char *s = changecmd.c_str();
  // Read first set of digits 
  s = readnum(skipwhite(s), &ranges[0][0]);
  if (!s)
    return ERROR;

  // Was that the only digit? 
  s = skipwhite(s);
  if (*s == ',') {
    s = readnum(s + 1, &ranges[0][1]);
    if (!s)
      return ERROR;
  } else { 
    ranges[0][1] = ranges[0][0];
  }

  // Get the letter (operation)
  Operation op;
  s = skipwhite(s);
  switch (*s) {
    case 'a':
      op = INSERT;
      break;
    case 'c':
      op = SUBST;
      break;
    case 'd':
      op = DELETE;
      break;
    default:
      return ERROR; // Bad format 
  }
  s++; // Past letter 

  // Read second set of digits 
  s = readnum(skipwhite(s), &ranges[1][0]);
  if (!s)
    return ERROR;

  // Was that the only digit?
  s = skipwhite(s);
  if (*s == ',') {
    s = readnum(s + 1, &ranges[1][1]);
    if (!s)
      return ERROR;
    s = skipwhite(s); // To move to end
  } else {
    ranges[1][1] = ranges[1][0];
  }

  return op;
}

const char* DiffAlgorithm::skipwhite(const char *s) {
  while (*s == ' ' || *s == '\t') 
    s++;
  return s;
}
  
const char* DiffAlgorithm::readnum(const char *s, int *pnum) {
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
