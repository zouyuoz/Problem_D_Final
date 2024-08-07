#include <vector>
//#include "AllZone.h"
#include <iostream>

using namespace std;

constexpr double DX = 0.001;
constexpr double DY = 0.001;

int main() { // 或是說 mikami 的前置作業，不一定是在 main，可能是獨立的函式
	//AllZone allZone(4);
	double a = 0;

	for (double i = 0; i < 1000000; i += DX) {
		a = i;
	}

	cout << "hel" << endl;
}