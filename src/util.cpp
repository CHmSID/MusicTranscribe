#include "util.h"
#include <vector>

using std::vector;

int read4Bytes(const vector<unsigned char>& v, long& idx){

	int x = (v[idx] << 24) |
	        (v[idx + 1] << 16) |
	        (v[idx + 2] << 8) |
	        (v[idx + 3]);

	idx += 4;
	return x;
}

int read3Bytes(const vector<unsigned char>& v, long& idx){

	int x = (0 << 24) |
	        (v[idx] << 16) |
	        (v[idx + 1] << 8) |
	        (v[idx + 2]);

	idx += 3;
	return x;
}

short read2Bytes(const vector<unsigned char>& v, long& idx){

	short x =   (v[idx] << 16) |
	            (v[idx + 1]);

	idx += 2;
	return x;
}

short read2Bytes(char x, char y){

	return (y << 8) | x;
}

char read1Byte(const vector<unsigned char>& v, long& idx){

	idx++;
	return v[idx - 1];
}
