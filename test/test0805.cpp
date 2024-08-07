#include <vector>
#include <iostream>
#include <set>
#include <string>
#include <memory>

using std::cout;
using std::vector;
using std::string;
using std::shared_ptr;
using std::unique_ptr;
using std::make_shared;
using std::make_unique;

class node {
public:
	int x, y;
	double weight = 0;
	node (int X, int Y): x(X), y(Y) {}
	node (int X, int Y, double w): x(X), y(Y), weight(w) {}
};

class node_manager {
public:
	// 什麼樣的資料型態比較適合儲存這些資料
	// 且利於未來的讀取與更動
	node_manager() {}
	void addNode(const node&) {}
};

class mm;

class mm_reacher {
public:
	shared_ptr<mm> mm_ptr = nullptr;
	mm_reacher() {}
};

class mm : public std::enable_shared_from_this<mm> {
public:
    int x, y;
    double weight = 0;
    mm(int X, int Y, double w) : x(X), y(Y), weight(w) {}
    mm(int X, int Y) : x(X), y(Y) {}

    vector<mm_reacher> mm_reacher_vec;
    void set_mm_ptr() {
        for (int i = 0; i < 10; ++i) {
            mm_reacher temp_mm_reacher;
            temp_mm_reacher.mm_ptr = shared_from_this();
            mm_reacher_vec.push_back(temp_mm_reacher);
        }
    }
};

class Point {
public:
    Point(int x = 99, int y = 99) : x(x), y(y) {}

    int x, y;
	
    bool operator ==(const Point &other) const {
    	return (x == other.x && y == other.y);
    }

	bool operator<(const Point &other) const {
		if (x + y < other.x + other.y) return 1;
		return x < other.x;
	}
};

class Terminal {
public:
	Terminal() {}
	Terminal(string Name, Point Coord) : name(Name), coord(Coord) {}
	string name;
	Point coord;

	bool operator<(const Terminal &other) const {
        if (coord < other.coord) return true;
        if (other.coord < coord) return false;
        return name < other.name;
    }
};

int main() {
	std::set<Terminal> a;
	a.insert(Terminal("a_block", Point(1, 2)));
	a.insert(Terminal("a_block", Point(2, 1)));
	a.insert(Terminal("a_block", Point(0, 3)));
	a.insert(Terminal("b_block", Point(2, 1)));

	cout << a.count(Terminal("a_block", Point(2, 1))) << "\n";

	cout << a.begin()->coord.y << "\n";

	for (auto i = a.begin(); i != a.end(); ++i) {
		Terminal the = *i;
		cout << the.name << ": " << the.coord.x << ", " << the.coord.y << "\n";
	}

	return 0;
	
    vector<shared_ptr<mm>> hi;
    for (int i = 0; i < 10; ++i) {
        shared_ptr<mm> temp_mm = make_shared<mm>(i, i + 2);
        temp_mm->set_mm_ptr();
        hi.push_back(temp_mm);
    }
	vector<mm_reacher> hello;
	for (auto const &m: hi) {
		for (auto const &r: m->mm_reacher_vec) {
			hello.push_back(r);
		}
	}

	for (auto const &r: hello) {
		cout << r.mm_ptr->x << ", " << r.mm_ptr->y << "\n";
	}

    cout << "\ndone.";
    return 0;
}