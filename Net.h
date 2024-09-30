#ifndef NET_H_INCLUDE
#define NET_H_INCLUDE

#include "Chip.h"
#include <cmath>
#include "include\rapidjson\document.h"
#include <array>
#include <memory>

using namespace rapidjson;
using std::shared_ptr;

class Net {
public:
	Net() {}
	Net(Terminal TX, Terminal RX);
	Net(int ID, Terminal TX, Terminal RX);

	int ID;
	int num;
	int max_net_num = 0;
	Terminal TX;
	vector<Terminal> RXs;
	vector<Edge> MUST_THROUGHs;
	vector<Edge> HMFT_MUST_THROUGHs;
	vector<Edge> orderedMTs;

	vector<Simple_Edge> path;

	Edge bBox;
	void getBBox();
	int bBoxArea() const;
	int bBoxHPWL() const;

	Net copyConstr() const;
	Net getSoleNet(int const &ID) const;
	void setOrderedMTs();
    friend std::ostream& operator<<(std::ostream& os, const Net& net);
	bool operator <(const Net& other) const;
};

class Net_Manager {
public:
	Net_Manager() {}
	int max_net_num = 0;
	vector<Net> totalNets;
	vector<Edge> allOrderedMTs;

	void ParserAllNets(int const &testCase, Chip const &chip);
};

#endif // NET_H_INCLUDE
