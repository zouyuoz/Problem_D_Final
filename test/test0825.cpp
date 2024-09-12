#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <set>
#include <vector>

using std::shared_ptr;
using std::set;
using std::vector;
using std::make_shared;
using std::cout;
using std::map;

class Point {
public:
    Point(int x = 99, int y = 99) : x(x), y(y) {}

    int x, y;
	
    bool operator ==(const Point &other) const {
    	return (x == other.x && y == other.y);
    }

	bool operator<(const Point &other) const {
		if (x + y < other.x + other.y) return 1;
		if (x + y > other.x + other.y) return 0;
		return x < other.x;
	}
};

class Pair {
public:
	Pair() : min(0), max(0) {}
	Pair(int Min, int Max) {
		// Min should smaller than Max, 
		// but in case we still check that
		this->min = std::min(Min, Max);
		this->max = std::max(Min, Max);
	}

	int min, max;

	void operator =(Pair const &other) {
		this->min = other.min;
		this->max = other.max;
	}

    bool operator ==(const Pair &other) const {
    	return (min == other.min && max == other.max);
    }

	bool operator <=(const Pair &other) const {
		return (min >= other.min && max <= other .max);
	}
};

class Cell : public std::enable_shared_from_this<Cell> {
public: 
	Cell(Pair X, Pair Y): x(X), y(Y) {
		node = Point((x.min + x.max) / 2, (y.min + y.max) / 2);
	}

    Pair x, y;
	int xIndex, yIndex;
	vector<shared_ptr<Cell>> next;

	Point node;
	bool inBlock();
	bool inNonfeed();
	bool valid() { return 1; /*TODO*/ }
	bool isBPR = 0;
	bool BPR_isVertical; // should always write

    bool enclose(const Point&);
	bool canGo(const shared_ptr<Cell>&);
	bool capacityEnough(int);

    bool operator <=(const Cell &other) const;
	bool operator <(const Cell &other) const;
    friend std::ostream& operator<<(std::ostream& os, const Cell& b);
};

class Cell_Manager {
private:
	set<int> x_value, y_value;
public:
	Cell_Manager() {}
	map<int, map<int, shared_ptr<Cell>>> cells;
	void setXYvalue(set<int>, set<int>);
	set<shared_ptr<Cell>> cellsOnVertex(const Point&);
	shared_ptr<Cell> cellEnclose(const Point&);
	void createCells_();
};

void Cell_Manager::setXYvalue(set<int> X_value, set<int> Y_value) {
	x_value = X_value;
	y_value = Y_value;
	return;
}

void Cell_Manager::createCells_() {
	int x_count = 0;
	for (auto x1 = x_value.begin(); x1 != x_value.end(); ++x1) {
        auto x2 = std::next(x1);
        if (x2 == x_value.end()) break;
		int y_count = -1;

		for (auto y1 = y_value.begin(); y1 != y_value.end(); ++y1) {
			auto y2 = std::next(y1);
			if (y2 == y_value.end()) break;

			Cell tempCell(Pair(*x1, *x2), Pair(*y1, *y2));
			tempCell.xIndex = x_count;
			tempCell.yIndex = ++y_count;
			cells[x_count][y_count] = std::make_shared<Cell>(tempCell);
		}
		++x_count;
    }
}

shared_ptr<Cell> Cell_Manager::cellEnclose(const Point &p) {
	int x_index = -1, y_index = -1;

	for (auto it = x_value.begin(); it != x_value.end(); ++it, ++x_index) {
		if (*it >= p.x) break; // probably
	}

	for (auto it = y_value.begin(); it != y_value.end(); ++it, ++y_index) {
		if (*it >= p.y) break; // probably
	}

	return cells[x_index][y_index];
}

set<shared_ptr<Cell>> Cell_Manager::cellsOnVertex(const Point &p) {
	set<shared_ptr<Cell>> cellsOnVertex;
	int x_index = 0, y_index = 0;

	for (auto it = x_value.begin(); it != x_value.end(); ++it, ++x_index) {
		if (*it == p.x) break; // probably
	}
	for (auto it = y_value.begin(); it != y_value.end(); ++it, ++y_index) {
		if (*it == p.y) break; // probably
	}
	cout << x_index << ", " << y_index << "\n";

	cellsOnVertex.insert(cells[x_index][y_index]);
	cellsOnVertex.insert(cells[x_index][y_index - 1]);
	cellsOnVertex.insert(cells[x_index - 1][y_index]);
	cellsOnVertex.insert(cells[x_index - 1][y_index - 1]);
	return cellsOnVertex;
}

int main(int argc, char* argv[]) {
	set<int> x_value = {0, 3, 5, 7, 10};
	set<int> y_value = {1, 4, 6, 8, 11};

	Cell_Manager test;
	test.setXYvalue(x_value, y_value);
	test.createCells_();
	// for (auto const &outer: test.cells) {
	// 	for (auto const &inner: outer.second) {
	// 		auto const cell = inner.second;
	// 		cout << cell->x.min << ", " << cell->x.max << ", " << cell->y.min << ", " << cell->y.max << "\n";
	// 	}
	// }

	// auto const cell = test.cellEnclose(Point(4, 9));				OK OK OK
	// cout << cell->x.min << ", " << cell->x.max << ", " << cell->y.min << ", " << cell->y.max << "\n";

	auto const four = test.cellsOnVertex(Point(3, 4));
	for (auto const &cell: four) {
		cout << cell->x.min << ", " << cell->x.max << ", " << cell->y.min << ", " << cell->y.max << "\n";
	}

	return 0;
}