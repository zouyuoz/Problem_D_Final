#ifndef CHANNEL_H_INCLUDED
#define CHANNEL_H_INCLUDED

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

class Terminal {
public:
	Terminal() {}
	Terminal(string Name, Point Coord) : name(Name), coord(Coord) {}
	string name;
	shared_ptr<Block> block = nullptr;
	Point coord;
	void absoluteCoord(Chip const &chip);

	bool operator <(const Terminal &other) const { return coord < other.coord; }
};

class Channel : public std::enable_shared_from_this<Channel> {
public: 
	Channel(Pair, Pair);
	Channel(Pair, Pair, bool);

    Pair x, y;
	vector<shared_ptr<Channel>> next;
	shared_ptr<Block> block = nullptr;

	Point node;
	bool inBlock();
	bool inNonfeed();
	bool valid = 1;
	bool isBPR = 0;
	shared_ptr<EdgeNetNum> TBENN;

    bool enclose(const Point&);
	bool canGo(shared_ptr<Channel>&);

    bool operator <=(const Channel &other) const;
	bool operator <(const Channel &other) const;
    friend std::ostream& operator<<(std::ostream& os, const Channel& b);
};

class Channel_Manager {
private:
	set<int> x_value, y_value;
public:
	Channel_Manager() {}
	map<int, map<int, shared_ptr<Channel>>> nodes;
	void setXYvalue(set<int>, set<int>);
	void createChannels(const vector<shared_ptr<Block>>&, const vector<Edge>&, const vector<shared_ptr<EdgeNetNum>>&);
	shared_ptr<Channel> channelEnclose(const Point&);
};

#endif // CHANNEL_H_INCLUDED