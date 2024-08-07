#include "Net.h"
#include "Band.h"
#include <cstddef>
#include <iomanip>
//#include <filesystem>

bool debugInfo = 0;

bool checkIfIntersect(shared_ptr<Band> &source, shared_ptr<Band> &target, 
                      vector<shared_ptr<Band>> &vec_source, 
                      vector<shared_ptr<Band>> &vec_target) {
    for (const auto &s : vec_source) {
        for (const auto &t : vec_target) {
            if (s->intersected(t)) {
                // which means path is found
                source = s;
                target = t;
                return true;
            }
        }
    }
    return false;
}

void printBands(const vector<shared_ptr<Band>>& bands) {
    for (const auto& band : bands) { 
        cout << " > " << *band << "\n"; 
    }
}



void outputToCSV(const string &block, const string &must_through, const string &_net, 
				 Chip &chip, Net &net, int id = 9999) {
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

	std::ofstream file_net(_net);
	file_net << "ID,x,y\n";

	file_mt.close();
    cout << "Data written to " << must_through << "\n";

	file_net.close();
    cout << "Data written to " << _net << "\n";
}

void outputToNet(Net &n, vector<shared_ptr<Band>> &record) {
    std::ofstream file;
    file.open("zzn.csv", std::ios::out | std::ios::app);

	file << n.ID << "," << n.TX.coord.x << "," << n.TX.coord.y << "\n";
	for (size_t i = 0; i < record.size(); ++i) {
		size_t j = (i + 1) % record.size();
		if (!j) break;
		double X = double(std::max(record[i]->x.min, record[j]->x.min) + std::min(record[i]->x.max, record[j]->x.max)) / 2;
		double Y = double(std::max(record[i]->y.min, record[j]->y.min) + std::min(record[i]->y.max, record[j]->y.max)) / 2;
		file << n.ID << "," << X << "," << Y << "\n";
	}
	file << n.ID << "," << n.RXs.begin()->coord.x << "," << n.RXs[0].coord.y << "\n";

	record.clear();
	file.close();
	return;
}

void outputToPath(string const &file, vector<shared_ptr<Channel>> channel) {
    std::ofstream f(file);
	std::ofstream d("zzi.csv");
    f << "x1,y1,x2,y2,bpr\n";
	d << "x,y\n";
	for (auto const &b : channel) {
		f << b->x.min << "," << b->y.min << "," << b->x.max << "," << b->y.max << ",";
		int color_channel = b->inBlock() + b->inNonfeed();
		f << color_channel << "\n";
		d << b->node.x << "," << b->node.y << "\n";
	}
	f.close();
	d.close();
    cout << "Data written to " << file << "\n";
}

int main() {
	int testCase = 6;

	// for (int t = 0; t < 7; ++t) {
	// 	int tbenn_non = 0;
	// 	int tbenn_fee = 0;
	// 	Chip chip(t);
	// 	for (auto const &b: chip.allBlocks) {
	// 		if (b->through_block_edge_net_num.size()) {
	// 			b->showBlockInfo();
	// 			b->is_feedthroughable? ++tbenn_fee: ++tbenn_non;
	// 		}
	// 	}
	// 	cout << t << " tbenn_non: " << tbenn_non << ", tbenn_fee: " << tbenn_fee << "\n";
	// }
	// return 0;

	Chip chip(testCase);
	// chip.initializeAllChannel();
	Net net;
	net.ParserAllNets(testCase, chip);
	// for (auto const &n: net.allNets) {
	// 	for (size_t rx1 = 0; rx1 < n.RXs.size(); ++rx1) { for (size_t rx2 = 0; rx2 < n.RXs.size(); ++rx2) {
	// 		if (rx1 == rx2) continue;
	// 		if (n.RXs[rx1].coord == n.RXs[rx2].coord) n.showNetInfo();
	// 	} }
	// }
	// return 0;
	outputToCSV("zzb.csv", "zzm.csv", "zzn.csv", chip, net);
	
	chip.initializeAllGrid();
	//outputToPath("zzp.csv", chip.allChannels);
	
	cout << "\ndone";
	return 0;

	vector<shared_ptr<Band>> record;
	int singleID = 0;

	for (Net &n : net.allNets) {
		if (n.MUST_THROUGHs.size() + n.HMFT_MUST_THROUGHs.size()) continue;
		for (size_t i = 0; i < n.RXs.size(); ++i) {
			Net solo_n(singleID++, n.TX, n.RXs[i]);
			bandSearchAlgorithm(solo_n, chip, record);
			outputToNet(solo_n, record);
		}
	}

	return 0;

	/*---------------------

	// customed test data
	Chip tesCh;
	vector<shared_ptr<Band>> record;
	for (int i = 0; i < 6; i++) { tesCh.allBlocks.push_back(make_unique<Block>("Block" + to_string(i))); }
	tesCh.allBlocks.push_back(make_unique<Block>("BtestS"));
	tesCh.allBlocks.push_back(make_unique<Block>("BtestT"));

	// regular borders
	tesCh.allBlocks[0]->vertices = { Point(11, 0), Point(11, 9), Point(17, 9), Point(17, 7), Point(14, 7), Point(14, 0) };
	tesCh.allBlocks[1]->vertices = { Point(9, 10), Point(9, 12), Point(20, 12), Point(20, 10) };
	tesCh.allBlocks[2]->vertices = { Point(17, 2), Point(17, 4), Point(22, 4), Point(22, 12), Point(25, 12), Point(25, 10), Point(28, 10), Point(28, 8), Point(25, 8), Point(25, 3), Point(22, 3), Point(22, 2) };
	tesCh.allBlocks[3]->vertices = { Point(0, 4), Point(4, 4), Point (4, 11), Point(7, 11), Point(7, 13), Point(23, 13), Point(23, 15), Point(0, 15) };
	tesCh.allBlocks[4]->vertices = { Point(24, 13), Point(24, 15), Point(33, 15), Point(33, 3), Point(30, 3), Point(30, 4), Point(27, 4), Point(27, 6), Point(30, 6), Point(30, 13) };
	tesCh.allBlocks[5]->vertices = { Point(0, 0), Point(0, 15), Point(33, 15), Point(33, 0) };

	// the block contains terminal
	tesCh.allBlocks[6]->vertices = { Point(4.9, 3.9), Point(4.9, 6.1), Point(7.1, 6.1), Point(7.1, 3.9) }; // testS
	tesCh.allBlocks[7]->vertices = { Point(25.5, 2), Point(25.5, 3.3), Point(28.8, 3.3), Point(28.8, 2) }; // testT
	for (auto &b : tesCh.allBlocks) { b->verticesToEdges(); for (auto &e : b->edges) { tesCh.allEdges.push_back(e); } }

	// make it ordered
	std::sort(tesCh.allEdges.begin(), tesCh.allEdges.end(), 
		[](const auto& a, const auto& b) {
			return a.fixed() < b.fixed(); 
		}
	);

	Terminal start("BtestS", Point(6, 5));
  	Terminal end("BtestT", Point(26, 3));
	Net n(start, end);
	n.allNets.push_back(n);

	outputToCSV("zzb.csv", "zzm.csv", "zzn.csv", tesCh, n);
	bandSearchAlgorithm(n, tesCh, record);

	cout << "done" << "\n";
    return 0;
	
	-----------------------
	
	---------------------*/
}