#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include <stack>

using namespace std;

class Pair {
public:
    double min, max;
    Pair(double min, double max) : min(min), max(max) {}
};

class Point {
public:
    double x, y;
    Point(double x, double y) : x(x), y(y) {}
};

class Block {};

class Channel : public enable_shared_from_this<Channel> {
public:
    Pair x, y;
    shared_ptr<Channel> prev;
    vector<pair<shared_ptr<Channel>, Point>> next;
    vector<pair<shared_ptr<Channel>, Point>> nextForNet;
    vector<shared_ptr<Block>> containFeedBlocks; 
    int edge_net_num = -1;
    bool shouldDelete = 0; 
    bool isBPR = 0;
    Point center;

    Channel(Pair, Pair);
    Channel(Pair, Pair, bool);

    bool intersected(shared_ptr<Channel> const &other);
    vector<shared_ptr<Channel>> splitChannel();
    bool operator <=(const Channel &other) const;
    bool operator <(const Channel &other) const;
    friend ostream& operator<<(ostream& os, const Channel& b);
};

Channel::Channel(Pair X, Pair Y) : x(X), y(Y), center((X.min + X.max) / 2, (Y.min + Y.max) / 2) {}
Channel::Channel(Pair X, Pair Y, bool l) : x(X), y(Y), isBPR(l), center((X.min + X.max) / 2, (Y.min + Y.max) / 2) {}

bool Channel::intersected(shared_ptr<Channel> const &other) {
    bool x_not_intersect = x.min >= other->x.max || x.max <= other->x.min;
    bool y_not_intersect = y.min >= other->y.max || y.max <= other->y.min;
    if (x_not_intersect || y_not_intersect) return 0;
    
    double X = (max(x.min, other->x.min) + min(x.max, other->x.max)) / 2;
    double Y = (max(y.min, other->y.min) + min(y.max, other->y.max)) / 2;
    next.push_back(make_pair(other, Point(X, Y))); 
    return 1;
}

bool Channel::operator <=(const Channel &other) const { 
    return this->x.min <= other.x.min && this->x.max <= other.x.max && this->y.min <= other.y.min && this->y.max <= other.y.max;
}

ostream& operator<<(ostream& os, const Channel& channel) {
    os << channel.isBPR << " ";
    os << "\tx[" << channel.x.min << ", " << channel.x.max << "]\ty[" << channel.y.min << ", " << channel.y.max << "] ";
    return os;
}

vector<shared_ptr<Channel>> findPath(shared_ptr<Channel> source, shared_ptr<Channel> target) {
    stack<shared_ptr<Channel>> stack;
    vector<shared_ptr<Channel>> path;
    source->prev = nullptr;
    stack.push(source);

    while (!stack.empty()) {
        shared_ptr<Channel> current = stack.top();
        stack.pop();

        if (current == target) {
            while (current != nullptr) {
                path.push_back(current);
                current = current->prev;
            }
            reverse(path.begin(), path.end());
            return path;
        }

        for (auto& next_channel : current->next) {
            shared_ptr<Channel> next = next_channel.first;
            if (next->prev == nullptr && next != source) {  // avoid revisiting
                next->prev = current;
                stack.push(next);
            }
        }
    }
    return path; // return empty if no path is found
}

int main() {
    // 示例用法
	double a = 1.531;
	int b = a*  2000;
	std::cout << a*2000 << "\n" << b << "\n"; 
    shared_ptr<Channel> source = make_shared<Channel>(Pair(0, 2), Pair(0, 2));
    shared_ptr<Channel> target = make_shared<Channel>(Pair(4, 6), Pair(4, 6));
    shared_ptr<Channel> mid = make_shared<Channel>(Pair(1, 5), Pair(1, 5));

    source->next.push_back(make_pair(mid, Point(1.5, 1.5)));
    mid->next.push_back(make_pair(target, Point(4.5, 4.5)));

    vector<shared_ptr<Channel>> path = findPath(source, target);

    if (!path.empty()) {
        cout << "Path found:\n";
        for (const auto& channel : path) {
            cout << *channel << "\n";
			cerr << "test\n";
        }
    } else {
        cout << "No path found.\n";
    }

    return 0;
}
