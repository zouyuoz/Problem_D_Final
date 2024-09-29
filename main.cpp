#include "Algorithm.h"
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

int main(int argc, char* argv[]) {
	int testCase = 0;

	Chip chip(testCase);
	Net_Manager net;
	net.ParserAllNets(testCase, chip);
	
	int tracks_um = std::stoi(argv[1]);
	int cellWidth = chip.UNITS_DISTANCE_MICRONS * (net.max_net_num / tracks_um);
	chip.initializeAllCell(net);

	A_star_algorithm algorithm(chip);
	int findNet = 640; // 1014 1016
	int count = 0; // 640
	int countMTs = 0; // 1453
	int countRXs = 0; // 363
	int totalAmount = net.totalNets.size();
	set<int> forbiddens;

	algorithm.RXsPath = {
		{Point(4,4), Point(4,3), Point(7,3)},
		{Point(4,4), Point(4,3), Point(6,3), Point(6,1)},
		{Point(4,4), Point(4,1), Point(0,1), Point(0,0)},
		{Point(4,4), Point(4,1), Point(2,1), Point(2,3)}
	};

	// algorithm.RXsPath = {
	// 	{Point(10,10), Point(9,9), Point(6,6)},
	// 	{Point(10,10), Point(9,9), Point(5,5), Point(3,3)},
	// 	{Point(10,10), Point(9,9), Point(5,5), Point(2,2)},
	// 	{Point(10,10), Point(9,9), Point(4,4)},
	// 	{Point(10,10), Point(8,8)},
	// 	{Point(10,10), Point(7,7), Point(1,1)},
	// };

	algorithm.addNodesToRXsPath();
	algorithm.simplifiedSpanningTree(algorithm.RXsPath);
	outputNet(algorithm.pathSegments);

	return 0;

	outputToCSV("zzb.csv", "zzm.csv", "zzn.csv", chip, net, findNet);
	outputCell("zzp.csv", chip.allCells);

	for (Net const &n : net.totalNets) {
		if (n.ID != findNet) continue;
		if (n.orderedMTs.size()) ++countMTs;
		else if (n.RXs.size() > 1) ++countRXs;
		else ++count;

		cout << n.ID << ": size: " << n.RXs.size() + n.orderedMTs.size();
		cout << ", bBox: " << n.bBoxArea() << " (" << count << "/" << totalAmount << ")\n";
		// continue;
		auto PATH = algorithm.getPath(n);
		if (!PATH.size()) forbiddens.insert(n.ID);
		outputNet(PATH);
		if (n.ID >= findNet) break;
	}

	cout << "Done search.\nForbiddens:\n";
	for (const int &f: forbiddens) cout << f << ", ";
	cout << "\b\b\n";

	cout << "normal: " << count;
	cout << "\nMTs: " << countMTs;
	cout << "\nRXs: " << countRXs;
	cout << "\ntotal: " << totalAmount;

	int pathFoundNum = count - forbiddens.size();
	cout << "\nPath found: " << pathFoundNum << " / " << count << "\n";
	cout << "Found rate: " << std::fixed << std::setprecision(2);
	cout << (float)pathFoundNum * 100 / count << "%";
	
	cout << "\ndone";
	return 0;

	/*-------------------*/
}