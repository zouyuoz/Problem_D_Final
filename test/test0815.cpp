#include <iostream>
#include <vector>
#include <fstream>

using namespace std;

void writeData(int num) {
	ofstream file("output_data.csv", ios::app);
	file << "hello:" << num << "\n";
	file.close();
	return;
}

int main() {
	bool hi = nullptr == NULL;
	cout << hi;
	return 0;
	vector<int> a = { 1, 2, 3, 4, 5 };
	auto b = std::move(a);
	for (const auto &thing: a) {
		cout << thing;
	}
	ofstream file("output_data.csv");
	file << "JUST A TEST\n";
	file.close();
	for (const auto &thing: b) {
		writeData(thing);
	}
	return 0;
}