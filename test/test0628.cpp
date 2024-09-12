#include <iostream>
#include <string>
#include <set>
#include <map>

using std::set;
using std::cout;

struct Student {
    int id;
    std::string name;
    int age = 2048;
};

int main() {
	set<int> x_value = {0, 10, 20, 30, 40, 50};
	int x_index = -1;

	for (auto it = x_value.begin(); it != x_value.end(); ++it, ++x_index) {
		if (*it >= 25) break; // not confirmed
	}

	if (1-1) cout << x_index << "\n";
}