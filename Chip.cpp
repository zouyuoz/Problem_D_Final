#include "Chip.h"
#include <fstream>
#include <memory>

Chip::Chip(int const &testCase) {
	// Open chip_top.def to get
	// blockName, blkID, coordinate, facingFlip
	const string TESTCASE = std::to_string(testCase);
	std::ifstream file_chip_top("cad_case0" + TESTCASE + "/case0" + TESTCASE + "/chip_top.def");

	string line;
	string startWithBlock = "- BLOCK_";
	regex getBlockName(R"(BLOCK_\d+)");
	regex getBlkID(R"(blk_\d+)");
	regex getCoordinate(R"(\(\s*([\d.]+)\s+([\d.]+)\s*\))");

	string startWithRegion = "- REGION_";
	regex getRegionName(R"(REGION_\d+)");

	std::smatch m;
	string getFaceFlip;

	int lineNumber = 0;

	while (getline(file_chip_top, line)) {
		++lineNumber;
		string temp;

		// get UNITS DISTANCE MICRONS
		if (lineNumber == 7) {
			std::istringstream iss(line);
			iss >> temp >> temp >> temp >> UNITS_DISTANCE_MICRONS;
			Cell::unitsDistanceUm = UNITS_DISTANCE_MICRONS;
		}

		// get borders
		if (lineNumber == 8) {
			std::istringstream iss(line);
			int border_X, border_Y;
			iss >> temp >> temp >> temp >> temp >> temp >> temp >> border_X >> border_Y;
			border = Point(border_X, border_Y);
		}

		// read block informations
		if (line.find(startWithBlock) == 0) {
			shared_ptr<Block> tempBlock = std::make_shared<Block>();
			// blockName
			if (regex_search(line, m, getBlockName)) {
				tempBlock->name = m.str();
				// Open caseOO_cfg.json to get
				// through_block_net_num, through_block_edge_net_num
				// block_port_region, is_feedthroughable, is_tile
				std::ifstream file_cfg("cad_case0" + TESTCASE + "/case0" + TESTCASE + "_cfg.json");
				std::stringstream buffer;
				buffer << file_cfg.rdbuf();
				string jsonString = buffer.str();
				Document document;
				document.Parse(jsonString.c_str());
				for (const auto &block : document.GetArray()) {
					if (tempBlock->name != block["block_name"].GetString()) continue;

					tempBlock->through_block_net_num = block["through_block_net_num"].GetInt();
					tempBlock->is_feedthroughable = block["is_feedthroughable"].GetString() == string("True");
					tempBlock->is_tile = block["is_tile"].GetString() == string("True");

					// through_block_edge_net_num
					const Value &TBENN = block["through_block_edge_net_num"];
					for (const auto &tbenn : TBENN.GetArray()) {
    					if (!(tbenn.Size() == 3 && tbenn[0].IsArray() && tbenn[1].IsArray())) continue;
						Point a(tbenn[0][0].GetDouble() * UNITS_DISTANCE_MICRONS, tbenn[0][1].GetDouble() * UNITS_DISTANCE_MICRONS);
						Point b(tbenn[1][0].GetDouble() * UNITS_DISTANCE_MICRONS, tbenn[1][1].GetDouble() * UNITS_DISTANCE_MICRONS);
						Edge port = Edge(a, b, tempBlock);
						tempBlock->through_block_edge_net_num.push_back(std::make_shared<EdgeNetNum>(port, tbenn[2].GetInt()));
					}

					// block_port_region
					const Value &BPR = block["block_port_region"];
					for (auto const &thing : BPR.GetArray()) {
						Point a(thing.GetArray()[0][0].GetDouble() * UNITS_DISTANCE_MICRONS, thing.GetArray()[0][1].GetDouble() * UNITS_DISTANCE_MICRONS);
						Point b(thing.GetArray()[1][0].GetDouble() * UNITS_DISTANCE_MICRONS, thing.GetArray()[1][1].GetDouble() * UNITS_DISTANCE_MICRONS);
						Edge port = Edge(a, b, tempBlock);
						tempBlock->block_port_region.push_back(std::move(port));
					}
					if (tempBlock->name == block["block_name"].GetString()) break;
				}
				file_cfg.close();
			}

			// blkID
			if (regex_search(line, m, getBlkID)) {
				tempBlock->blkID = m.str();

				// Open blk file to get vertices
				std::ifstream file_blk("cad_case0" + TESTCASE + "/case0" + TESTCASE + "/" + tempBlock->blkID + ".def");
				int lineNum = 1;
				string verticesInfo;
				while (getline(file_blk, verticesInfo)) {
					if (lineNum == 8) break;
					lineNum++;
				}
				file_blk.close();
				sregex_iterator iter(verticesInfo.begin(), verticesInfo.end(), getCoordinate);
				sregex_iterator end;
				while (iter != end) {
					std::smatch match = *iter;
					int x = stoi(match[1].str());
					int y = stoi(match[2].str());
					tempBlock->vertices.push_back(Point(x, y));
					++iter;
				}
				if (tempBlock->vertices.size() == 2) tempBlock->expandVertices();
			}

			// getCoordinate
			if (regex_search(line, m, getCoordinate) && m.size() == 3) {
				int x = stoi(m[1]);
				int y = stoi(m[2]);
				tempBlock->coordinate = Point(x, y);
			}

			// getFacingFlip
			tempBlock->facingFlip = line.substr(line.length() - 4, 2);

			// do faceAndFlip and shift to ALL members have vertices
			tempBlock->transposeAllVertices();

			// store the fliped and shifted vertices into edges
			tempBlock->verticesToEdges();

			tempBlock->adjustAllPorts();

			// collect edges from tempBlock, and write into totEdge
			for (Edge &e: tempBlock->edges) allEdges.push_back(e);
			for (Edge &e: tempBlock->block_port_region) allBPRs.push_back(e);
			for (auto &e: tempBlock->through_block_edge_net_num) allTBENNs.push_back(e);
			// turn off this when testing

			// write into totZone
			allBlocks.push_back(std::move(tempBlock));
		}

		// read region information
		Region tempRegion;
		if (line.find(startWithRegion) == 0) {
			// regionName
			if (regex_search(line, m, getRegionName)) {
				tempRegion.name = m.str();
			}
			// vertices
			sregex_iterator iter(line.begin(), line.end(), getCoordinate);
			sregex_iterator end;
			while (iter != end) {
				std::smatch match = *iter;
				int x = stoi(match[1].str());
				int y = stoi(match[2].str());
				tempRegion.vertices.push_back(Point(x, y));
				++iter;
			}
			tempRegion.expandVertices();
			allRegions.push_back(Region(std::move(tempRegion)));
		}
	}
	file_chip_top.close();

	// Edges: already have block Edges
	// here adding the chip border
	shared_ptr<Block> BORDER = std::make_shared<Block>("border");
	BORDER->vertices = { Point(0, 0), Point(0, border.y), border, Point(border.x, 0) };
	BORDER->verticesToEdges();
	for (Edge &e : BORDER->edges) allEdges.push_back(e);
	allBlocks.push_back(std::move(BORDER));

	// make it ordered
	std::sort(allEdges.begin(), allEdges.end(),
		[](const auto& a, const auto& b) {
			return a.fixed() < b.fixed();
		}
	);  
}

shared_ptr<Block> Chip::getBlock(string blockName) const {
    for (auto &b : allBlocks) {
        if (b->name == blockName) return b;
    }
	return nullptr;
}

Region Chip::getRegion(string regionName) const {
	for (auto const &r : allRegions) {
		if (r.name == regionName) return r;
	}
	return Region();
}

Chip::~Chip() {}