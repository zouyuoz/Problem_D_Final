#include <functional>
#include <iostream>
#include <set>
#include <vector>
#include <unordered_map>

#include "Point.h"
#include "Edge.h"

using std::vector;
using std::set;
using std::cout;

void kindaAlgorithm(vector<vector<Point>> &all, int index = 0) {
	if (all[0].size() - 1 == index) return;
	++index;
	// cout << "iter: " << index << "\n";
    set<Point> nextsFirst;
    for (const auto &single : all) {
        nextsFirst.insert(single[index]);
    }

	// cout << "> in index " << index << ", " << nextsFirst.size() << " nums";
	// for (auto const p: nextsFirst) cout << ", " << p;
	// cout << "\n";

    for (const auto &p : nextsFirst) {
        cout << p << "-> ";
        vector<vector<Point>> nexts;
        for (const auto &single : all) {
            if (index < single.size() && single[index] == p) {
                nexts.push_back(single);
            }
        }
		// cout << nexts.size() << " vectors' " << index << "th num is " << p << "\n";
        // elements in nexts have same value of vec[index] (and previous ones)
        kindaAlgorithm(nexts, index);
		cout << "\n";
    }
	// cout << "end of iter " << index << "\n";
	return;
}

void kindaAlgorithm(const vector<vector<int>> &all, int element = -99, int index = -1) {
	if (all[0].size() - 1 == index) return;
	++index;
	// cout << "iter: " << index << "\n";
    set<int> nextsFirst;
    for (const auto &single : all) {
        nextsFirst.insert(single[index]);
    }

	// cout << "> in index " << index << ", " << nextsFirst.size() << " nums";
	// for (auto const p: nextsFirst) cout << ", " << p;
	// cout << "\n";

    for (const auto &p : nextsFirst) {
        if (index) cout << element << "->" << p << "\n";
        vector<vector<int>> nexts;
        for (const auto &single : all) {
            if (index < single.size() && single[index] == p) {
                nexts.push_back(single);
            }
        }
		// cout << nexts.size() << " vectors' " << index << "th num is " << p << "\n";
        // elements in nexts have same value of vec[index] (and previous ones)
        kindaAlgorithm(nexts, p, index);
    }
	// cout << "end of iter " << index << "\n";
	return;
}

int main() {
	vector<vector<int>> allP = {
		{10, 9, 6},
		{10, 9, 5, 3},
		{10, 9, 5, 2},
		{10, 9, 4},
		{10, 8},
		{10, 7, 1},
	};
	kindaAlgorithm(allP);

	auto helo = std::move(allP);

	for (auto a: allP) {
		for (auto b: a) {
			cout << b;
		}
	}
	
	return 0;
}