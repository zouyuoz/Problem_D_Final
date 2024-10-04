#ifndef ZONE_H_INCLUDED
#define ZONE_H_INCLUDED

#include "Point.h"

#include <string>
#include <utility>
#include <vector>

using std::string;
using std::vector;

class Zone {
public:
	Zone() {}
	string name;
	vector<Point> vertices;

	void expandVertices() {
		Point tempVertice(vertices[0].x, vertices[1].y);
		vertices.insert(vertices.begin() + 1, tempVertice);
		tempVertice = Point(vertices[2].x, vertices[0].y);
		vertices.push_back(tempVertice);
	}
	
	virtual ~Zone() {}
};

#endif // ZONE_H_INCLUDED