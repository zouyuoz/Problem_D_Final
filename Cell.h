#ifndef CELL_H_INCLUDED
#define CELL_H_INCLUDED

#include "Block.h"
#include <map>
#include <memory>
#include <set>
#include <utility>

using std::map;
using std::set;
using std::shared_ptr;
using std::pair;

class Chip;
class Net_Manager;

class Cell : public std::enable_shared_from_this<Cell> {
public:
	Cell(){}
	Cell(Pair, Pair);
	Cell(const Cell &other);
	static int unitsDistanceUm;
	static int tracks_um;

    Pair x, y;
	int xIndex, yIndex;
	shared_ptr<Block> block = nullptr;

	Point node;
	void modifiedNode(int, bool);
	bool inBlock();
	bool inNonfeed();
	bool valid(int, bool);
	Edge BPR;
	shared_ptr<EdgeNetNum> TBENN;
	vector<Edge> someNetsMTs;
	bool isBPR() { return BPR.p1.x != -99; }
	bool isSomeNetsMT() { return someNetsMTs.size(); }

    bool enclose(const Point&);
	void checkInsideBlock(vector<shared_ptr<Block>>);
	bool EdgeBelongs2Cell(const Edge &e);
	bool capacityEnough(int);

    bool operator <=(const Cell &other) const;
	bool operator <(const Cell &other) const;
	Cell& operator =(const Cell& other);
    friend std::ostream& operator<<(std::ostream& os, const Cell& b);
};

class Cell_Manager {
private:
	set<int> x_value, y_value;
public:
	Cell_Manager() {}
	map<int, map<int, shared_ptr<Cell>>> cells;
	void setXYvalue(set<int>, set<int>);
	void createCells(const vector<shared_ptr<Block>>&, const vector<Edge>&, const vector<shared_ptr<EdgeNetNum>>&, const Net_Manager);
	shared_ptr<Cell> cellEnclose(const Point&);
	shared_ptr<Cell> cellsOnVertex(const Point&, const Edge&);
	shared_ptr<Cell> middleCellOfMT(const Edge&);
	set<shared_ptr<Cell>> getNeighbor(const shared_ptr<Cell>&);
};

class Terminal {
public:
	Terminal() {}
	Terminal(string Name, Point Coord): name(Name), coord(Coord) {}
	Terminal(shared_ptr<Block> b): name(b->name), coord(Point(0,0)), block(b) {}
	string name;
	Point coord;
	shared_ptr<Block> block = nullptr;
	void absoluteCoord();

	bool operator <(const Terminal &other) const { return coord < other.coord; }
};

#endif // CELL_H_INCLUDED