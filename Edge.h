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

class Edge {
public:
	Edge(){}
	Edge(bool v, Pair r, int f, shared_ptr<Block> b): block(b) {
		if (v) { first = Point(f, r.min); second = Point(f, r.max); }
		else { first = Point(r.min, f); second = Point(r.max, f); }
	}
	Edge(Point First, Point Second): first(First), second(Second) {}
	Edge(Point First, Point Second, shared_ptr<Block> b): first(First), second(Second), block(b) {}

	Point first, second;
	shared_ptr<Block> block = nullptr;

	bool isVertical() const { return first.x == second.x; }
	int fixed() const { return isVertical() ? first.x : first.y; }
	Pair ranged() const { return isVertical() ? Pair(first.y, second.y) : Pair(first.x, second.x); }

	bool is_feedthroughable;
	bool is_BPR;
	bool is_TRENN;
	// not sure have an variable to record the charactor of this edge (like TBENN, BPR, etc.)

	bool inRange(int const &value) const { return ranged().min <= value && ranged().max >= value; }
	bool inRange(Edge const &other) const { return !(ranged().min >= other.ranged().max || ranged().max <= other.ranged().min); }
	// bool overlap(Edge const &other) const { return isVertical() == other.isVertical() && std::abs(fixed() - other.fixed()) < 0.0011 && inRange(other); }
	bool operator ==(const Edge &other) const { return this->fixed() == other.fixed(); }
};

#endif // EDGE_H_INCLUDED