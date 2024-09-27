void kindaAlgorithm(const vector<vector<int>> &all, int index = -1) {
	if (all[0].size() - 1 == index) return;
	++index;
	// cout << "iter: " << index << "\n";
    set<int> nextsFirst;
    for (const auto &single : all) {
        nextsFirst.insert(single[index]);
    }

	// cout << "> in index " << index << ", " << nextsFirst.size() << " nums";
	// for (auto const p: nextsFirst) cout << ", " << p;
	// cout << "\n";

    for (const auto &p : nextsFirst) {
        cout << p << "-> ";
        vector<vector<int>> nexts;
        for (const auto &single : all) {
            if (index < single.size() && single[index] == p) {
                nexts.push_back(single);
            }
        }
		// cout << nexts.size() << " vectors' " << index << "th num is " << p << "\n";
        // elements in nexts have same value of vec[index] (and previous ones)
        kindaAlgorithm(nexts, index);
		cout << "\n";
    }
	// cout << "end of iter " << index << "\n";
	return;
}