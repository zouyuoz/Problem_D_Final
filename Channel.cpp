#include "Channel.h"
#include <memory>

bool debugInfoChannel = 0;

Channel::Channel(Pair X, Pair Y): x(X), y(Y) {
	node = Point((x.min + x.max) / 2, (y.min + y.max) / 2);
}
Channel::Channel(Pair X, Pair Y, bool l): x(X), y(Y), isBPR(l) { /*for initialized bands*/ }

bool Channel::inBlock() { return !(!block); }

bool Channel::inNonfeed() {
	if (!block) return 0;
	return !block->is_feedthroughable;
}

bool Channel::enclose(const Point &p) { return p.x >= x.min && p.x <= x.max && p.y >= y.min && p.y <= y.max; }

bool Channel::canGo(shared_ptr<Channel>& other) {
	if (!other->valid) return 0;
	// if ()
	if (isBPR || other->isBPR) return 1;
	return (inNonfeed() == other->inNonfeed());
}

bool Channel::operator <=(const Channel &other) const { return this->x <= other.x && this->y <= other.y; }

std::ostream& operator <<(std::ostream& os, const Channel& channel) {
    os << channel.isBPR << " ";
	os << "\tx[" << channel.x.min << ", " << channel.x.max << "]\ty[" << channel.y.min << ", " << channel.y.max << "] ";
    return os;
}

void Channel_Manager::setXYvalue(set<int> X_value, set<int> Y_value) {
	x_value = X_value;
	y_value = Y_value;
	return;
}

shared_ptr<Block> checkInsideBlock(vector<shared_ptr<Block>> blocks, Point const &p) {
	for (auto const &o: blocks) {
		if (o->enclose(p)) return o;
	}
	return nullptr;
}

bool EdgeBelongs2Channel(const Edge &e, const Channel &channel) {
	bool edgeIsVertical = e.isVertical()
		&& (e.fixed() == channel.x.min || e.fixed() == channel.x.max)
		&& e.ranged().max >= channel.y.max && e.ranged().min <= channel.y.min;

	bool edgeIsHorizontal = !e.isVertical()
		&& (e.fixed() == channel.y.min || e.fixed() == channel.y.max)
		&& e.ranged().max >= channel.x.max && e.ranged().min <= channel.x.min;

	return (edgeIsVertical || edgeIsHorizontal);
}

void Channel_Manager::createChannels(
	const vector<shared_ptr<Block>> &allBlocks,
	const vector<Edge> &allBPRs,
	const vector<shared_ptr<EdgeNetNum>> &allTBENNs
) {
	for (auto x1 = x_value.begin(); x1 != x_value.end(); ++x1) {
        auto x2 = std::next(x1);
        if (x2 == x_value.end()) break;

		for (auto y1 = y_value.begin(); y1 != y_value.end(); ++y1) {
			auto y2 = std::next(y1);
			if (y2 == y_value.end()) break;

			Channel tempChannel(Pair(*x1, *x2), Pair(*y1, *y2));
			tempChannel.block = checkInsideBlock(allBlocks, tempChannel.node);

			for (auto const &e: allBPRs) {
				if (e.block == tempChannel.block && EdgeBelongs2Channel(e, tempChannel)) tempChannel.isBPR = 1;
			}
			for (auto const &enn: allTBENNs) {
				const Edge e = enn->edge;
				if (e.block == tempChannel.block && EdgeBelongs2Channel(e, tempChannel)) tempChannel.TBENN = enn;
			}
			nodes[tempChannel.node.x][tempChannel.node.y] = std::make_shared<Channel>(tempChannel);
		}
    }
}

shared_ptr<Channel> Channel_Manager::channelEnclose(const Point &p) {
	int X, Y;

    auto it = x_value.upper_bound(p.x);
    if (it != x_value.begin() && it != x_value.end()) {
		X = (*std::prev(it) + *it) / 2;
    }

    it = y_value.upper_bound(p.y);
    if (it != y_value.begin() && it != y_value.end()) {
		Y = (*std::prev(it) + *it) / 2;
    }

	auto x_it = nodes.find(X);
    if (x_it != nodes.end()) {
        auto y_it = x_it->second.find(Y);
		if (y_it != x_it->second.end()) return y_it->second;
		else cout << "We can't find the proper node\nWell somthing's wrong\n";
    }
	return nullptr;
}

/*-------------------------------------*/

