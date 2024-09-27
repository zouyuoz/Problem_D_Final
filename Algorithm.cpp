#include "Algorithm.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>

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

void Node::calculate_h(const vector<Terminal> &endPoint) {
	for (int i = 0; i < endPoint.size(); ++i) {
		h_value += abs(endPoint[0].coord.x - cell->node.x) + abs(endPoint[0].coord.y - cell->node.y); // distance_to_end;
	}
	if (endPoint.size() > 1) h_value *= (2/3);
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

	return std::move(neighbor);
}

void outputTemporaryPath(const vector<Point> path) {
	cout << ":\n";
	for (int i = 0; i < path.size(); ++i) {
		cout << path[i].x << "," << path[i].y << "\n";
	}
	// cout << path[i].x << "," << path[i].y << "," << path[i + 1].x << "," << path[i + 1].y << "\n";
	return;
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

vector<Simple_Edge> A_star_algorithm::getPath(const Net &net) {
	path = {};
	bool hasMTs = net.orderedMTs.size();
	bool hasRXs = net.RXs.size() > 1;

	if (hasMTs) {
		handleHasMTsNets(net);
	} else if (hasRXs) {
		handleMultRXNets(net);
	} else {
		handleNormalNets(net);
	}

	makePathToSegments(hasRXs);
	return std::move(pathSegments);
}

// first get middle cell of a MTs
// when finding path, consider every cellMT is a temporary RX
// and do the normal algorithm
// Once the algorithm output a Node, let it be the new TX
// and take the next cellMT as new RX
// repeat this process until the last cellMT finish searching
// eventually do the algorithm route to the very RX
// finally, back trace the path and return it
void A_star_algorithm::handleHasMTsNets(const Net &net) {
	Point s = net.TX.coord;
	Point t = net.RXs[0].coord;
	shared_ptr<Cell> source = allCells.cellEnclose(s);
	shared_ptr<Cell> target = allCells.cellEnclose(t);
	shared_ptr<Node> sourceNode = make_shared<Node>(source); // the VERY source

	for (size_t i = 0; i < net.orderedMTs.size(); ++i) {
		auto repeaterCell = allCells.middleCellOfMT(net.orderedMTs[i]);
		sourceNode = findPath(sourceNode, repeaterCell, net);
	}

	sourceNode = findPath(sourceNode, target, net); // TODO
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
				for (int i = 0; i < net.RXs.size(); ++i) {
					if (nextCell->block == net.RXs[i].block) return 1;
				}
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
	vector<shared_ptr<Cell>> targetCells;
	for (const auto &rx: net.RXs) {
		targetCells.push_back(allCells.cellEnclose(rx.coord));
	}
	Point s = net.TX.coord;
	shared_ptr<Node> sourceNode = make_shared<Node>(allCells.cellEnclose(s));
	auto targetNodes = findPathRXs(sourceNode, targetCells, net);

	for (auto it = targetNodes.begin(); it != targetNodes.end(); ++it) {
		Point thisRX;
		shared_ptr<Node> thisTarget = *it;
		for (auto const t: net.RXs) {
			if (thisTarget->cell->enclose(t.coord)) {
				thisRX = t.coord;
				break;
			}
		}
		backTraceFinalPath(thisTarget, net.TX.coord, thisRX);
		RXsPath.push_back(std::move(path));
	}
	return;
}

vector<shared_ptr<Node>> A_star_algorithm::findPathRXs(shared_ptr<Node> sourceNode, vector<shared_ptr<Cell>> targets, const Net &net) {
	path = {};
	vector<shared_ptr<Node>> targetNodes;
	auto RXs = net.RXs;
	std::ofstream file("log.txt");

	set<shared_ptr<Node>> Open = { sourceNode };
	set<shared_ptr<Node>> Close;

	int iter = -1;
	while (!Open.empty()) {
		// find node with smallest f(n) in open
		shared_ptr<Node> nowNode = findSmallestFValueNode(Open);

		set<shared_ptr<Cell>> neighbors = allCells.getNeighbor(nowNode->cell);
		for (auto &n: neighbors) {
			if (!n) { continue; }

			auto t = targets.begin();
			for (; t != targets.end(); ++t) {
				if (n == *t) {
					cout << "PATH found!\n";
					auto it = RXs.begin();
					for (; it != RXs.end(); ++it) {
						if (n->enclose((*it).coord)) break;
					}
					RXs.erase(it);
					shared_ptr<Node> final = nowNode->generateNeighbor(n, file);
					targetNodes.push_back(final);
					targets.erase(t);
					break;
				}
			}
			if (targets.empty()) return targetNodes;

			if (!canGoNext(nowNode->cell, n, net)) {
				// INVALID: can't go or not belong terminals
				continue;
			}
			if (!n->capacityEnough(net.num)) {
				// INVALID: capacity not enough
				continue;
			}

			shared_ptr<Node> neighbor = nowNode->generateNeighbor(n, file);
			neighbor->calculate_g(nowNode);
			neighbor->calculate_h(RXs);
			neighbor->calculate_f();

			bool alreadyExistInClose = 0;
			for (const auto &close: Close) {
				if (neighbor->cell == close->cell) {
					alreadyExistInClose = 1;
					break;
				}
			}
			if (alreadyExistInClose) continue;

			bool alreadyExistInOpenAndNotBetter = 0;
			for (auto it = Open.begin(); it != Open.end(); ++it) {
				if (neighbor->cell == (*it)->cell) {
					if (neighbor->f_value >= (*it)->f_value) {
						alreadyExistInOpenAndNotBetter = 1;
					} else {
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
	}
	file.close();
	return targetNodes;
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

	set<shared_ptr<Node>> Open = { sourceNode };
	set<shared_ptr<Node>> Close;

	int iter = -1;
	while (!Open.empty()) {
		// find node with smallest f(n) in open
		shared_ptr<Node> nowNode = findSmallestFValueNode(Open);

		set<shared_ptr<Cell>> neighbors = allCells.getNeighbor(nowNode->cell);
		for (auto &n: neighbors) {
			if (!n) { continue; }

			if (n == target) {
				cout << "PATH found!\n";
				shared_ptr<Node> final = nowNode->generateNeighbor(n, file);
				return final;
			}

			if (!canGoNext(nowNode->cell, n, net)) {
				// INVALID: can't go or not belong terminals
				continue;
			}
			if (!n->capacityEnough(net.num)) {
				// INVALID: capacity not enough
				continue;
			}

			shared_ptr<Node> neighbor = nowNode->generateNeighbor(n, file);
			neighbor->calculate_g(nowNode);
			neighbor->calculate_h(net.RXs);
			neighbor->calculate_f();

			bool alreadyExistInClose = 0;
			for (const auto &close: Close) {
				if (neighbor->cell == close->cell) {
					alreadyExistInClose = 1;
					break;
				}
			}
			if (alreadyExistInClose) continue;

			bool alreadyExistInOpenAndNotBetter = 0;
			for (auto it = Open.begin(); it != Open.end(); ++it) {
				if (neighbor->cell == (*it)->cell) {
					if (neighbor->f_value >= (*it)->f_value) {
						alreadyExistInOpenAndNotBetter = 1;
					} else {
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
	}
	file.close();
	cout << "PATH NOT found!\n";
	return nullptr;
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

void A_star_algorithm::addNodesToRXsPath() {
	set<Point> nodes;
	for (const auto &path: RXsPath) {
		for (const auto &p: path) {
			nodes.insert(p);
		}
	}

	vector<vector<Point>> newRXsPath;

	for (int j = 0; j < RXsPath.size(); ++j) {
		// in a single path
		// calling newRXsPath[j];
		newRXsPath[j].push_back(RXsPath[j][0]);

		for (int i = 0; i < RXsPath[j].size() - 1; ++i) {
			// for a single segment of path
			vector<Point> intersections = { RXsPath[j][i + 1] };
			for (const auto &n: nodes) {
				if (
					std::min(RXsPath[j][i].x, RXsPath[j][i + 1].x) <= n.x
					&& std::max(RXsPath[j][i].x, RXsPath[j][i + 1].x) >= n.x
					&& std::min(RXsPath[j][i].y, RXsPath[j][i + 1].y) <= n.y
					&& std::max(RXsPath[j][i].y, RXsPath[j][i + 1].y) >= n.y
				) {
					intersections.push_back(n);
				}
			}

			if (RXsPath[j][i] < RXsPath[j][i + 1]) {
				std::sort(intersections.begin(), intersections.end(),
				[](const Point &a, const Point &b) {
					return a < b;
				});
			} else {
				std::sort(intersections.begin(), intersections.end(),
				[](const Point &a, const Point &b) {
					return b < a;
				});
			}

			for (const auto &p: intersections) newRXsPath[j].push_back(p);
		}
	}

	RXsPath.swap(newRXsPath);
	newRXsPath.clear();
	return;
}

void A_star_algorithm::simplifiedSpanningTree(vector<vector<Point>> &all, Point parent, int index) {
	if (all[0].size() - 1 == index) return;
	++index;

    set<Point> nextsFirst;
    for (const auto &single : all) {
        nextsFirst.insert(single[index]);
    }

    for (const auto &p : nextsFirst) {
        if (index) pathSegments.emplace_back(parent, p);
        vector<vector<Point>> nexts;
        for (const auto &single : all) {
            if (index < single.size() && single[index] == p) {
                nexts.push_back(single);
            }
        }
        simplifiedSpanningTree(nexts, index);
    }
	return;
}

void A_star_algorithm::makePathToSegments(bool hasRXs) {
	if (!hasRXs) {
		if (path.size() == 1) {
			pathSegments.emplace_back(path[0], path[0]);
			return;
		}
		for (int i = 0; i < path.size() - 1; ++i) {
			pathSegments.emplace_back(path[i], path[i + 1]);
		}
	} else {
		addNodesToRXsPath();
		simplifiedSpanningTree(RXsPath);
	}
	return;
}