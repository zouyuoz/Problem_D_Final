#include "Net.h"
#include <cstddef>
#include <memory>

Net::Net(Terminal tx, Terminal rx): TX(tx) {
	RXs.push_back(rx);
}
Net::Net(int id, Terminal tx, Terminal rx): ID(id), TX(tx) {
	RXs.push_back(rx);
}
Net Net::copyConstr() const {
	Net theCopied;
	theCopied.ID = ID;
	theCopied.num = num;
	theCopied.TX = TX;
	theCopied.bBox = bBox;
	return std::move(theCopied);
}

void Net_Manager::ParserAllNets (int const &testCase, Chip const &chip) {
	const int UDM = chip.UNITS_DISTANCE_MICRONS;
	const string TESTCASE = std::to_string(testCase);
	std::ifstream file("cad_case0" + TESTCASE + "/case0" + TESTCASE + ".json");
	std::stringstream buffer;
	buffer << file.rdbuf();
	string jsonString = buffer.str();
	Document document;
	document.Parse(jsonString.c_str());
	for (auto const &net : document.GetArray()) {
		Net tempNet;

		// ID:
		tempNet.ID = net["ID"].GetInt();
		// TX_NAME:
		tempNet.TX.name = net["TX"].GetString();
		// TX_COORD:
		tempNet.TX.coord = Point(net["TX_COORD"][0].GetDouble() * UDM, net["TX_COORD"][1].GetDouble() * UDM);

		if (tempNet.TX.name[0] == 'B') {
			tempNet.TX.block = chip.getBlock(tempNet.TX.name);
			tempNet.TX.absoluteCoord();
		}
		else for (const auto &b: chip.allBlocks) {
			if (b->enclose(tempNet.TX.coord)) { tempNet.TX.block = b; break; }
		}

		// RX_NAME:
		vector<string> tempRXNameArray;
		for (const auto &rxName : net["RX"].GetArray()) {
			tempRXNameArray.push_back(rxName.GetString());
		}
		// RX_COORD:
		vector<Point> tempRXCoordArray;
		for (const auto &coord : net["RX_COORD"].GetArray()) {
			tempRXCoordArray.push_back(Point(coord[0].GetDouble() * UDM, coord[1].GetDouble() * UDM));
		}
		// Write into RXs
		std::set<Terminal> non_repeat_rxs;
		for (int i = 0; i < tempRXNameArray.size(); i++){
			Terminal RX(tempRXNameArray[i], tempRXCoordArray[i]);
			if (tempRXNameArray[i][0] == 'B') {
				RX.block = chip.getBlock(tempRXNameArray[i]);
				RX.absoluteCoord();
			}
			else for (const auto &b: chip.allBlocks) {
				if (b->enclose(RX.coord)) { RX.block = b; break; }
			}
			
			non_repeat_rxs.insert(RX);
		}
		for (auto it = non_repeat_rxs.begin(); it !=non_repeat_rxs.end(); ++it) tempNet.RXs.push_back(*it);
		non_repeat_rxs.clear();

		// getBBox
		tempNet.getBBox();

		// NUM:
		tempNet.num = net["NUM"].GetInt();
		if (tempNet.num > max_net_num) max_net_num = tempNet.num;

		// HMFT_MUST_THROUGH:
		for (const auto &mt : net["HMFT_MUST_THROUGH"].GetObject()) {
			Edge tempMT;
			string blockName = mt.name.GetString();
			auto tempMTBlock = chip.getBlock(blockName);
			Point blockCoord = tempMTBlock->coordinate;
			for (auto const &coord : mt.value.GetArray()) {
				Point first(coord[0].GetDouble() * UDM + blockCoord.x, coord[1].GetDouble() * UDM + blockCoord.y);
				Point second(coord[2].GetDouble() * UDM + blockCoord.x, coord[3].GetDouble() * UDM + blockCoord.y);
				tempMT = Edge(first, second, tempMTBlock);
				tempMTBlock->adjustPortCoordinate(tempMT);
				tempMT.netID = tempNet.ID;
				tempNet.HMFT_MUST_THROUGHs.push_back(tempMT);
			}
		}

		// MUST_THROUGH:
		for (const auto &mt : net["MUST_THROUGH"].GetObject()) {
			Edge tempMT;
			string blockName = mt.name.GetString();
			auto tempMTBlock = chip.getBlock(blockName);
			Point blockCoord = tempMTBlock->coordinate;
			for (auto const &coord : mt.value.GetArray()) {
				Point first(coord[0].GetDouble() * UDM + blockCoord.x, coord[1].GetDouble() * UDM + blockCoord.y);
				Point second(coord[2].GetDouble() * UDM + blockCoord.x, coord[3].GetDouble() * UDM + blockCoord.y);
				tempMT = Edge(first, second, tempMTBlock);
				tempMTBlock->adjustPortCoordinate(tempMT);
				tempMT.netID = tempNet.ID;
				tempNet.MUST_THROUGHs.push_back(tempMT);
			}
		}
		if (tempNet.HMFT_MUST_THROUGHs.size() || tempNet.MUST_THROUGHs.size()) {
			tempNet.setOrderedMTs();
			allOrderedMTs.insert(allOrderedMTs.end(), tempNet.orderedMTs.begin(), tempNet.orderedMTs.end());
		}
		// 50 57 1014, 1014 interesting
		// allNets.push_back(tempNet);
		totalNets.push_back(std::move(tempNet));
	}
	file.close();
	return;
}

Net Net::getSoleNet(int const &id) const {
	Net soleNet = copyConstr();
	soleNet.RXs.clear();
	soleNet.RXs.push_back(RXs[id]);
	return soleNet;
}

/*=======================================*/

void insertEdgeCoords2Set(set<int> &x_value, set<int> &y_value, const Edge &e) {
	x_value.insert(e.first.x);
	x_value.insert(e.second.x);
	y_value.insert(e.first.y);
	y_value.insert(e.second.y);
	return;
}

void Chip::initializeAllCell(const Net_Manager &net) {
	set<int> x_value, y_value;

	for (size_t i = 0; i < allEdges.size(); ++i) insertEdgeCoords2Set(x_value, y_value, allEdges[i]);
	for (size_t i = 0; i < allBPRs.size(); ++i) insertEdgeCoords2Set(x_value, y_value, allBPRs[i]);
	for (size_t i = 0; i < allTBENNs.size(); ++i) {
		auto e = allTBENNs[i]->edge;
		insertEdgeCoords2Set(x_value, y_value, e);
	}
	for (size_t i = 0; i < net.allOrderedMTs.size(); ++i) insertEdgeCoords2Set(x_value, y_value, net.allOrderedMTs[i]);

    allCells.setXYvalue(x_value, y_value);
	allCells.createCells(allBlocks, allBPRs, allTBENNs, net);
	return;
}

void Cell_Manager::createCells(
	const vector<shared_ptr<Block>> &allBlocks,
	const vector<Edge> &allBPRs,
	const vector<shared_ptr<EdgeNetNum>> &allTBENNs,
	const Net_Manager net
) {
	int x_count = 0;
	for (auto x1 = x_value.begin(); x1 != x_value.end(); ++x1) {
        auto x2 = std::next(x1);
        if (x2 == x_value.end()) break;
		int y_count = -1;

		for (auto y1 = y_value.begin(); y1 != y_value.end(); ++y1) {
			auto y2 = std::next(y1);
			if (y2 == y_value.end()) break;

			Cell tempCell(Pair(*x1, *x2), Pair(*y1, *y2));
			tempCell.checkInsideBlock(allBlocks);

			for (auto const &e: allBPRs) {
				if (tempCell.EdgeBelongs2Cell(e)) tempCell.BPR = e;
			}
			for (auto const &enn: allTBENNs) {
				const Edge e = enn->edge;
				if (tempCell.EdgeBelongs2Cell(e)) tempCell.TBENN = enn;
			}
			for (auto const &e: net.allOrderedMTs) {
				if (tempCell.EdgeBelongs2Cell(e)) {
					tempCell.someNetsMTs.push_back(e);
				}
			}
			// if (tempCell.someNetsMTs.size() > 1) {
			// 	cout << "\n";
			// 	for (auto const &e: tempCell.someNetsMTs) {
			// 		cout << e.block->name << " : " << e.netID << "\n";
			// 	}
			// }
			tempCell.xIndex = x_count;
			tempCell.yIndex = ++y_count;
			cells[x_count][y_count] = std::make_shared<Cell>(tempCell);
		}
		++x_count;
    }
}

/*=======================================*/

void Terminal::absoluteCoord() {
	if (name[0] == 'B') {
		coord.x += block->coordinate.x;
		coord.y += block->coordinate.y;
	}
	return;
}

void Net::getBBox() {
	vector<Point> coords;
	for(Terminal const &rx : RXs){
		coords.push_back(rx.coord);
	}
	coords.push_back(TX.coord);

	int x_min = coords[0].x, x_max = x_min, y_min = coords[0].y, y_max = y_min;
	for (auto const &c : coords) {
		int x_this = c.x, y_this = c.y;
		if (x_this > x_max) x_max = x_this;
		if (x_this < x_min) x_min = x_this;
		if (y_this > y_max) y_max = y_this;
		if (y_this < y_min) y_min = y_this;
	}
	bBox = Edge(Point(x_min, y_min), Point(x_max, y_max));
	return;
}
int Net::bBoxArea() const {
	int x_edge_length_mod = (bBox.second.x - bBox.first.x) / 2000;
	int y_edge_length_mod = (bBox.second.y - bBox.first.y) / 2000;
	return x_edge_length_mod * y_edge_length_mod;
}
int Net::bBoxHPWL() const {
	return (bBox.second.x - bBox.first.x) + (bBox.second.y - bBox.first.y);
}

void Net::setOrderedMTs() {
	orderedMTs = MUST_THROUGHs;
	if (!HMFT_MUST_THROUGHs.size()) return;

	auto mt = HMFT_MUST_THROUGHs[0];
	Point edgeMidPoint((mt.first.x + mt.second.x) / 2, (mt.first.y + mt.second.y) / 2);
	int mDistanceToSource = (edgeMidPoint.x - TX.coord.x) + (edgeMidPoint.y - TX.coord.y);
	int mDistanceToTarget = (edgeMidPoint.x - RXs[0].coord.x) + (edgeMidPoint.y - RXs[0].coord.y);

	if (mDistanceToSource < mDistanceToTarget) orderedMTs.insert(orderedMTs.begin(),mt);
	else orderedMTs.push_back(mt);
	return;
}

std::ostream& operator <<(std::ostream& os, const Net& net) {
	os << "ID: " << net.ID << "\n"
	<< "TX: " << net.TX.name << " (" << net.TX.coord.x << ", " << net.TX.coord.y << ")" << ", In ";
	if (net.TX.block) os << net.TX.block->name << "\n";
	else os << "no block\n";
	os << "RXs: " << "\n";
	for(const Terminal &rx: net.RXs){
		os << " - " << rx.name << " (" << rx.coord.x << ", " << rx.coord.y << ")" << ", In ";
		if (rx.block) os << rx.block->name << "\n";
		else os << "no block\n";
	}
	os << "NUM: " << net.num << "\n"
	<< "MUST_THROUGH: " << "\n";
	if (net.MUST_THROUGHs.size()) {
		for (const Edge &t : net.MUST_THROUGHs) {
			os << " - " << t.block->name;
			os << " (" << t.first.x << ", " << t.first.y << ") (" << t.second.x << ", " << t.second.y << ")\n";
		}
	}
	os << "HMFT_MUST_THROUGH: " << "\n";
	if (net.HMFT_MUST_THROUGHs.size()) {
		for (const Edge &t : net.HMFT_MUST_THROUGHs){
			os << " - " << t.block->name;
			os << " (" << t.first.x << ", " << t.first.y << ") (" << t.second.x << ", " << t.second.y << ")\n";
		}
	}
	os << "----------------------" << "\n";
	return os;
}
bool Net::operator <(const Net& other) const {
	if (bBoxArea() != other.bBoxArea()) return bBoxArea() < other.bBoxArea();
	if (bBoxHPWL() != other.bBoxHPWL()) return bBoxHPWL() < other.bBoxHPWL();
	return ID < other.ID;
}