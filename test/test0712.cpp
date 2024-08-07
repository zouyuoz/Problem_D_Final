#include <iostream>
#include "Edge.h"

using namespace std;

void Print(const Edge &e) {
	cout << "["<<e.ranged().min<<", "<<e.ranged().max<<"], "<<e.fixed();
	return;
}

void Modifies(Edge &e) {
	e.first.y++;
	e.second.y++;
	return;
}

int main() {
	double a = 1, b = 10;
	vector<Edge> blockEdges;
	for (size_t i = 0; i < 10; i++) {
		blockEdges.push_back(Edge(Point(a++, 0), Point(b++, 0)));
		cout << i <<": ";
		Print(blockEdges[i]);
		cout << endl;
	}
	
	vector<Edge> referenceEdges;
	for (size_t i = 0; i < 10; i += 2) {
		Edge &testEdge = blockEdges[i];
		Modifies(testEdge);
		referenceEdges.push_back(testEdge);
	}

	for(const Edge &e : blockEdges) { Print(e); cout << endl;}

	cout << "\ndone";
	return 0;
}