#include "Algorithm.h"
#include <memory>

A_star_algorithm::A_star_algorithm (Net &net, Chip &chip) {
	// Initialize: find where source/target belongs
	// if is in channel, set that channel include terminal
	// else extend vertical and horizontal channel
	// do until intersect with an existed channel.

	// For those channels intersect with "channel include terminal" ------------- (1)
	// Store these channels to "open", set these channels' parent "start point"
	// Calculate F value: M_distance(from parent) + M_distance(from target) + turn cost
	// After finish calcute, origin channels store to "close"

	// From "open", find the channel with the smallest F (call it S-channel)
	// Do (1), three situations:
	// Z: if it is target, set find path true, break loop
	// A: if it is not in "open" then, store in "open", set parent, calculate F
	// B: if it is in "open", re-calculate the F value, and update F if smaller
	// After that, store S-channel to "close"
	// end loop

	// back trace the path by parent
	// record path into path
	// return

	Point source = net.TX.coord;
	Point target = net.RXs[0].coord;

	shared_ptr<Channel> source_0 = std::make_shared<Channel>(net.TX, 0, chip.allEdges);
	shared_ptr<Channel> source_1 = std::make_shared<Channel>(net.TX, 1, chip.allEdges);
	shared_ptr<Channel> target_0 = std::make_shared<Channel>(net.RXs[0], 0, chip.allEdges);
	shared_ptr<Channel> target_1 = std::make_shared<Channel>(net.RXs[0], 1, chip.allEdges);

	vector<shared_ptr<Channel>> open = { source_0, source_1 };
	vector<shared_ptr<Channel>> close;
	const vector<shared_ptr<Channel>> allChannels = chip.allChannels;

	for (auto const &c: allChannels) {
		if (source_0->intersected(c)) open.push_back(c);
		if (source_1->intersected(c)) open.push_back(c);
	}

	// while(target)

	while (1) {
		double min_F = INT_MAX;
		for (auto const &open_channel: open) {
			if (calculate_F())
		}
	}
}

double calculate_F (Point &current, Point &source, Point &target, bool turn) {}