#include "Net.h"

Net::Net(Terminal tx, Terminal rx): TX(tx) {
	RXs.push_back(rx);
}
Net::Net(int id, Terminal tx, Terminal rx): ID(id), TX(tx) {
	RXs.push_back(rx);
}

void Net::ParserAllNets (int const &testCase, Chip const &chip) {
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
		if (tempNet.TX.name[0] == 'B') tempNet.TX.block = chip.getBlock(tempNet.TX.name);
		// TX_COORD:
		tempNet.TX.coord = Point(net["TX_COORD"][0].GetDouble() * UDM, net["TX_COORD"][1].GetDouble() * UDM);
		tempNet.TX.absoluteCoord(chip);

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
			if (tempRXNameArray[i][0] == 'B') RX.block = chip.getBlock(tempRXNameArray[i]);
			RX.absoluteCoord(chip);
			non_repeat_rxs.insert(RX);
	    }
		for (auto it = non_repeat_rxs.begin(); it !=non_repeat_rxs.end(); ++it) tempNet.RXs.push_back(*it);
		non_repeat_rxs.clear();

		// NUM:
		tempNet.num = net["NUM"].GetInt();

		// HMFT_MUST_THROUGH:
        for (const auto &hmftmt : net["HMFT_MUST_THROUGH"].GetObject()) {
            MUST_THROUGH tempHmftmt;
            tempHmftmt.blockName = hmftmt.name.GetString();
			Point blockCoord = chip.getBlock(tempHmftmt.blockName)->coordinate;
			for (auto const &coord : hmftmt.value.GetArray()) {
				Point first(coord[0].GetDouble() * UDM + blockCoord.x, coord[1].GetDouble() * UDM + blockCoord.y);
				Point second(coord[2].GetDouble() * UDM + blockCoord.x, coord[3].GetDouble() * UDM + blockCoord.y);
				tempHmftmt.edges.push_back(Edge(first, second));
			}
            tempNet.HMFT_MUST_THROUGHs.push_back(tempHmftmt);
        }

        // MUST_THROUGH:
        for (const auto &mt : net["MUST_THROUGH"].GetObject()) {
            MUST_THROUGH tempMt;
            tempMt.blockName = mt.name.GetString();
			Point blockCoord = chip.getBlock(tempMt.blockName)->coordinate;
			for (auto const &coord : mt.value.GetArray()) {
				Point first(coord[0].GetDouble() * UDM + blockCoord.x, coord[1].GetDouble() * UDM + blockCoord.y);
				Point second(coord[2].GetDouble() * UDM + blockCoord.x, coord[3].GetDouble() * UDM + blockCoord.y);
				tempMt.edges.push_back(Edge(first, second));
			}
            tempNet.MUST_THROUGHs.push_back(tempMt);
        }

		// write into net
		allNets.push_back(tempNet);
	}
	file.close();
	return;
}

Net Net::getNet(int const &id) const {
	for (const Net &n : allNets) {
		if (n.ID == id) return n;
	}
	return Net();
}

void Terminal::absoluteCoord (Chip const &chip) {
    int x = 0, y = 0;
    if (name[0] == 'B') {
        x = chip.getBlock(name)->coordinate.x;
        y = chip.getBlock(name)->coordinate.y;
    }
    coord.x += x;
    coord.y += y;
	return;
}

Edge findBoundBox(vector<Point> const &coords){
	int x_min = coords[0].x, x_max = coords[0].x, y_min = coords[0].y, y_max = coords[0].y;
	for (auto const &c : coords) {
		int x_this = c.x, y_this = c.y;
		if (x_this > x_max) x_max = x_this;
		if (x_this < x_min) x_min = x_this;
		if (y_this > y_max) y_max = y_this;
		if (y_this < y_min) y_min = y_this;
	}
	return Edge(Point(x_min, y_min), Point(x_max, y_max));
}

Edge Net::getBoundBoxArea() const {
	vector<Point> coords;
	for(Terminal const &rx : RXs){
		coords.push_back(rx.coord);
	}
	coords.push_back(TX.coord);
	return findBoundBox(coords);
}

void Net::showNetInfo() const {
	cout << "ID: " << ID << "\n"
	<< "TX: " << TX.name << " (" << TX.coord.x << ", " << TX.coord.y << ")" << "\n"
	<< "RXs: " << "\n";
	for(const Terminal &rx: RXs){
		cout << " - " << rx.name << " (" << rx.coord.x << ", " << rx.coord.y << ")" << "\n";
	}
	cout << "NUM: " << num << "\n"
	<< "MUST_THROUGH: " << "\n";
	if (MUST_THROUGHs.size()) {
		for (const MUST_THROUGH &t : MUST_THROUGHs) {
			cout << " - " << t.blockName;
			Edge c = t.edges[0];
			cout << " (" << c.first.x << ", " << c.first.y << ") (" << c.second.x << ", " << c.second.y << ")\n";
			if (t.edges.size() == 1) continue;
			else {
				for (int i = 0; i < t.blockName.size() + 3; i++) { cout << " "; }
				Edge c = t.edges[1];
				cout << " (" << c.first.x << ", " << c.first.y << ") (" << c.second.x << ", " << c.second.y << ")\n";
			}
		}
	}
	cout << "HMFT_MUST_THROUGH: " << "\n";
	if (HMFT_MUST_THROUGHs.size()) {
		for (const MUST_THROUGH &t : HMFT_MUST_THROUGHs){
			cout << " - " << t.blockName;
			Edge c = t.edges[0];
			cout << " (" << c.first.x << ", " << c.first.y << ") (" << c.second.x << ", " << c.second.y << ")\n";
			if (t.edges.size() == 1) { continue; }
			else {
				for (int i = 0; i < t.blockName.size() + 3; i++) { cout << " "; }
				Edge c = t.edges[1];
				cout << " (" << c.first.x << ", " << c.first.y << ") (" << c.second.x << ", " << c.second.y << ")\n";
			}
		}
	}
	cout << "----------------------" << "\n";
}