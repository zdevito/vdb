#ifndef _STRING_TABLE_H
#define _STRING_TABLE_H
#include <map>
#include <string>
#include <assert.h>

struct StringTable {
	StringTable() { 
		next_key = 0;
	}
	int Intern(const char * str) {
		std::string ss = str;
		if(string_to_key.count(ss) == 0) {
			string_to_key[ss] = next_key;
			key_to_string[next_key] = ss;
			next_key++;
		}
		return string_to_key[str];
	}
	const char * Extern(int key) {
		assert(key_to_string.count(key));
		return key_to_string[key].c_str();
	}
	std::map< std::string, int> string_to_key;
	std::map< int, std::string> key_to_string;
	int next_key;
};

#endif