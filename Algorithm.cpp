#include "Algorithm.h"
#include <cmath>
#include <cstddef>
#include <fstream>
#include <memory>
#include <utility>

using std::abs;

const bool Log = 0;

bool Node::operator <(const Node &other) const {
	// future add if has mt, have higher priority
	if (f_value != other.f_value) return f_value < other.f_value;
	return cell < other.cell;
}

void Node::calculate_g(const shared_ptr<Node> &nowNode) {
    int distance = abs(cell->node.x - nowNode->cell->node.x) + abs(cell->node.y - nowNode->cell->node.y);
    bool extraTurn = direction != nowNode->direction && nowNode->direction != Direction::O;
    g_value = nowNode->g_value + distance;
	if (extraTurn) g_value += TURN_COST * (int)(std::pow(turn, 4) - std::pow(turn - 1, 4));
}

void Node::calculate_h(const Point &endPoint) {
	h_value = abs(endPoint.x - cell->node.x) + abs(endPoint.y - cell->node.y); // distance_to_end;
	return;
}

void Node::calculate_f() {
	f_value = g_value + h_value;
	return;
}

A_star_algorithm::A_star_algorithm(Chip &caseChip) {
	chip = caseChip;
	allCells = caseChip.allCells;
	return;
}

shared_ptr<Node> Node::generateNeighbor(const shared_ptr<Cell> &n, std::ofstream &file) {
	shared_ptr<Node> neighbor = make_shared<Node>(n);
	neighbor->parent = shared_from_this();

	int delta_X = n->node.x - cell->node.x;
	int delta_Y = n->node.y - cell->node.y;

	if (delta_X && !delta_Y) neighbor->direction = Direction::HOR;
	else if (!delta_X && delta_Y) neighbor->direction = Direction::VER;
	else { cout << "wtf"; neighbor->direction = Direction::O; }

	neighbor->turn = turn + int(neighbor->direction != direction);

	if (Log) {
		file << "\t\tnowNode Direction: ";
		if (direction == Direction::HOR) file << "HOR\t";
		else if (direction == Direction::VER) file << "VER\t";
		else file << "ORI\t";
		file << "Turn: " << turn << "\n";
		file << "\t\tneigbor Direction: ";
		if (neighbor->direction == Direction::HOR) file << "HOR\t";
		else if (neighbor->direction == Direction::VER) file << "VER\t";
		file << "Turn: " << neighbor->turn << "\n";
	}

	return std::move(neighbor);
}

void Node::outputTemporaryPath() {
	cout << "(" << cell->node.x << "," << cell->node.y << ")\n";
	shared_ptr<const Node> thisNode = this->parent;
	while (thisNode) {
		cout << "(" << thisNode->cell->node.x << "," << thisNode->cell->node.y << ")\n";
		thisNode = thisNode->parent;
	}
}

shared_ptr<Node> findSmallestFValueNode(const set<shared_ptr<Node>> &open) {
	// return *open.begin(); // should verified
	// dude why is that so weird...
	int smallestFvalue = INT_MAX;
	auto nodeWithSmallestFvalue = *open.begin();
	for (const auto &o: open) {
		if (o->f_value < smallestFvalue) {
			smallestFvalue = o->f_value;
			nodeWithSmallestFvalue = o;
		}
	}
	return nodeWithSmallestFvalue;
}

vector<Point> A_star_algorithm::getPath(const Net &net) {
	path = {};
	bool hasMTs = net.HMFT_MUST_THROUGHs.size() + net.MUST_THROUGHs.size();
	bool hasRXs = net.RXs.size() > 1;

	if (hasMTs) {
		handleHasMTsNets(net);
	} else if (hasRXs) {
		handleMultRXNets(net);
	} else {
		handleNormalNets(net);
	}

	return std::move(path);
}

// first get which cells belongs to a port
// and do it to all the ports
// store these Cells to MTs
// when doing the algorithm, these cells is set to throughable
// (if is originally not throughable)
// after find a path, check if all the mt in MTs are included
// if no, redo algorithm, else export this path
shared_ptr<Cell> A_star_algorithm::fromMTGetCells(const Edge &mt) {

	if (mt.first == mt.second) {
		// handle single terminal cell (maybe target cell)
		return allCells.cellEnclose(mt.first);
	}

	auto edgeCell_1_candidates = allCells.cellsOnVertex(mt.first);
	auto edgeCell_2_candidates = allCells.cellsOnVertex(mt.second);
	shared_ptr<Cell> edgeCell_1, edgeCell_2;

	for (auto &cell: edgeCell_1_candidates) {
		if (cell->EdgeBelongs2Cell(mt)) {
			edgeCell_1 = cell;
			break;
		}
	}
	for (auto &cell: edgeCell_2_candidates) {
		if (cell->EdgeBelongs2Cell(mt)) {
			edgeCell_2 = cell;
			break;
		}
	}

	int xIndex_1 = edgeCell_1->xIndex, yIndex_1 = edgeCell_1->yIndex;
	int xIndex_2 = edgeCell_2->xIndex, yIndex_2 = edgeCell_2->yIndex;
	
	if (xIndex_1 == xIndex_2) {
		// means this port is vertical
		int yAvg = (yIndex_1 + yIndex_2) / 2;
		return allCells.cells[xIndex_1][yAvg];
	} else if (yIndex_1 == yIndex_2) {
		// means this port is horizontal
		int xAvg = (xIndex_1 + xIndex_2) / 2;
		return allCells.cells[xAvg][yIndex_1];
	}
	return nullptr;
}
void A_star_algorithm::handleHasMTsNets(const Net &net) {
	Point s = net.TX.coord;
	Point t = net.RXs[0].coord;
	shared_ptr<Cell> source = allCells.cellEnclose(s);
	shared_ptr<Cell> target = allCells.cellEnclose(t);
	shared_ptr<Node> sourceNode = make_shared<Node>(source); // the VERY source

	for (size_t i = 0; i < net.orderedMTs.size(); ++i) {
		auto repeaterCell = fromMTGetCells(net.orderedMTs[i]);
		sourceNode = findPath_HasMTs(sourceNode, repeaterCell, net);
	}

	sourceNode = findPath_HasMTs(sourceNode, target, net); // TODO
	backTraceFinalPath(sourceNode, net.TX.coord, net.RXs[0].coord);
	return;
}

bool checkPathIncludeAllMTs(const shared_ptr<Node> &path, const Net &net) {
	shared_ptr<Node> theNode = path;
	auto MTs = net.orderedMTs;
	while (theNode) {
		if (theNode->cell->isSomeNetsMT()){
			auto MTsInThisCell = theNode->cell->someNetsMTs;
			auto itMT = MTs.begin();
			bool foundMT = 0;

			for (const auto &mt: MTsInThisCell) {
				cout << "MTInThisCell: " << mt.block->name << ", " << mt.netID << "\n";
				for (; itMT != MTs.end(); ++itMT) {
					auto MT = *itMT;
					cout << "MTOfNet: " << MT.block->name << ", " << MT.netID << "\n";
					if (mt == MT && net.ID == MT.netID) {
						foundMT = 1;
						break;
					}
				}
				if (foundMT) {
					MTs.erase(itMT);
					break;
				}
			}
			if (MTs.empty()) break;
		}
		theNode = theNode->parent;
	}
	cout << MTs.size();
	return MTs.empty();
}

shared_ptr<Node> A_star_algorithm::findPath_HasMTs(shared_ptr<Node> sourceNode, shared_ptr<Cell> target, const Net &net) {
	path = {};
	std::ofstream file("log.txt");
	if (Log) file << "Finding " << *(sourceNode->cell) << " --> " << *target << "\n";

	set<shared_ptr<Node>> Open = { sourceNode };
	set<shared_ptr<Node>> Close;

	int iter = -1;
	while (!Open.empty()) {
		// find node with smallest f(n) in open
		shared_ptr<Node> nowNode = findSmallestFValueNode(Open);

		if (Log) {
			file << "------- iter " << ++iter << " -------\n";
			file << "nowNode(" << nowNode->cell->node.x << "," << nowNode->cell->node.y << ")\t";
			file << "f: " << nowNode->f_value << "\n";
		}

		set<shared_ptr<Cell>> neighbors = allCells.getNeighbor(nowNode->cell);
		for (auto &n: neighbors) {
			if (!n) {
				if (Log) file << "\tINVALID: nullptr\n";
				continue;
			}

			if (Log) file << "\tneighbor" << *n << "\t";
			if (n == target) {
				if (Log) file << "Path Found!\n";
				cout << "PATH found!\n";
				shared_ptr<Node> final = nowNode->generateNeighbor(n, file);
				return final;
			}

			if (!canGoNext(nowNode->cell, n, net)) {
				if (Log) file << "INVALID: can't go or not belong terminals\n";
				continue;
			}
			if (!n->capacityEnough(net.num)) {
				if (Log) file << "INVALID: capacity not enough\n";
				continue;
			}
			// if (/*can't go but it is mt or hmftmt*/1) /*then it can go*/ 1; // TODO
			if (Log && n->isBPR()) file << "isBPR\t";
			if (Log) file << "SUCCESS\n";

			shared_ptr<Node> neighbor = nowNode->generateNeighbor(n, file);
			neighbor->calculate_g(nowNode);
			neighbor->calculate_h(net.RXs[0].coord);
			neighbor->calculate_f();

			bool alreadyExistInClose = 0;
			for (const auto &close: Close) {
				if (neighbor->cell == close->cell) {
					if (Log) {
						file << "\t\talreadyExistInClose\tnow_f: " << close->f_value;
						file << "\tnew_f: " << neighbor->f_value;
						if (neighbor->f_value < close->f_value) file << " @WEIRD";
						file << "\n";
					}
					alreadyExistInClose = 1;
					break;
				}
			}
			if (alreadyExistInClose) continue;

			bool alreadyExistInOpenAndNotBetter = 0;
			for (auto it = Open.begin(); it != Open.end(); ++it) {
				if (neighbor->cell == (*it)->cell) {
					if (Log) file << "\t\talreadyExistInOpen\t";
					if (Log) file << "now_f: " << (*it)->f_value << "\tnew_f: " << neighbor->f_value << "\t";
					if (neighbor->f_value >= (*it)->f_value) {
						if (Log) file << "notBetter\n";
						alreadyExistInOpenAndNotBetter = 1;
					} else {
						if (Log) file << "isBetter\n";
						Open.erase(it);
					}
					break;
				}
			}
			if (alreadyExistInOpenAndNotBetter) continue;

			Open.insert(neighbor);
			if (Log) file << "\t\tInsert this neighbor, f: " << neighbor->f_value << "\n";
		}
		Open.erase(nowNode);
		Close.insert(nowNode);
		if (Log) file << "End of iter " << iter << ", now have " << Open.size() << " nodes in Open\n";
	}
	file.close();
	cout << "PATH NOT found!\n";
	return nullptr;
}

bool A_star_algorithm::canGoNext(shared_ptr<Cell> nowCell, shared_ptr<Cell> nextCell, const Net &net) {
	if (!nextCell->valid()) return 0;							// can't go to an invalid cell, no matter what

	bool notMTBlocks = 1;
	for (const auto &netMt: net.orderedMTs) {
		if (nowCell->block == netMt.block) notMTBlocks = 0;
		if (nextCell->block == netMt.block) notMTBlocks = 0;
	}

	if (notMTBlocks) {
		if (nowCell->isBPR() || nextCell->isBPR()) {			// BPR cells could be passable
			// direction should be considered
			Edge BPR = nowCell->isBPR() ? nowCell->BPR : nextCell->BPR;
			if (directionIntoPort(nowCell, nextCell, BPR)) return 1;
		}
		if ((bool)nowCell->TBENN || (bool)nextCell->TBENN) {	// TBENN cells could be passable
			// direction should be considered
			auto TBENN = (bool)nowCell->TBENN ? nowCell->TBENN : nextCell->TBENN;
			Edge e = TBENN->edge;
			if (directionIntoPort(nowCell, nextCell, e)) return 1;
		}
	}

	bool showde = 1;

	if (nowCell->inBlock()) {
		if (nextCell->inBlock()) return nowCell->block == nextCell->block;
		else { // exit a block
			bool isLeavingMTBlock = 0;
			for (const auto &netMt: net.orderedMTs) {
				if (nowCell->block == netMt.block) {
					isLeavingMTBlock = 1;
					if (showde) cout << "leaveMT " << netMt.block->name;
					break;
				}
			}
			if (isLeavingMTBlock) {
				for (const auto &cellMt: nowCell->someNetsMTs) {
					if (showde) cout << "\n - cellMT: " << cellMt.netID;
					if (cellMt.netID == net.ID) {
						if (directionIntoPort(nowCell, nextCell, cellMt)) {
							if (showde) cout << "...o\n";
							return 1;
						}
					}
				}
				if (showde) cout << "...x\n";
				return 0;
			}
			if (nowCell->block->noPort()) {
				if (nowCell->block->is_feedthroughable) return 1;
				if (nowCell->block == net.TX.block) return 1;
			}
		}
	}
	else {
		if (nextCell->inBlock()) { // enter a block
			bool isLeavingMTBlock = 0;
			for (const auto &netMt: net.orderedMTs) {
				if (nextCell->block == netMt.block) {
					isLeavingMTBlock = 1;
					if (showde) cout << "enterMT " << netMt.block->name;
					break;
				}
			}
			if (isLeavingMTBlock) {
				for (const auto &cellMt: nextCell->someNetsMTs) {
					if (showde) cout << "\n - cellMT: " << cellMt.netID;
					if (cellMt.netID == net.ID) {
						if (directionIntoPort(nowCell, nextCell, cellMt)) {
							if (showde) cout << "...o\n";
							return 1;
						}
					}
				}
				if (showde) cout << "...x\n";
				return 0;
			}
			if (nextCell->block->noPort()) {
				if (nextCell->block->is_feedthroughable) return 1;
				if (nextCell->block == net.RXs[0].block) return 1;
			}
		}
		else return 1;
	}
	return 0;
}

// should record every Node through found_Path
// and every Node could be the new TX of other RXs
// so how do we do that, well I don't know lol
// maybe we could try this method and the old one
// and compare which one is more optimized
void A_star_algorithm::handleMultRXNets(const Net &net) {
	set<shared_ptr<Cell>> targetCells;
	for (const auto &rx: net.RXs) {
		targetCells.insert(allCells.cellEnclose(rx.coord));
	}
	Point s = net.TX.coord;
	shared_ptr<Node> sourceNode = make_shared<Node>(allCells.cellEnclose(s));
	
}

void A_star_algorithm::handleNormalNets(const Net &net) {
	Point s = net.TX.coord;
	Point t = net.RXs[0].coord;

	if (s == t) {
		cout << "PATH found!\n";
		path.push_back(s);
		return;
	}

	shared_ptr<Cell> source = allCells.cellEnclose(s);
	shared_ptr<Cell> target = allCells.cellEnclose(t);

	if (source == target) {
		monotonicPath(s, t);
		return;
	}

	shared_ptr<Node> sourceNode = make_shared<Node>(source);
	shared_ptr<Node> targetNode = findPath(sourceNode, target, net);
	backTraceFinalPath(targetNode, s, t);
	return;
}

shared_ptr<Node> A_star_algorithm::findPath(shared_ptr<Node> sourceNode, shared_ptr<Cell> target, const Net &net) {
	path = {};
	std::ofstream file("log.txt");
	if (Log) file << "Finding " << *(sourceNode->cell) << " --> " << *target << "\n";

	set<shared_ptr<Node>> Open = { sourceNode };
	set<shared_ptr<Node>> Close;

	int iter = -1;
	while (!Open.empty()) {
		// find node with smallest f(n) in open
		shared_ptr<Node> nowNode = findSmallestFValueNode(Open);

		if (Log) {
			file << "------- iter " << ++iter << " -------\n";
			file << "nowNode(" << nowNode->cell->node.x << "," << nowNode->cell->node.y << ")\t";
			file << "f: " << nowNode->f_value << "\n";
		}

		set<shared_ptr<Cell>> neighbors = allCells.getNeighbor(nowNode->cell);
		for (auto &n: neighbors) {
			if (!n) {
				if (Log) file << "\tINVALID: nullptr\n";
				continue;
			}

			if (Log) file << "\tneighbor" << *n << "\t";
			if (n == target) {
				if (Log) file << "Path Found!\n";
				cout << "PATH found!\n";
				shared_ptr<Node> final = nowNode->generateNeighbor(n, file);
				return final;
			}

			if (!canGoNext(nowNode->cell, n, net)) {
				if (Log) file << "INVALID: can't go or not belong terminals\n";
				continue;
			}
			if (!n->capacityEnough(net.num)) {
				if (Log) file << "INVALID: capacity not enough\n";
				continue;
			}
			// if (/*can't go but it is mt or hmftmt*/1) /*then it can go*/ 1; // TODO
			if (Log && n->isBPR()) file << "isBPR\t";
			if (Log) file << "SUCCESS\n";

			shared_ptr<Node> neighbor = nowNode->generateNeighbor(n, file);
			neighbor->calculate_g(nowNode);
			neighbor->calculate_h(net.RXs[0].coord);
			neighbor->calculate_f();

			bool alreadyExistInClose = 0;
			for (const auto &close: Close) {
				if (neighbor->cell == close->cell) {
					if (Log) {
						file << "\t\talreadyExistInClose\tnow_f: " << close->f_value;
						file << "\tnew_f: " << neighbor->f_value;
						if (neighbor->f_value < close->f_value) file << " @WEIRD";
						file << "\n";
					}
					alreadyExistInClose = 1;
					break;
				}
			}
			if (alreadyExistInClose) continue;

			bool alreadyExistInOpenAndNotBetter = 0;
			for (auto it = Open.begin(); it != Open.end(); ++it) {
				if (neighbor->cell == (*it)->cell) {
					if (Log) file << "\t\talreadyExistInOpen\t";
					if (Log) file << "now_f: " << (*it)->f_value << "\tnew_f: " << neighbor->f_value << "\t";
					if (neighbor->f_value >= (*it)->f_value) {
						if (Log) file << "notBetter\n";
						alreadyExistInOpenAndNotBetter = 1;
					} else {
						if (Log) file << "isBetter\n";
						Open.erase(it);
					}
					break;
				}
			}
			if (alreadyExistInOpenAndNotBetter) continue;

			Open.insert(neighbor);
			if (Log) file << "\t\tInsert this neighbor, f: " << neighbor->f_value << "\n";
		}
		Open.erase(nowNode);
		Close.insert(nowNode);
		if (Log) file << "End of iter " << iter << ", now have " << Open.size() << " nodes in Open\n";
	}
	file.close();
	cout << "PATH NOT found!\n";
	return nullptr;
}

bool A_star_algorithm::_canGoNext(shared_ptr<Cell> nowCell, shared_ptr<Cell> nextCell, Terminal s, Terminal t) {
	if (!nextCell->valid()) return 0;							// can't go to an invalid cell, no matter what
	if (nowCell->isBPR() || nextCell->isBPR()) {				// BPR cells could be passable
		// direction should be considered
		Edge BPR = nowCell->isBPR() ? nowCell->BPR : nextCell->BPR;
		if (directionIntoPort(nowCell, nextCell, BPR)) return 1;
	}
	if ((bool)nowCell->TBENN || (bool)nextCell->TBENN) {		// TBENN cells could be passable
		// direction should be considered
		auto TBENN = (bool)nowCell->TBENN ? nowCell->TBENN : nextCell->TBENN;
		Edge e = TBENN->edge;
		if (directionIntoPort(nowCell, nextCell, e)) return 1;
	}

	if (nowCell->inBlock()) {
		if (nextCell->inBlock()) return nowCell->block == nextCell->block;
		else { // exit a block
			if (nowCell->block->noPort()) {
				if (nowCell->block->is_feedthroughable) return 1;
				if (nowCell->block == s.block) return 1;
			}
		}
	}
	else {
		if (nextCell->inBlock()) { // enter a block
			if (nextCell->block->noPort()) {
				if (nextCell->block->is_feedthroughable) return 1;
				if (nextCell->block == t.block) return 1;
			}
		}
		else return 1;
	}

	return 0;
}

bool A_star_algorithm::directionIntoPort(shared_ptr<Cell> nowCell, shared_ptr<Cell> nextCell, const Edge& e) {
	bool directionNext = !(nowCell->node.x - nextCell->node.x);
	if (directionNext != e.isVertical()) return 1;
	else return 0;
}

void A_star_algorithm::backTraceFinalPath(shared_ptr<Node> &final, Point s, Point t) {
	if (!final) return;
	shared_ptr<Node> theNode = final;
	Direction nowDirection = theNode->direction;
	Direction t_direction = nowDirection, s_direction;

	int turn_num = 0;

	while (theNode->parent) {
		if (theNode->direction != nowDirection) {
			++turn_num;
			path.push_back(theNode->cell->node);
			auto p = theNode->cell->node;
			nowDirection = theNode->direction;
			s_direction = theNode->direction;
		}
		theNode = theNode->parent;
	}

	if (!turn_num) {
		monotonicPath(s, t);
		return;
	}

	if (s_direction == Direction::VER) path.back() = Point(s.x, path.back().y);
	else path.back() = Point(path.back().x, s.y);
	path.push_back(s);

    reverse(path.begin(), path.end());

	if (t_direction == Direction::VER) path.back() = Point(t.x, path.back().y);
	else path.back() = Point(path.back().x, t.y);
	path.push_back(t);

	return;
}

void A_star_algorithm::monotonicPath(Point s, Point t) {
	bool isStraight = s.x == t.x || s.y == t.y;
	path.push_back(s);
	if (!isStraight) path.push_back(Point(s.x, t.y));
	path.push_back(t);
	return;
}