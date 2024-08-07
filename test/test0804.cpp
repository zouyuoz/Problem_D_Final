#include <vector>
#include <algorithm>
#include <set>

struct Pair {
    int min, max;
    Pair(int _min = 0, int _max = 0) : min(_min), max(_max) {}
};

class Channel {
public:
    Pair x, y;
    Channel(int x1 = 0, int y1 = 0, int x2 = 0, int y2 = 0)
        : x(x1, x2), y(y1, y2) {}

    bool operator<(const Channel& other) const {
        if (x.min != other.x.min) return x.min < other.x.min;
        if (y.min != other.y.min) return y.min < other.y.min;
        if (x.max != other.x.max) return x.max < other.x.max;
        return y.max < other.y.max;
    }

    bool operator==(const Channel& other) const {
        return x.min == other.x.min && x.max == other.x.max &&
               y.min == other.y.min && y.max == other.y.max;
    }
};

std::vector<Channel> splitAndDeduplicateChannels(const std::vector<Channel>& channels) {
    std::vector<int> x_coords, y_coords;
    for (const auto& channel : channels) {
        x_coords.push_back(channel.x.min);
        x_coords.push_back(channel.x.max);
        y_coords.push_back(channel.y.min);
        y_coords.push_back(channel.y.max);
    }

    std::sort(x_coords.begin(), x_coords.end());
    std::sort(y_coords.begin(), y_coords.end());
    x_coords.erase(std::unique(x_coords.begin(), x_coords.end()), x_coords.end());
    y_coords.erase(std::unique(y_coords.begin(), y_coords.end()), y_coords.end());

    std::set<Channel> result;
    for (size_t i = 0; i < x_coords.size() - 1; ++i) {
        for (size_t j = 0; j < y_coords.size() - 1; ++j) {
            int x1 = x_coords[i], x2 = x_coords[i + 1];
            int y1 = y_coords[j], y2 = y_coords[j + 1];
            
            for (const auto& channel : channels) {
                if (x1 >= channel.x.min && x2 <= channel.x.max &&
                    y1 >= channel.y.min && y2 <= channel.y.max) {
                    result.insert(Channel(x1, y1, x2, y2));
                    break;
                }
            }
        }
    }

    return std::vector<Channel>(result.begin(), result.end());
}

// Example usage
#include <iostream>

int main() {
    std::vector<Channel> channels = {
        Channel(1900,0,2100,300),
		Channel(300,0,1000,400),
		Channel(0,0,300,600),
		Channel(2100,0,2500,1400),
		Channel(1000,0,1100,1500),
		Channel(1100,0,1400,2500),
		Channel(0,400,300,600),
		Channel(0,1500,400,2200),
		Channel(0,2200,600,2500),
		Channel(0,0,1400,400),
		Channel(0,1400,1500,1500),
		Channel(0,1100,2500,1400),
		Channel(1000,400,1400,1000),
		Channel(1000,1000,2500,1100),
		Channel(1500,1000,2100,1400),
		Channel(1400,1000,1500,2500),
		Channel(400,1100,1000,1500),
		Channel(0,1100,400,2500),
		Channel(1100,1500,1500,2100),
		Channel(1100,2100,2500,2500),
		Channel(2200,1900,2500,2500),
		Channel(1900,0,2500,300),
		Channel(1500,2100,2200,2500),
		Channel(2100,300,2500,1000),
		Channel(2200,1900,2500,2100),
		Channel(400,2200,600,2500)
    };

    std::vector<Channel> result = splitAndDeduplicateChannels(channels);

    for (const auto& channel : result) {
        std::cout << channel.x.min << "," << channel.y.min << ","
                  << channel.x.max << "," << channel.y.max << ",0" << std::endl;
    }

    return 0;
}