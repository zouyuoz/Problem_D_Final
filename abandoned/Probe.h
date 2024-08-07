#ifndef PROBE_H_INCLUDE
#define PROBE_H_INCLUDE

#include "Point.h"
#include "Edge.h"
#include <vector>
#include <string>

class Probe {
public:
    Probe() {}
    Probe(Point coord, string zoneName, bool directionX, int level, Probe* parent);
    Point coord;
    string zoneName;
    bool directionX; // true if extended in X direction, false if in Y direction
    int level; // not sure if really need...
    Probe* parentProbe;

    Probe *extendedProbe(double dx, double dy, int lv);
    bool hitWall(vector<Edge> const &walls) const;
    bool alreadyExist(vector<Probe*> const &oldProbes) const;
};

#endif // PROBE_H_INCLUDE
