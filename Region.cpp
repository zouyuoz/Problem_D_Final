#include "Region.h"

Region::Region() {}

Region::Region(Region const &r) {
	this->name = r.name;
	this->vertices = r.vertices;
}

void Region::showRegionInfo() const {
	cout << name << "\n";
	for (Point v : vertices) {
		cout << "(" << v.x << ", " << v.y << ")" << "\n";
	}
	cout << "----------------------" << "\n";
}