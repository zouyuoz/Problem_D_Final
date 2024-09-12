#include <iostream>
#include <string>
#include <memory>
#include <set>
#include <vector>

using std::shared_ptr;
using std::set;
using std::vector;
using std::make_shared;
using std::cout;

class Cell : public std::enable_shared_from_this<Cell> {
private:
	std::string tag;
	int price;
public:
	Cell();
	Cell(int p): price(p) {}
	int getPrice() const { return price; }
	std::string getTag() const;
	bool operator <(const Cell &other) { return price < other.price; }
};

class Node : public std::enable_shared_from_this<Node> {
private:
	int type;
public:
	Node();
	Node(shared_ptr<Cell> i, shared_ptr<Node> p): cell(i), parent(p) {}
	shared_ptr<Cell> cell;
	shared_ptr<Node> parent = nullptr;
};

bool checkPathIncludeAllMTs(const shared_ptr<Node> &path, vector<set<shared_ptr<Cell>>> MTsCells) {
	shared_ptr<Node> theNode = path;
	while (theNode) {
		cout << theNode->cell->getPrice() << "\n";
		auto itMT = MTsCells.begin();
		bool foundMTCell = 0;
		for (itMT = MTsCells.begin(); itMT != MTsCells.end(); ++itMT) {
			for (auto it = (*itMT).begin(); it != (*itMT).end(); ++it) {
				auto MTCell = *it;
				if (theNode->cell == MTCell) {
					cout << "same cell " << MTCell->getPrice() << "\n";
					foundMTCell = 1;
					break;
				}
				else {
					cout << "not same " << MTCell->getPrice() << "\n";
				}
			}
			if (foundMTCell) break;
		}
		if (foundMTCell) MTsCells.erase(itMT);
		if (MTsCells.empty()) break;
		theNode = theNode->parent;
	}
	return MTsCells.empty();
}

int main(int argc, char* argv[]) {
	vector<int> avec = {1,2,3};
	vector<int> temp = {8,7,6};
	vector<int> nodo = {};
	avec.insert(avec.end(), temp.begin(), temp.end());
	for (auto &ait: avec) cout << ait;
	cout << "\n";
	for (auto &tempit: temp) cout << tempit;
	cout << "\n";
	avec.insert(avec.end(), nodo.begin(), nodo.end());
	cout << avec.size();
	return 0;
	
	shared_ptr<Cell> c_1 = make_shared<Cell>(1);
	shared_ptr<Cell> c_3 = make_shared<Cell>(3);
	shared_ptr<Cell> c_5 = make_shared<Cell>(5);
	shared_ptr<Cell> c_11 = make_shared<Cell>(11);
	shared_ptr<Cell> c_13 = make_shared<Cell>(13);
	shared_ptr<Cell> c_15 = make_shared<Cell>(15);
	shared_ptr<Cell> c_21 = make_shared<Cell>(21);
	shared_ptr<Cell> c_23 = make_shared<Cell>(23);
	shared_ptr<Cell> c_25 = make_shared<Cell>(25);

	vector<set<shared_ptr<Cell>>> MTsCells = {
		{ c_1, c_3, c_5 },
		{ c_11, c_13, c_15 },
		{ c_21, c_23, c_25 }
	};

	shared_ptr<Cell> c_2 = make_shared<Cell>(2);
	shared_ptr<Cell> c_4 = make_shared<Cell>(4);
	shared_ptr<Cell> c_22 = make_shared<Cell>(22);

	shared_ptr<Node> a = make_shared<Node>(c_5, nullptr);
	shared_ptr<Node> b = make_shared<Node>(c_2, a);
	shared_ptr<Node> c = make_shared<Node>(c_4, b);
	shared_ptr<Node> d = make_shared<Node>(c_11, c);
	shared_ptr<Node> e = make_shared<Node>(c_22, d);
	shared_ptr<Node> f = make_shared<Node>(c_13, e);
	if (checkPathIncludeAllMTs(f, MTsCells)) cout << "hellYeah";
	else cout << "shit";
	return 0;
}