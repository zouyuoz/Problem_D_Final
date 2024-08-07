#include <iostream>
#include <vector>
#include <utility>

using namespace std;

class Pair {
public:
    double min, max;
    Pair *parent;

    Pair() : min(0), max(0), parent(nullptr) {}
    Pair(double Min, double Max, Pair *Parent) : min(Min), max(Max), parent(Parent) {}

    Pair* extendPair(double d) {
        return new Pair(this->min + d, this->max + 1, this);
    }

	bool operator ==(Pair* const &that) {
		return this->min == that->min;
	}

    bool existed(vector<Pair*> pairs) {
        for (Pair* p : pairs) {
            if (this->min == p->min) {
                cout << " rejected.";
                return 1;
            }
        }
        return 0;
    }
};

void test() {
    vector<Pair*> OldPair;
    vector<Pair*> Current;

    Current.push_back(new Pair(0, 0, nullptr));

    double index = 0;
    int iter = 0;
    
	Pair *trace = nullptr;

    while(1) {
        cout << "iter: " << iter++ << endl;

		bool find = 0;
        for(auto const &o : OldPair){
            if (o->min > 10) {
                trace = o;
				find = 1;
                break;
            }
        }
		if (find) break;

        for (auto &c : Current) { OldPair.push_back(c); }

        vector<Pair*> Extended;

        for(auto &c : Current) {
            vector<Pair*> tempExtended;
            tempExtended.push_back(c->extendPair(1));
            tempExtended.push_back(c->extendPair(2));
            tempExtended.push_back(c->extendPair(3));

            for (Pair* te : tempExtended) {
                cout << " + adding " << te->min << ", " << te->max;
                if (te->existed(OldPair) || te->existed(Current) || te->existed(Extended)) {
                    cout << endl;
                    delete te; // 避免記憶體洩漏
                    continue;
                }
                Extended.push_back(te);
                cout << endl;
            }
        }

        //for (auto &c : Current) { delete c; }
		//cout << "哇操牛逼";
        Current.clear();

        Current = std::move(Extended);

        cout << "Current:\n";
        for (auto const &pp : Current) cout << " > " << pp->min << ", " << pp->max << endl;
        cout << "OldPair:\n";
        for (auto const &pp : OldPair) cout << " > " << pp->min << ", " << pp->max << endl;
        if (iter > 10) break;
        cout << "================================\n";
    }

    cout << "*** complete test ***\n";
    while (trace) {
        cout << trace->min;
        if (trace->parent) cout << " > ";
        trace = trace->parent;
    }

    // 清理動態分配的記憶體
	delete trace;
    for (auto &o : OldPair) { delete o; }
    for (auto &c : Current) { delete c; }
}

int main() {
    //test();
	Pair *a = new Pair(1, 0, nullptr);
	Pair *b = new Pair(1, 1, a);
	Pair aPrime = *a;
	
	if (aPrime == b) cout << "yes\n";
    cout << "\ndone";
    return 0;
}
