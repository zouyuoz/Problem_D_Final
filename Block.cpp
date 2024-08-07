#include "Block.h"
#include <climits>
#include <iostream>

Block::Block(){}

Block::Block(string border) {
	this->name = border;
	this->is_feedthroughable = 0;
}

Block::Block(Block const &b) {
	this->name = b.name;
	this->vertices = b.vertices;

	this->coordinate = b.coordinate;
	this->facingFlip = b.facingFlip;
	this->blkID = b.blkID;

	this->through_block_net_num = b.through_block_net_num;
	this->through_block_edge_net_num = b.through_block_edge_net_num;
	this->block_port_region = b.block_port_region;
	this->is_feedthroughable = b.is_feedthroughable;
	this->is_tile = b.is_tile;
	this->edges = b.edges;
}

void Block::verticesToEdges() {
  	//if (b.is_feedthroughable) return; // should be revise, TODO!!
	for (size_t i = 0; i < vertices.size(); i++) {
		Edge newEdge(vertices[i], vertices[(i + 1) % vertices.size()]);
		newEdge.block = shared_from_this();
		edges.push_back(std::move(newEdge));
	}

	return;
}

// void Block::addENN(const Edge &E, const int &NN) {
// 	Edge TBE(E.first, E.second);
// 	TBE.block = shared_from_this();
// 	through_block_edge_net_num.push_back(EdgeNetNum(TBE, NN));
// }

// void Block::addBPR(const Edge &e) {
// 	Edge BPR(e.first, e.second);
// 	BPR.block = shared_from_this();
// 	block_port_region.push_back(std::move(BPR));
// }

void faceAndFlip(vector<Point> &anyVertices, char const &face, char const &flip) {
	vector<Point> tempVertices;
	tempVertices = std::move(anyVertices);
	
	// facing
	for (Point &vertex : tempVertices) {
		switch (face) {
		case 'N': { anyVertices.push_back(vertex); break; }
		case 'W': { anyVertices.push_back(Point(-vertex.y, vertex.x)); break; }
		case 'S': { anyVertices.push_back(Point(-vertex.x, -vertex.y)); break; }
		case 'E': { anyVertices.push_back(Point(vertex.y, -vertex.x)); break; }
		}
	}

	// flip
	if (flip == 'F') { for (Point &vertex : anyVertices) { vertex.x = -vertex.x; } }

	return;
}

void shift(vector<Point> &anyVertices, Point const &min) {
	for (Point &vertex : anyVertices) {
		vertex.x += min.x;
		vertex.y += min.y;
	}

	return;
}

vector<Point> edge2Point(Edge const &e) {
	return vector<Point> { e.first, e.second };
}

void Block::transposeAllVertices() {
	// three things need to transpose:
	// 1. vertices
	// 2. Edges in through_block_edge_net_num
	// 3. Edges in block_port_region

	char flip = facingFlip[0];
	char face = facingFlip[1];

	faceAndFlip(vertices, face, flip);

	// find the new min, later all kinds of vertices if shifted base on this point
	Point min(INT_MAX, INT_MAX);

	for (Point const &vertex : vertices) {
		if (vertex.x < min.x) min.x = vertex.x;
		if (vertex.y < min.y) min.y = vertex.y;
		if (vertex.x > top_coordinate.x) top_coordinate.x = vertex.x;
		if (vertex.y > top_coordinate.y) top_coordinate.y = vertex.y;
	}
	top_coordinate.x += (coordinate.x - min.x);
	top_coordinate.y += (coordinate.y - min.y);
	min.x = -min.x;
	min.y = -min.y;

	// first done the vertices of the block
	shift(vertices, min);
	shift(vertices, coordinate);

	// then all the other
	// note that after experiment, they only need shift base on coordinate
	// NO NEED to faceFlip!!!
	for (auto &TBENN : through_block_edge_net_num) {
		vector<Point> pointsOfEdge = edge2Point(TBENN->edge);
		shift(pointsOfEdge, coordinate);
		TBENN->edge.first = pointsOfEdge[0];
		TBENN->edge.second = pointsOfEdge[1];
	}
	for (Edge &BPR : block_port_region) {
		vector<Point> pointsOfEdge = edge2Point(BPR);
		shift(pointsOfEdge, coordinate);
		BPR.first = pointsOfEdge[0];
		BPR.second = pointsOfEdge[1];
	}

	return;
}

bool isPointOnEdge(const Point& p, const Edge& edge) {
    if (edge.isVertical()) { return p.x == edge.fixed() && edge.inRange(p.y);
    } else { return p.y == edge.fixed() && edge.inRange(p.x); }
}

bool Block::enclose(const Point p) const {
    bool inside = false;
	if (p.x <= coordinate.x || p.x >= top_coordinate.x || p.y <= coordinate.y || p.y >= top_coordinate.y) return false;
    for (const Edge& edge : edges) {
        if (isPointOnEdge(p, edge)) return false; // count as exterior if on edge

        if ((edge.first.y > p.y) != (edge.second.y > p.y)) {
            int intersectX = edge.first.x + (p.y - edge.first.y) * (edge.second.x - edge.first.x) / (edge.second.y - edge.first.y);
            if (p.x < intersectX) {
                inside = !inside;
            }
        }
    }
    return inside;
}

bool Block::canThrough(bool direction_X, Point const point, Pair target) {
	std::cerr << "\nWARNING: canThrough is design for checking a point "
			  << "inside a block can penetrate a nonfeed block through "
			  << "its BPR or TBENN, and is temporarily unfinish.\n";
	int fix = direction_X? point.y: point.x;
	for (auto const &e: block_port_region) {
		if (direction_X == e.isVertical() && e.inRange(fix)) return 1;
	}
	for (auto const &t: through_block_edge_net_num) {
		Edge const e = t->edge;
		if (direction_X == e.isVertical() && e.inRange(fix)) return 1;
	}
	return 0;
}

bool Block::operator <(const Block& other) const { return this->name < other.name; }

void Block::showBlockInfo() const {
	cout << "blockName: '" << name << "'" << "\n"
	<< "blkID: '" << blkID << "'" << "\n"
	<< "coordinate: (" << coordinate.x << ", " << coordinate.y
	<< ") " << "\n"
	<< "facingFlip: '" << facingFlip << "'" << "\n";
	
	cout << "vertices: " << "\n";
	for (auto v : vertices) {
		cout<< v.x << "\t" << v.y << "\n";
	}

	cout << "through_block_net_num: " << through_block_net_num << "\n"
	<< "through_block_edge_net_num: ";
	for (auto const &TBENN : through_block_edge_net_num) {
		cout << "\n" << "(" << TBENN->edge.first.x << ", " << TBENN->edge.first.y << ") ("
		<< TBENN->edge.second.x << ", " << TBENN->edge.second.y << ") " << TBENN->net_num;
	}
	cout << "\n";

	cout << "block_port_region: ";
	for (Edge const &BPR : block_port_region) {
		cout << "\n" << "(" << BPR.first.x << ", " << BPR.first.y
		<< ") (" << BPR.second.x << ", " << BPR.second.y << ")";
	}
	cout << "\n";
	
	cout << "is_feedthroughable: " << is_feedthroughable << "\n"
	<< "is_tile: " << is_tile << "\n";
	cout << "----------------------" << "\n";
}