#include "Algorithm.h"
#include <float.h>
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
	int findNet = 1539; // 1014 1016
	int count = 0;
	int countNor = 0; // 640
	int countMTs = 0; // 1453
	int countRXs = 0; // 363, max:1539
	// 987???
	int totalAmount = net.totalNets.size();
	set<int> forbiddens;

	outputToCSV("zzb.csv", "zzm.csv", "zzn.csv", chip, net, findNet);
	outputCell("zzp.csv", chip.allCells);

	// algorithm.RXsPath = {
	// 	{Point(3560000,2280000),Point(3560000,360000),Point(4840000,360000)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(215168,3283000),Point(215168,3336000),Point(190000,3336000)},
	// 	{Point(3560000,2280000),Point(3560000,161980),Point(6360000,161980),Point(6360000,40000),Point(6520000,40000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9639000,164080),Point(9639000,1240000)},   
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9490000,164080),Point(9490000,2200000)},   
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2600000),Point(8840000,2600000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8944000,2862000),Point(8944000,3226998)},     
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2679998),Point(7880000,2679998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8068972,2862000),Point(8068972,2920000),Point(8040000,2920000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8840000,2862000),Point(8840000,3880000),Point(8760000,3880000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9569000,164080),Point(9569000,2865800),Point(9678984,2865800),Point(9678984,5254000),Point(9639000,5254000),Point(9639000,5320000)},       
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,3800000),Point(7640000,3800000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7965000,2862000),Point(7965000,5480000),Point(7880000,5480000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4040000),Point(6840000,4040000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,2950000),Point(6808000,2950000),Point(6808000,2912000),Point(6760000,2912000)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(705000,6090000),Point(705000,6198998),Point(760000,6198998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8680000,2862000),Point(8680000,6171999),Point(8440000,6171999),Point(8440000,6198998)},       
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4600000),Point(6680000,4600000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,2950000),Point(6808000,2950000),Point(6808000,2280000),Point(6760000,2280000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4520000),Point(6120000,4520000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,3079998),Point(4680000,3079998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(1418000,6090000),Point(1418000,6198998),Point(2040000,6198998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(2890000,6090000),Point(2890000,6119998),Point(2920000,6119998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(4770000,6090000),Point(4770000,6120000),Point(4840000,6120000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4799120),Point(5041480,4799120),Point(5041480,5640000),Point(5000000,5640000)},
	// 	{Point(3560000,2280000),Point(3560000,360000),Point(4840000,360000)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(215168,3283000),Point(215168,3336000),Point(190000,3336000)},
	// 	{Point(3560000,2280000),Point(3560000,161980),Point(6360000,161980),Point(6360000,40000),Point(6520000,40000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9639000,164080),Point(9639000,1240000)},   
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9490000,164080),Point(9490000,2200000)},   
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2600000),Point(8840000,2600000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8944000,2862000),Point(8944000,3226998)},     
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2679998),Point(7880000,2679998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8068972,2862000),Point(8068972,2920000),Point(8040000,2920000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8840000,2862000),Point(8840000,3880000),Point(8760000,3880000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9569000,164080),Point(9569000,2865800),Point(9678984,2865800),Point(9678984,5254000),Point(9639000,5254000),Point(9639000,5320000)},       
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,3800000),Point(7640000,3800000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7965000,2862000),Point(7965000,5480000),Point(7880000,5480000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4040000),Point(6840000,4040000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,2950000),Point(6808000,2950000),Point(6808000,2912000),Point(6760000,2912000)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(705000,6090000),Point(705000,6198998),Point(760000,6198998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8680000,2862000),Point(8680000,6171999),Point(8440000,6171999),Point(8440000,6198998)},       
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4600000),Point(6680000,4600000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,2950000),Point(6808000,2950000),Point(6808000,2280000),Point(6760000,2280000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4520000),Point(6120000,4520000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,3079998),Point(4680000,3079998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(1418000,6090000),Point(1418000,6198998),Point(2040000,6198998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(2890000,6090000),Point(2890000,6119998),Point(2920000,6119998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(4770000,6090000),Point(4770000,6120000),Point(4840000,6120000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4799120),Point(5041480,4799120),Point(5041480,5640000),Point(5000000,5640000)}
	// };
	
	// algorithm.RXsPath = {
	// 	{Point(3560000,2280000),Point(3560000,360000),Point(4840000,360000)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(215168,3283000),Point(215168,3336000),Point(190000,3336000)},
	// 	{Point(3560000,2280000),Point(3560000,161980),Point(6360000,161980),Point(6360000,40000),Point(6520000,40000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9639000,164080),Point(9639000,1240000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9490000,164080),Point(9490000,2200000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2600000),Point(8840000,2600000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8944000,2862000),Point(8944000,3226998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2679998),Point(7880000,2679998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8068972,2862000),Point(8068972,2920000),Point(8040000,2920000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8840000,2862000),Point(8840000,3880000),Point(8760000,3880000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9569000,164080),Point(9569000,2865800),Point(9678984,2865800),Point(9678984,5254000),Point(9639000,5254000),Point(9639000,5320000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,3800000),Point(7640000,3800000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7965000,2862000),Point(7965000,5480000),Point(7880000,5480000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4040000),Point(6840000,4040000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,2950000),Point(6808000,2950000),Point(6808000,2912000),Point(6760000,2912000)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(705000,6090000),Point(705000,6198998),Point(760000,6198998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(8680000,2862000),Point(8680000,6171999),Point(8440000,6171999),Point(8440000,6198998)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4600000),Point(6680000,4600000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,2950000),Point(6808000,2950000),Point(6808000,2280000),Point(6760000,2280000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4520000),Point(6120000,4520000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,3079998),Point(4680000,3079998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(1418000,6090000),Point(1418000,6198998),Point(2040000,6198998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(2890000,6090000),Point(2890000,6119998),Point(2920000,6119998)},
	// 	{Point(3560000,2280000),Point(3560000,2886420),Point(675000,2886420),Point(675000,3283000),Point(107000,3283000),Point(107000,5313000),Point(34000,5313000),Point(34000,6090000),Point(4770000,6090000),Point(4770000,6120000),Point(4840000,6120000)},
	// 	{Point(3560000,2280000),Point(3560000,164080),Point(9470000,164080),Point(9470000,2862000),Point(7760000,2862000),Point(7760000,4799120),Point(5041480,4799120),Point(5041480,5640000),Point(5000000,5640000)}
	// };

	// algorithm.addNodesToRXsPath();
	// algorithm.simplifiedSpanningTree(algorithm.RXsPath);
	// outputNet(algorithm.pathSegments);

	// return 0;

	for (Net const &n : net.totalNets) {
		if (n.ID != findNet) continue;
		if (n.orderedMTs.size()) ++countMTs;
		else if (n.RXs.size() > 1) ++countRXs;
		else ++countNor;

		// if (n.bBoxArea()) continue;
		// if (n.RXs.size() == 1) continue;

		++count;
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

	cout << "normal: " << countNor;
	cout << "\nMTs: " << countMTs;
	cout << "\nRXs: " << countRXs;
	cout << "\nwantFind: " << totalAmount;

	int pathFoundNum = count - forbiddens.size();
	cout << "\nPath found: " << pathFoundNum << " / " << count << "\n";
	cout << "Found rate: " << std::fixed << std::setprecision(2);
	cout << (float)pathFoundNum * 100 / count << "%";
	
	cout << "\ndone";
	return 0;

	/*-------------------*/
}