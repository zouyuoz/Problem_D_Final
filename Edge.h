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

class Edge : public std::enable_shared_from_this<Edge> {
public:
	Edge(): first(), second() {}
	Edge(bool v, Pair r, int f, shared_ptr<Block> b): block(b) {
		if (v) { first = Point(f, r.min); second = Point(f, r.max); }
		else { first = Point(r.min, f); second = Point(r.max, f); }
	}
	Edge(Point First, Point Second) {
		first = First < Second ? First : Second;
		second = First < Second ? Second : First;
	}
	Edge(Point First, Point Second, shared_ptr<Block> b): block(b) {
		first = First < Second ? First : Second;
		second = First < Second ? Second : First;
	}
	Edge(const Edge& other) {
		first = other.first;
		second = other.second;
		block = other.block;
		netID = other.netID;
	}

	Point first, second;
	shared_ptr<Block> block = nullptr;
	int netID = -99;

	bool isVertical() const { return first.x == second.x; }
	int fixed() const { return isVertical() ? first.x : first.y; }
	Pair ranged() const { return isVertical() ? Pair(first.y, second.y) : Pair(first.x, second.x); }

	bool inRange(int const &value) const { return ranged().min <= value && ranged().max >= value; }
	bool inRange(Edge const &other) const { return !(ranged().min >= other.ranged().max || ranged().max <= other.ranged().min); }
	// bool overlap(Edge const &other) const { return isVertical() == other.isVertical() && std::abs(fixed() - other.fixed()) < 0.0011 && inRange(other); }
	bool operator ==(const Edge &other) const { return first == other.first && second == other.second; }
	Edge& operator =(const Edge &other) {
		first = other.first;
		second = other.second;
		block = other.block;
		netID = other.netID;
		return *this;
	}
	friend std::ostream& operator <<(std::ostream &os, const Edge &e) {
		os << "(" << e.first.x << ", " << e.first.y << "),(" << e.second.x << ", " << e.second.x << ")";
		return os; 
	}
};

#endif // EDGE_H_INCLUDED