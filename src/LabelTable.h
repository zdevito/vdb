#ifndef _LABEL_TABLE_H
#define _LABEL_TABLE_H
#include <vector>
#include <map>
#include "Frame.h"
static GLubyte category_colors[][3] = {
{114, 158, 206},
{255, 158, 74},
{103, 191, 92}, 
{237, 102, 93},
{173, 139, 201},
{168, 120, 110},
{237, 151, 202},
{162, 162, 162},
{205, 204, 93},
{109, 204, 218}, 
{144, 169, 202},
{225, 157, 90},
{122, 193, 108},
{225, 122, 120},
{197, 176, 213},
{196, 156, 148},
{247, 182, 210},
{199, 199, 199},
{219, 219, 141},
{158, 218, 229}
};
static int N_CATEGORY_COLORS = 20;

struct LabelTable {
	void colorFor(int string, Color * result) {
		if(table.count(string) == 0) {
			table[string] = names.size();
			names.push_back(string);
		}
		int idx = table[string] % N_CATEGORY_COLORS;
		for(int i = 0; i < 3; i++) {
			result->c[i] = category_colors[idx][i] / 255.f;
		}
	}
	void clear() {
		table.clear();
		names.clear();
	}
	std::map<int, int> table;
	std::vector<int> names;
};

#endif