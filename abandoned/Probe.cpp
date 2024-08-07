#include "Probe.h"
#include <cmath>
#include <stdexcept>

Probe::Probe(Point coord, std::string zoneName, bool directionX, int level, Probe* parent)
    : coord(coord), zoneName(zoneName), directionX(directionX), level(level), parentProbe(parent) {}

Probe *Probe::extendedProbe(double dx, double dy, int lv) {
    Point newPoint(coord.x + dx, coord.y + dy);
    return new Probe(newPoint, zoneName, !directionX, lv, this);
}

bool Probe::hitWall(const std::vector<Edge>& walls) const {
    for(const Edge &w : walls) {
        if (zoneName == w.name) continue;
        if (!directionX && w.isVertical) {
            if(coord.x == w.fixedCoord && w.inRange(coord.y)) {
                return true;
            }
        }
        if (directionX && !w.isVertical) {
            if(coord.y == w.fixedCoord && w.inRange(coord.x)) {
                return true;
            }
        }
    }
    const double boundaryL = 0.0;
    const double boundaryR = 66.0;
    const double boundaryD = 0.0;
    const double boundaryU = 31.0;
    if (coord.x >= boundaryR || coord.x <= boundaryL || coord.y >= boundaryU || coord.y <= boundaryD) {
        return true;
    }
    return false;
}

bool Probe::alreadyExist(vector<Probe*> const &oldProbes) const {
    for (Probe *p : oldProbes) {
        if (coord == p->coord) return true;
    }
    return false;
}
