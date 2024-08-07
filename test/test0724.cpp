#include <vector>
#include <iostream>
#include <algorithm>
#include <memory>

class Point {
public:
    Point(double x = 99, double y = 99) : x(x), y(y) {}

    double x, y;
	
    bool operator ==(const Point &other) const {
    	return (x == other.x && y == other.y);
    }
};

class Pair {
public:
	Pair() : min(0), max(0) {}
	Pair(double Min, double Max) {
		// Min should smaller than Max, 
		// but in case we still check that
		this->min = std::min(Min, Max);
		this->max = std::max(Min, Max);
	}

	double min, max;

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

class Block;  // 前向聲明

class Edge {
public:
    Edge() {}
    Edge(Pair r, double f) : first(Point(r.min, f)), second(Point(r.max, f)) {}
    Edge(Point First, Point Second) : first(First), second(Second) {}

    Point first, second;
    std::shared_ptr<Block> block;  // 修改為 shared_ptr

    bool isVertical() const { return first.x == second.x; }
    double fixed() const { return (isVertical() * first.x) + (!isVertical() * first.y); }
    Pair ranged() const { return isVertical() ? Pair(first.y, second.y) : Pair(first.x, second.x); }

    bool is_feedthroughable;

    bool inRange(double const &value) const { return ranged().min <= value && ranged().max >= value; }
    bool operator ==(const Edge &that) const { return this->first == that.first && this->second == that.second; }
};

class Block : public std::enable_shared_from_this<Block> {
public:
    std::vector<Point> vertices = {
        Point(0, 0), Point(3, 0), Point(3, 2), Point(2, 2),
        Point(2, 1), Point(1, 1), Point(1, 2), Point(0, 2)
    };

    std::vector<Edge> edges;

    void verticesToEdges() {
        for (size_t i = 0; i < vertices.size(); i++) {
            Edge newEdge(vertices[i], vertices[(i + 1) % vertices.size()]);
            newEdge.block = shared_from_this();  // 使用 shared_from_this()
            edges.push_back(newEdge);
        }
    }

    // 其他 Block 成員...
};

// 使用示例
int main() {
    auto block = std::make_shared<Block>();
	std::vector<std::shared_ptr<Block>> test;
	test.push_back(block);
	
	block->vertices.push_back(Point(0, 1));
    block->verticesToEdges();

	for (auto const &e : test[0]->edges) {
		std::cout << e.first.x << e.first.y << e.second.x << e.second.y << "\n";
	}

	return 0;
}