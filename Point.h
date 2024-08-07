#ifndef POINT_H_INCLUDE
#define POINT_H_INCLUDE

#include <algorithm>

class Point {
public:
    Point(int x = 99, int y = 99) : x(x), y(y) {}

    int x, y;
	
    bool operator ==(const Point &other) const {
    	return (x == other.x && y == other.y);
    }

	bool operator<(const Point &other) const {
		if (x + y < other.x + other.y) return 1;
		if (x + y > other.x + other.y) return 0;
		return x < other.x;
	}
};

class Pair {
public:
	Pair() : min(0), max(0) {}
	Pair(int Min, int Max) {
		// Min should smaller than Max, 
		// but in case we still check that
		this->min = std::min(Min, Max);
		this->max = std::max(Min, Max);
	}

	int min, max;

	void operator =(Pair const &other) {
		this->min = other.min;
		this->max = other.max;
	}

    bool operator ==(const Pair &other) const {
    	return (min == other.min && max == other.max);
    }

	bool operator <=(const Pair &other) const {
		return (min >= other.min && max <= other .max);
	}
};

#endif // POINT_H_INCLUDE