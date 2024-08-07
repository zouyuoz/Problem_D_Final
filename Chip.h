#ifndef CHIP_H_INCLUDED
#define CHIP_H_INCLUDED

#include "Block.h"
#include "Region.h"
#include "Edge.h"
#include "Zone.h"
#include "Channel.h"

#include "include\rapidjson\document.h"
#include <fstream>
#include <iomanip>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <sstream>
#include <string>

using std::unique_ptr;
using std::regex;
using std::sregex_iterator;
using namespace rapidjson;

class Chip : public Zone {
public:
	Chip() {}
	Chip(int const &testCase);
	void initializeAllChannel();
	void initializeAllGrid();

	Point border;
	int UNITS_DISTANCE_MICRONS;
	
	vector<shared_ptr<Block>> allBlocks;
	vector<Region> allRegions;
	vector<Edge> allEdges;
	vector<Edge> allBPRs;
	vector<shared_ptr<EdgeNetNum>> allTBENNs;
	Channel_Manager allChannels;

	shared_ptr<Block> getBlock(string) const;
	Region getRegion(string) const;
	void showAllZones() const;
	~Chip();
};

#endif // CHIP_H_INCLUDED