#ifndef ALGORITHM_H_INCLUDED
#define ALGORITHM_H_INCLUDED

#include "Net.h"
#include "Band.h"
#include <cstddef>
#include <iomanip>

class A_star_algorithm {
public:
	A_star_algorithm(Net &net, Chip &chip);
	vector<Point> path;
};

#endif // ALGORITHM_H_INCLUDED