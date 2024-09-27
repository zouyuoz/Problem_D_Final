#ifndef POINT_H_INCLUDE
#define POINT_H_INCLUDE

#include <algorithm>
#include <iostream>

class Point {
public:
    Point(int x = -99, int y = -99) : x(x), y(y) {}

    int x, y;
	
    bool operator ==(const Point &other) const {
    	return (x == other.x && y == other.y);
    }

	bool operator<(const Point &other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
	}

	friend std::ostream& operator<<(std::ostream &os, const Point &p) {
		os << "(" << p.x << "," << p.y << ")";
		return os;
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