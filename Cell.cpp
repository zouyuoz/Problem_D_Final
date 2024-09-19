#include "Cell.h"
#include <memory>

bool debugInfoCell = 0;

Cell::Cell(Pair X, Pair Y): x(X), y(Y) {
	node = Point((x.min + x.max) / 2, (y.min + y.max) / 2);
}

bool Cell::inBlock() { return !(block == nullptr); }

bool Cell::inNonfeed() {
	if (!block) return 0;
	return !block->is_feedthroughable;
}

bool Cell::enclose(const Point &p) { return p.x >= x.min && p.x <= x.max && p.y >= y.min && p.y <= y.max; }

bool Cell::canGo(const shared_ptr<Cell>& other) { // now abandoned
	if (!other->valid()) return 0;
	// if (isBPR || other->isBPR) return 1;
	if (other->block) {
		if (!inNonfeed() && other->block->is_feedthroughable) return 1;
	}
	return (block == other->block);
}

bool Cell::capacityEnough(int net_num) {
	if (!block) return 1;
	return block->through_block_net_num >= net_num;
}
bool Cell::operator <=(const Cell &other) const { return this->x <= other.x && this->y <= other.y; }

bool Cell::operator <(const Cell &other) const { return node < other.node; }

Cell::Cell(const Cell& other):
	x(other.x),
	y(other.y),
	xIndex(other.xIndex),
	yIndex(other.yIndex),
	block(other.block),
	node(other.node),
	BPR(other.BPR),
	TBENN(other.TBENN),
	someNetsMTs(other.someNetsMTs)
{}

Cell& Cell::operator =(const Cell& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        xIndex = other.xIndex;
        yIndex = other.yIndex;
        block = other.block;
        node = other.node;
        BPR = other.BPR;
        TBENN = other.TBENN;
        someNetsMTs = other.someNetsMTs;
    }
    return *this;
}

std::ostream& operator <<(std::ostream& os, const Cell& Cell) {
	os << "(" << Cell.node.x << "," << Cell.node.y << ")";
	return os;
}

void Cell_Manager::setXYvalue(set<int> X_value, set<int> Y_value) {
	x_value = X_value;
	y_value = Y_value;
	return;
}

void Cell::checkInsideBlock(vector<shared_ptr<Block>> blocks) {
	for (auto const &o: blocks) {
		if (o->enclose(node)) { block = o; return;}
	}
	block = nullptr;
	return;
}

bool Cell::EdgeBelongs2Cell(const Edge &e) {
	bool edgeIsVertical = e.isVertical()
		&& (e.fixed() == x.min || e.fixed() == x.max)
		&& e.ranged().max >= y.max && e.ranged().min <= y.min;

	bool edgeIsHorizontal = !e.isVertical()
		&& (e.fixed() == y.min || e.fixed() == y.max)
		&& e.ranged().max >= x.max && e.ranged().min <= x.min;

	return (edgeIsVertical || edgeIsHorizontal) && (e.block == block);
}

void Cell_Manager::createCells(
	const vector<shared_ptr<Block>> &allBlocks,
	const vector<Edge> &allBPRs,
	const vector<shared_ptr<EdgeNetNum>> &allTBENNs,
	int cellWidth,
	Point border
) {
	for (int i = 0, x_count = 0; i < border.x; i += cellWidth, ++x_count) {
		for (int j = 0, y_count = 0; j < border.y; j += cellWidth, ++y_count) {
			Cell tempCell(Pair(i, i + cellWidth), Pair(j, j + cellWidth));
			tempCell.checkInsideBlock(allBlocks);

			for (auto const &e: allBPRs) {
				if (e.block == tempCell.block && tempCell.EdgeBelongs2Cell(e)) tempCell.BPR = e;
			}
			for (auto const &enn: allTBENNs) {
				const Edge e = enn->edge;
				if (e.block == tempCell.block && tempCell.EdgeBelongs2Cell(e)) tempCell.TBENN = enn;
			}
			tempCell.xIndex = x_count;
			tempCell.yIndex = y_count;
			cells[x_count][y_count] = std::make_shared<Cell>(tempCell);
		}
	}
}

shared_ptr<Cell> Cell_Manager::cellEnclose(const Point &p) {
	int x_index = -1, y_index = -1;

	for (auto it = x_value.begin(); it != x_value.end(); ++it, ++x_index) {
		if (*it >= p.x) break; // probably
	}

	for (auto it = y_value.begin(); it != y_value.end(); ++it, ++y_index) {
		if (*it >= p.y) break; // probably
	}

	return cells[x_index][y_index];
}

set<shared_ptr<Cell>> Cell_Manager::cellsOnVertex(const Point &p) {
	set<shared_ptr<Cell>> cellsOnPoint;
	int x_index = 0, y_index = 0;

	for (auto it = x_value.begin(); it != x_value.end(); ++it, ++x_index) { if (*it == p.x) break; }
	for (auto it = y_value.begin(); it != y_value.end(); ++it, ++y_index) { if (*it == p.y) break; }

	cellsOnPoint.insert(cells[x_index][y_index]);
	cellsOnPoint.insert(cells[x_index][y_index - 1]);
	cellsOnPoint.insert(cells[x_index - 1][y_index]);
	cellsOnPoint.insert(cells[x_index - 1][y_index - 1]);

	return cellsOnPoint;
}

set<shared_ptr<Cell>> Cell_Manager::getNeighbor(const shared_ptr<Cell> &theCell) {
	set<shared_ptr<Cell>> neighbors;
	neighbors.insert(cells[theCell->xIndex + 1][theCell->yIndex]);
	neighbors.insert(cells[theCell->xIndex - 1][theCell->yIndex]);
	neighbors.insert(cells[theCell->xIndex][theCell->yIndex + 1]);
	neighbors.insert(cells[theCell->xIndex][theCell->yIndex - 1]);
	return neighbors;
}

/*-------------------------------------*/