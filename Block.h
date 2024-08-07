#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#include "Zone.h"
#include "Edge.h"
#include <iostream>
#include <memory>

using std::cout;

class EdgeNetNum {
public:
	EdgeNetNum(Edge e, int n) : edge(e), net_num(n) {}
	Edge edge;
	// net.num < net_num -> can use this edge
	int net_num;
};

class Block : public Zone, public std::enable_shared_from_this<Block> {
public:
	Block();
	Block(string);
	Block(Block const &);

	Point coordinate;
	Point top_coordinate = Point(0, 0);
	string facingFlip;
	string blkID;

	int through_block_net_num;
	vector<shared_ptr<EdgeNetNum>> through_block_edge_net_num;
	vector<Edge> block_port_region;
	bool is_feedthroughable;
	bool is_tile;
	vector<Edge> edges;

	void verticesToEdges();
	// void addENN(const Edge&, const int&);
	// void addBPR(const Edge&);
	void transposeAllVertices();
	void showBlockInfo() const;
	bool enclose(const Point) const;
	bool canThrough(bool, Point, Pair = Pair(0, 0));
	bool operator <(const Block&) const;
	// 還沒寫net穿過block會減少through_block_net_num
};

#endif // BLOCK_H_INCLUDED