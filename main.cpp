#include "Algorithm.h"
#include <float.h>
#include <iomanip>
#include <memory>
#include <ostream>
#include <string>
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
			file_mt << mt.p1.x << "," << mt.p1.y << "," << mt.p2.x << "," << mt.p2.y << ",0\n";
		}
		for (auto const &hi : b->through_block_edge_net_num) {
			auto mt = hi->edge;
			file_mt << mt.p1.x << "," << mt.p1.y << "," << mt.p2.x << "," << mt.p2.y << ",1\n";
		}
	}

	if (id != 9999) {
		std::ofstream d("zzi.csv");
		d << "x,y\n";
		for (const auto &n: net.totalNets) {
			if (n.ID != id) continue;
			d << n.TX.coord.x << "," << n.TX.coord.y << "\n";
			for (int i = 0; i < n.RXs.size(); ++i) {
				d << n.RXs[i].coord.x << "," << n.RXs[i].coord.y << "\n";
			}
			for (auto const &MT: n.MUST_THROUGHs) {
				auto mt = MT;
				file_mt << mt.p1.x << "," << mt.p1.y << "," << mt.p2.x << "," << mt.p2.y << ",3\n";
			}
			for (auto const &MT: n.HMFT_MUST_THROUGHs) {
				auto mt = MT;
				file_mt << mt.p1.x << "," << mt.p1.y << "," << mt.p2.x << "," << mt.p2.y << ",3\n";
			}
			if (n.ID == id) break;
		}
		d.close();
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

void outputNet(vector<Simple_Edge> path) {
	if (!path.size()) return;
	std::ofstream file("zzn.csv", std::ios::app);
	for (int i = 0; i < path.size(); ++i) {
		file << path[i].p1.x << "," << path[i].p1.y << "," << path[i].p2.x << "," << path[i].p2.y << "\n";
	}
	file.close();
	return;
}

void outputRPT(Net net, int CASE) {
	std::ofstream file("case0" + std::to_string(CASE) + "_net.rpt", std::ios::app);
	file << "[ID " << net.ID << "]\n";
	for (const auto &segment: net.path) {
		file << "(" << segment.p1.x << "," << segment.p1.y << "),";
		file << "(" << segment.p2.x << "," << segment.p2.y << ")\n";
	}
	file.close();
	return;
}

int main(int argc, char* argv[]) {
	// change your test case here (0~6)
	int testCase = 0;

	Chip chip(testCase);
	Net_Manager net;
	net.ParserAllNets(testCase, chip);
	
	int tracks_um = std::stoi(argv[1]);
	chip.initializeAllCell(net);

	A_star_algorithm algorithm(chip, tracks_um);

	// below are variables to show what type of net are found
	// also if you want to find specific net, you can also change "findNet" here
	int findNet = -1; // 1014 1016
	int count = 0;
	int countNor = 0; // 640, 49 quite long
	int countMTs = 0; // 269, 1453
	int countRXs = 0; // 363, max:1539
	int notFound = 0;
	int totalAmount = net.totalNets.size();

	// these two functions will output every chip informations
	// including blocks, nets, edges, cells to .csv files
	// for visualizing the chip top
	// if you need visualing chip top, just set the boolean to true
	bool visualizeChip = 0;
	if (visualizeChip) {
		outputToCSV("zzb.csv", "zzm.csv", "zzn.csv", chip, net, findNet);
		outputCell("zzp.csv", chip.allCells);
	}

	// refresh rpt file
	std::ofstream rpt("case0" + std::to_string(testCase) + "_net.rpt");

	for (Net &n : net.totalNets) {
		if (findNet != -1 && n.ID < findNet) continue;
		// or you can change any condition of nets
		// to determine what nets to be routed

		if (n.orderedMTs.size()) ++countMTs;
		else if (n.RXs.size() > 1) ++countRXs;
		else ++countNor;

		++count;
		cout << n.ID << ": size: " << n.RXs.size() + n.orderedMTs.size();
		cout << ", bBox: " << n.bBoxArea() << " (" << count << "/" << totalAmount << ")\n";

		auto PATH = algorithm.getPath(n);
		if (!PATH.size()) ++notFound;
		outputRPT(n, testCase);
		if (visualizeChip) outputNet(PATH);

		if (findNet != -1 && n.ID >= findNet) break;
	}

	cout << "Done search.\n";
	cout << "normal: " << countNor;
	cout << "\nMTs: " << countMTs;
	cout << "\nRXs: " << countRXs;
	cout << "\nnot found: " << notFound;

	int pathFoundNum = count - notFound;
	cout << "\nPath found: " << pathFoundNum << " / " << count << "\n";
	cout << "Found rate: " << std::fixed << std::setprecision(2);
	cout << (float)pathFoundNum * 100 / count << "%";
	
	cout << "\ndone";
	return 0;

	/*-------------------*/
}