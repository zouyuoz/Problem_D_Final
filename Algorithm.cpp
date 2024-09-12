#include "Algorithm.h"
#include <cmath>
#include <fstream>
#include <memory>
#include <utility>

using std::abs;

const bool Log = 1;

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

	if (hasMTs) handleMTsNets(net);
	else if (hasRXs) handleRXsNets(net);
	else handleNormalNets(net);

	return std::move(path);
}

// first get which cells belongs to a port
// and do it to all the ports
// store these Cells to MTs
// when doing the algorithm, these cells is set to throughable
// (if is originally not throughable)
// after find a path, check if all the mt in MTs are included
// if no, redo algorithm, else export this path
set<shared_ptr<Cell>> A_star_algorithm::fromMTGetCells(const Edge &mt) {
	set<shared_ptr<Cell>> MTCells;

	if (mt.first == mt.second) {
		// handle single terminal cell (maybe target cell)
		MTCells.insert(allCells.cellEnclose(mt.first));
		return MTCells;
	}

	auto edgeCell_1_candidates = allCells.cellsOnVertex(mt.first);
	auto edgeCell_2_candidates = allCells.cellsOnVertex(mt.second);
	shared_ptr<Cell> edgeCell_1, edgeCell_2;

	for (auto &cell: edgeCell_1_candidates) {
		if (cell->EdgeBelongs2Cell(mt)) { edgeCell_1 = cell; break; }
	}
	for (auto &cell: edgeCell_2_candidates) {
		if (cell->EdgeBelongs2Cell(mt)) { edgeCell_2 = cell; break; }
	}

	int xIndex_1 = edgeCell_1->xIndex, yIndex_1 = edgeCell_1->yIndex;
	int xIndex_2 = edgeCell_2->xIndex, yIndex_2 = edgeCell_2->yIndex;
	if (xIndex_1 == xIndex_2) {
		// means this port is vertical
		int yMin = std::min(yIndex_1, yIndex_2), yMax = std::max(yIndex_1, yIndex_2);
		for (int y = yMin; y <= yMax; ++y) { MTCells.insert(allCells.cells[xIndex_1][y]); }
	} else if (yIndex_1 == yIndex_2) {
		// means this port is horizontal
		int xMin = std::min(xIndex_1, xIndex_2), xMax = std::max(xIndex_1, xIndex_2);
		for (int x = xMin; x <= xMax; ++x) { MTCells.insert(allCells.cells[x][yIndex_1]); }
	}

	return MTCells;
}
void A_star_algorithm::handleMTsNets(const Net &net) {
	Point s = net.TX.coord;
	Point t = net.RXs[0].coord;
	shared_ptr<Cell> source = allCells.cellEnclose(s);
	shared_ptr<Node> sourceNode = make_shared<Node>(source); // the VERY source

	for (size_t i = 0; i < net.orderedMTs.size(); ++i) {
		sourceNode = findPath(sourceNode, net.orderedMTs[i], net.ID);
		if (!sourceNode) return;
	}
	shared_ptr<Cell> target = allCells.cellEnclose(t);
	sourceNode = findPath(sourceNode, target, net); // TODO
	backTraceFinalPath(sourceNode, net.TX.coord, net.RXs[0].coord);
	return;
}

bool checkPathIncludeAllMTs(const shared_ptr<Node> &path, vector<set<shared_ptr<Cell>>> MTsCells) {
	shared_ptr<Node> theNode = path;
	while (theNode) {
		auto itMT = MTsCells.begin();
		bool foundMTCell = 0;
		for (itMT = MTsCells.begin(); itMT != MTsCells.end(); ++itMT) {
			for (auto it = (*itMT).begin(); it != (*itMT).end(); ++it) {
				auto MTCell = *it;
				if (theNode->cell == MTCell) {
					foundMTCell = 1;
					break;
				}
			}
			if (foundMTCell) break;
		}
		if (foundMTCell) MTsCells.erase(itMT);
		if (MTsCells.empty()) break;
		theNode = theNode->parent;
	}
	return MTsCells.empty();
}

shared_ptr<Node> A_star_algorithm::findPath(
	shared_ptr<Node> sourceNode,
	const Edge &mt, int netID
) {
	set<shared_ptr<Node>> Open = { sourceNode };
	set<shared_ptr<Node>> Close;

	Point t((mt.first.x + mt.second.x) / 2, (mt.first.y + mt.second.y) / 2);
	cout << mt.first.x << ", " << mt.first.y << ", " << mt.second.x << ", " << mt.second.y << "\n";

	int iter = -1;
	std::ofstream file("log.txt");
	if (Log) file << "Finding " << *sourceNode->cell << " --> " << t.x << ", " << t.y << "\n";

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
			if (!n) { if (Log) file << "\tINVALID: nullptr\n"; continue; }

			if (Log) file << "\tneighbor" << *n << "\t";
			// if (n->isSomeNetsMT()) {
			// 	auto m = n->someNetsMT;
			// 	cout << m.first.x << ", " << m.first.y << ", " << m.second.x << ", " << m.second.y << "\n";
			// }
			if (n->someNetsMT == mt) {
				cout << "probably?\n";
				// need to check the direction is right, TODO!
				if (directionIntoPort(nowNode->cell, n, mt)) {
					cout << "PATH found!\t";
					cout << n->node.x << ", " << n->node.y << "\n";
					shared_ptr<Node> final = nowNode->generateNeighbor(n, file);
					return final;
				}
				else continue;
			}
			Terminal sourceRepeater(sourceNode->cell->block);
			Terminal targetRepeater(mt.block);
			if (!canGoNext(nowNode->cell, n, sourceRepeater, targetRepeater, netID)) {
				if (Log) file << "INVALID: can't go or not belong terminals\n";
				continue;
			}
			// if (!n->capacityEnough(net.num)) continue;
			if (Log && n->isBPR()) file << "isBPR\t";
			if (Log) file << "SUCCESS\n";

			shared_ptr<Node> neighbor = nowNode->generateNeighbor(n, file);
			neighbor->calculate_g(nowNode);
			neighbor->calculate_h(t);
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
		}
		Open.erase(nowNode);
		Close.insert(nowNode);
		if (Log) file << "End of iter " << iter << ", now have " << Open.size() << " nodes in Open\n";
	}
	cout << "PATH NOT found!\n";
	return nullptr;
}

// should record every Node through found_Path
// and every Node could be the new TX of other RXs
// so how do we do that, well I don't know lol
// maybe we could try this method and the old one
// and compare which one is more optimized
void A_star_algorithm::handleRXsNets(const Net &net) {
	int RX_count = net.RXs.size();
	return;
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

			if (!canGoNext(nowNode->cell, n, net.TX, net.RXs[0])) {
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

bool A_star_algorithm::canGoNext(shared_ptr<Cell> nowCell, shared_ptr<Cell> nextCell, Terminal s, Terminal t, int netID) {
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
			// if (nowCell->block == s.block && s.name == "MTTerminal") {
			// 	if (nowCell->someNetsMT.first.x == -99) {
			// 		// cout << "no MT\n";
			// 		return 0;
			// 	}
			// 	else {
			// 		// cout << "nowNetMtNetID: " << nowCell->someNetsMT.netID;
			// 		// cout << ", routingNetID: " << netID << "\n";
			// 		return nowCell->someNetsMT.netID == netID;
			// 	}
			// }

			if (nowCell->block->noPort()) {
				if (nowCell->block->is_feedthroughable) return 1;
				if (nowCell->block == s.block) return 1;
			}
		}
	}
	else {
		if (nextCell->inBlock()) { // enter a block
			// if (nextCell->block == t.block && t.name == "MTTerminal") {
			// 	if (nextCell->someNetsMT.first.x == -99) {
			// 		// cout << "no MT\n";
			// 		return 0;
			// 	}
			// 	else {
			// 		// cout << "nextNetMtNetID: " << nextCell->someNetsMT.netID;
			// 		// cout << ", routingNetID: " << netID << "\n";
			// 		return nextCell->someNetsMT.netID == netID;
			// 	}
			// }

			if (nextCell->block->noPort()) {
				if (nextCell->block->is_feedthroughable) return 1;
				if (nextCell->block == t.block) return 1;
			}
		}
		else return 1;
	}
	return 0;

	/*
	if (nowCell->inBlock()) {
		if (nowCell->block->noPort()) {
			if (nowCell->block == s.block) return 1;			// escape from source block
		} // else return nowCell->block == nextCell->block;		// if this block has port and this is not port, then can't go
	}
	if (nextCell->inBlock()) {
		if (nextCell->block->noPort()) {						// if a block has port, then must go through port
			if (nextCell->block->is_feedthroughable) return 1;	// can go feedthroughable blocks
			if (nextCell->block == t.block) return 1;			// enter target block
		} // else return nowCell->block == nextCell->block;		// if this block has port and this is not port, then can't go
	} else return !(nowCell->inBlock());						// can go to channels (no block)
	return nowCell->block == nextCell->block;					// if in the same block, then can go, else can't
	*/
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
	if (isStraight) {
		path.push_back(s);
		path.push_back(t);
		return;
	}
	cout << "not straight";
	path.push_back(s);
	path.push_back(Point(s.x, t.y));
	path.push_back(t);
	return;
}