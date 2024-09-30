#ifndef ALGORITHM_H_INCLUDED
#define ALGORITHM_H_INCLUDED

#include "Net.h"
#include "Cell.h"
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <memory>

using std::make_shared;
constexpr int TURN_COST = 2000; // this punishment can modified
constexpr int IN_BLOCK = 10; // this punishment can modified

enum class Direction { O, VER, HOR };

class Node : public std::enable_shared_from_this<Node> {
public:
	shared_ptr<Cell> cell;
	shared_ptr<Node> parent = nullptr;
	int turn = -1;
	Direction direction = Direction::O;
	Node(shared_ptr<Cell> Grid, int Turn): cell(Grid), turn(Turn) {}
	Node(shared_ptr<Cell> Grid): cell(Grid) {}
	
	int g_value = 0;
	int h_value = 0;
	int f_value = 0;
	void calculate_g(const shared_ptr<Node>&);
	void calculate_h(const vector<Terminal>&);
	void calculate_f();

	void outputTemporaryPath();
	shared_ptr<Node> generateNeighbor(const shared_ptr<Cell>&, std::ofstream&);

	bool operator <(const Node &other) const;
	bool operator ==(const Node &other) const {
		return cell == other.cell && f_value == other.f_value;
	}
};

class A_star_algorithm {
private:
	Chip chip;
	Cell_Manager allCells;
public:
	A_star_algorithm(Chip &caseChip, int &tracks_per_um);
	vector<Point> path;
	vector<vector<Point>> RXsPath;
	vector<Simple_Edge> pathSegments;

	vector<Simple_Edge> getPath(Net &net);
	void handleMultRXNets(Net &net);
	void handleHasMTsNets(Net &net);
	void handleNormalNets(Net &net);
	shared_ptr<Node> findPath(shared_ptr<Node>, shared_ptr<Cell>, const Net&);
	set<shared_ptr<Node>> findPathRXs(shared_ptr<Node>, vector<shared_ptr<Cell>>, const Net&);

	bool canGoNext(shared_ptr<Cell>, shared_ptr<Cell>, const Net&);
	bool directionIntoPort(shared_ptr<Cell>, shared_ptr<Cell>, const Edge&);
	void backTraceFinalPath(shared_ptr<Node>&, Point, Point);
	void monotonicPath(Point, Point);
	void addNodesToRXsPath();
	void simplifiedSpanningTree(vector<vector<Point>>&, int = -1, Point = Point());
	void makePathToSegments(bool);
};

#endif // ALGORITHM_H_INCLUDED