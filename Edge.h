#ifndef EDGE_H_INCLUDED
#define EDGE_H_INCLUDED

#include "Point.h"
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <iostream>

using std::shared_ptr;

class Block;

class Simple_Edge {
public:
	Point p1, p2;
	Simple_Edge(): p1(), p2() {}
	Simple_Edge(Point first, Point second): p1(first), p2(second) {}
	// bool operator <(const Simple_Edge &other) const {
	// 	// use for simplified spanning tree
	// 	// use set so that Simple_Edge will not repeat
    //     if (!(first == other.first)) return first < other.first;
    //     return second < other.second;
	// }
};

class Edge : public Simple_Edge, public std::enable_shared_from_this<Edge> {
public:
	Edge(): Simple_Edge() {}
	Edge(bool v, Pair r, int f, shared_ptr<Block> b): block(b) {
		if (v) { p1 = Point(f, r.min); p2 = Point(f, r.max); }
		else { p1 = Point(r.min, f); p2 = Point(r.max, f); }
	}
	Edge(Point First, Point Second) {
		p1 = First < Second ? First : Second;
		p2 = First < Second ? Second : First;
	}
	Edge(Point First, Point Second, shared_ptr<Block> b): block(b) {
		p1 = First < Second ? First : Second;
		p2 = First < Second ? Second : First;
	}
	Edge(const Edge& other) {
		p1 = other.p1;
		p2 = other.p2;
		block = other.block;
		netID = other.netID;
	}

	shared_ptr<Block> block = nullptr;
	int netID = -99;

	bool isVertical() const {
		return p1.x == p2.x;
	}
	int fixed() const {
		return isVertical() ? p1.x : p1.y;
	}
	Pair ranged() const {
		return isVertical() ? Pair(p1.y, p2.y) : Pair(p1.x, p2.x);
	}

	bool inRange(int const &value) const {
		return ranged().min <= value && ranged().max >= value;
	}
	bool inRange(Edge const &other) const {
		return !(ranged().min >= other.ranged().max || ranged().max <= other.ranged().min);
	}
	bool operator ==(const Edge &other) const {
		return p1 == other.p1 && p2 == other.p2;
	}
	Edge& operator =(const Edge &other) {
		p1 = other.p1;
		p2 = other.p2;
		block = other.block;
		netID = other.netID;
		return *this;
	}
	friend std::ostream& operator <<(std::ostream &os, const Edge &e) {
		os << "(" << e.p1.x << ", " << e.p1.y << "),(" << e.p2.x << ", " << e.p2.y << ")";
		return os; 
	}
};

#endif // EDGE_H_INCLUDED