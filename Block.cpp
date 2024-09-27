#include "Block.h"
#include <climits>
#include <cstddef>
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
	return vector<Point> { e.p1, e.p2 };
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
		TBENN->edge.p1 = pointsOfEdge[0];
		TBENN->edge.p2 = pointsOfEdge[1];
	}
	for (Edge &BPR : block_port_region) {
		vector<Point> pointsOfEdge = edge2Point(BPR);
		shift(pointsOfEdge, coordinate);
		BPR.p1 = pointsOfEdge[0];
		BPR.p2 = pointsOfEdge[1];
	}

	return;
}

void Block::adjustAllPorts() {
	for (size_t i = 0; i < block_port_region.size(); ++i) {
		adjustPortCoordinate(block_port_region[i]);
	}
	for (size_t i = 0; i < through_block_edge_net_num.size(); ++i) {
		adjustPortCoordinate(through_block_edge_net_num[i]->edge);
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

        if ((edge.p1.y > p.y) != (edge.p2.y > p.y)) {
            int intersectX = edge.p2.x + (p.y - edge.p1.y) * (edge.p2.x - edge.p1.x) / (edge.p2.y - edge.p1.y);
            if (p.x < intersectX) {
                inside = !inside;
            }
        }
    }
    return inside;
}

bool Block::noPort() const {
	return !(through_block_edge_net_num.size() + block_port_region.size());
}

void Block::adjustPortCoordinate(Edge &theEdge) {
	int adjustCoordValue = 0;
	for (size_t i = 0; i < edges.size(); ++i) {
		Edge blockEdge = edges[i];
		bool belongs2BlockEdge = blockEdge.inRange(theEdge)
			&& std::abs(blockEdge.fixed() - theEdge.fixed()) <= 2
			&& blockEdge.isVertical() == theEdge.isVertical();
		if (belongs2BlockEdge) {
			adjustCoordValue = blockEdge.fixed();
			break;
		}
	}
	if (adjustCoordValue == theEdge.fixed()) return;
	if (theEdge.isVertical()) {
		// fixed == X
		theEdge.p1.x = adjustCoordValue;
		theEdge.p2.x = adjustCoordValue;
	} else {
		// fixed == Y
		theEdge.p1.y = adjustCoordValue;
		theEdge.p2.y = adjustCoordValue;
	}
	return;
}

bool Block::operator <(const Block& other) const { return this->name < other.name; }

std::ostream& operator<<(std::ostream& os, const Block& b) {
	os << "blockName: '" << b.name << "'" << "\n"
	<< "blkID: '" << b.blkID << "'" << "\n"
	<< "coordinate: (" << b.coordinate.x << ", " << b.coordinate.y
	<< ") " << "\n"
	<< "facingFlip: '" << b.facingFlip << "'" << "\n";
	
	os << "vertices:\n";
	for (auto v : b.vertices) {
		os << v.x << "\t" << v.y << "\n";
	}

	os << "through_block_net_num: " << b.through_block_net_num << "\n"
	<< "through_block_edge_net_num: ";
	for (auto const &TBENN : b.through_block_edge_net_num) {
		os << "\n" << "(" << TBENN->edge.p1.x << ", " << TBENN->edge.p1.y << ") ("
		<< TBENN->edge.p2.x << ", " << TBENN->edge.p2.y << ") " << TBENN->net_num;
	}
	os << "\n";

	os << "block_port_region: ";
	for (Edge const &BPR : b.block_port_region) {
		os << "\n" << "(" << BPR.p1.x << ", " << BPR.p1.y
		<< ") (" << BPR.p2.x << ", " << BPR.p2.y << ")";
	}
	os << "\n";
	
	os << "is_feedthroughable: " << b.is_feedthroughable << "\n"
	<< "is_tile: " << b.is_tile << "\n";
	os << "----------------------" << "\n";

	return os;
}