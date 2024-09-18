#include "Algorithm.h"
#include <cstddef>
#include <iomanip>
#include <memory>
#include <ostream>
//#include <filesystem>

bool debugInfo = 0;

void outputToCSV(
	const string &block,
	const string &must_through,
	const string &_net,
	Chip &chip, Net_Manager net, int id = 9999
) {
	std::ofstream file_block(block);
    file_block << "group,x,y,is_feed\n";

	for (auto const &b : chip.allBlocks) {
		if (b->name[0] != 'B') continue;
		for (auto const &v : b->vertices) {
			file_block << b->name << "," << v.x << "," << v.y << "," << b->is_feedthroughable << "\n";
		}
	}
	for (auto const &r : chip.allRegions) {
		for (auto const &v : r.vertices) {
			file_block << r.name << "," << v.x << "," << v.y << ",2\n";
		}
	}

    file_block.close();
    cout << "Data written to " << block << "\n";

	std::ofstream file_mt(must_through);
    file_mt << "x1,y1,x2,y2,mt\n";

	for (auto const &b : chip.allBlocks) {
		for (auto const &mt : b->block_port_region) {
			file_mt << mt.first.x << "," << mt.first.y << "," << mt.second.x << "," << mt.second.y << ",0\n";
		}
		for (auto const &hi : b->through_block_edge_net_num) {
			auto mt = hi->edge;
			file_mt << mt.first.x << "," << mt.first.y << "," << mt.second.x << "," << mt.second.y << ",1\n";
		}
	}

	if (id != 9999) {
		for (const auto &n: net.totalNets) {
			if (n.ID != id) continue;
			std::ofstream d("zzi.csv");
			d << "x,y\n";
			d << n.TX.coord.x << "," << n.TX.coord.y << "\n" << n.RXs[0].coord.x << "," << n.RXs[0].coord.y << "\n";
			d.close();
			for (auto const &MT: n.MUST_THROUGHs) {
				auto mt = MT;
				file_mt << mt.first.x << "," << mt.first.y << "," << mt.second.x << "," << mt.second.y << ",3\n";
			}
			for (auto const &MT: n.HMFT_MUST_THROUGHs) {
				auto mt = MT;
				file_mt << mt.first.x << "," << mt.first.y << "," << mt.second.x << "," << mt.second.y << ",3\n";
			}
			if (n.ID == id) break;
		}
	}

	std::ofstream file_net(_net);
	file_net << "x1,y1,x2,y2\n";

	file_mt.close();
    cout << "Data written to " << must_through << "\n";

	file_net.close();
    cout << "Data written to " << _net << "\n";
}

void outputCell(string const &file, Cell_Manager cell) {
    std::ofstream f(file);
	// std::ofstream d("zzi.csv");
    f << "x1,y1,x2,y2,bpr\n";
	// d << "x,y\n";
	for (auto const &a : cell.cells) {
		for (auto const &b_total: a.second) {
			auto b = b_total.second;
			f << b->x.min << "," << b->y.min << "," << b->x.max << "," << b->y.max << ",";
			int color_cell = b->inBlock() + b->inNonfeed() + b->isBPR();
			f << color_cell << "\n";
			// d << b->node.x << "," << b->node.y << "\n";
		}
	}
	f.close();
	// d.close();
    cout << "Data written to " << file << "\n";
}

void outputNet(vector<Point> path) {
	if (!path.size()) return;
	std::ofstream file("zzn.csv", std::ios::app);
	if (path.size() == 1) {
		file  << path[0].x << "," << path[0].y <<  "," << path[0].x << "," << path[0].y << "\n";
		file.close();
		return;
	}
	for (size_t i = 0; i < path.size() - 1; ++i) {
		file << path[i].x << "," << path[i].y << "," << path[i + 1].x << "," << path[i + 1].y << "\n";
	}
	file.close();
	return;
}

int main(int argc, char* argv[]) {
	int testCase = 4;

	Chip chip(testCase);
	Net_Manager net;
	net.ParserAllNets(testCase, chip);
	
	int tracks_um = std::stoi(argv[1]);
	int cellWidth = chip.UNITS_DISTANCE_MICRONS * (net.max_net_num / tracks_um);
	chip.initializeAllCell(net);

	A_star_algorithm algorithm(chip);
	int findNet = 1151; // 1014 1016
	int count = 0;
	int totalAmount = net.totalNets.size();
	set<int> forbiddens;

	outputToCSV("zzb.csv", "zzm.csv", "zzn.csv", chip, net, findNet);
	outputCell("zzp.csv", chip.allCells);

	for (Net const &n : net.totalNets) {
		// if (n.ID != findNet) continue;
		if (n.HMFT_MUST_THROUGHs.size() + n.MUST_THROUGHs.size()) continue;
		if (n.RXs.size() > 1) continue;
		// if (n.ID != findNet && 1) continue;
		++count;
		cout << n.ID << ":\tbBox: " << n.bBoxArea() << "\t(" << count << " / " << totalAmount << ")\t";
		auto PATH = algorithm.getPath(n);
		if (!PATH.size()) forbiddens.insert(n.ID);
		outputNet(PATH);
		// if (n.ID >= findNet) break;
	}

	cout << "Done search.\nForbiddens:\n";
	for (const int &f: forbiddens) cout << f << ", ";
	cout << "\b\b\n";

	int pathFoundNum = count - forbiddens.size();
	cout << "Path found: " << pathFoundNum << " / " << count << "\n";
	cout << "Found rate: " << std::fixed << std::setprecision(2);
	cout << (float)pathFoundNum * 100 / count << "%";
	
	cout << "\ndone";
	return 0;

	/*-------------------*/
}