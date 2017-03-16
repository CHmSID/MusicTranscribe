#ifndef UTIL_HPP
#define UTIL_HPP

#include <vector>

using std::vector;

extern int read4Bytes(const vector<unsigned char>& v, long& idx);
extern int read3Bytes(const vector<unsigned char>& v, long& idx);
extern short read2Bytes(const vector<unsigned char>& v, long& idx);
extern short read2Bytes(char x, char y);
extern char* readNBytes(const vector<unsigned char>& v, long& idx, int n);
extern char read1Byte(const vector<unsigned char>& v, long& idx);
#endif
