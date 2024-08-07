#include <vector>
#include "AllZone.h"
#include "Probe.h"

using namespace std;

constexpr double DX = 0.001;
constexpr double DY = 0.001;

bool mikami (vector<Probe> sourceProbes, vector<Probe> targetProbes,
			 AllZone const &allZone) {
				
	//step 1: initializaiotn
	vector<Probe> CSP; // stands for current source probes
	vector<Probe> OSP; // stands for old source probes
	vector<Probe> CTP; // stands for current target probes
	vector<Probe> OTP; // stands for old target probes

	// sourceProbes 跟 targetProbes 還沒寫

	vector<Wall> const walls = allZone.Walls.allWalls;

	Probe sourceProbeForBacktrace;
	Probe targetProbeForBacktrace;

	while(1){
		// step 2: check if intersect
		for(Probe const &s : CSP){ 
			for(Probe const &t : CTP){ 
				if (s.coord == t.coord){
					// which means path is found
					sourceProbeForBacktrace = s;
					targetProbeForBacktrace = t;
					break;
				}
			}
			for(Probe const &t : OTP){
				if (s.coord == t.coord){
					// which means path is found
					sourceProbeForBacktrace = s;
					targetProbeForBacktrace = t;
					break;
				}
			}
		}

		// step 3: copy CSP to OSP; copy CTP to OTP
		// current 的點要存回去 old
		OSP.insert(OSP.end(), CSP.begin(), CSP.end());
		OTP.insert(OTP.end(), CTP.begin(), CTP.end());

		// current 的資料應該不能刪掉，因為還要 extend
		// 只是在這之後(、被清除之前)調用 current probes 應該只能 const &

		// 4. 生成與 current probes 垂直的 extendedProbes，並先把他們暫存在一個 vector 裡面
		// 要分成 from source 跟 from target
		vector<Probe> ESP; // stands for extended source probes
		vector<Probe> ETP; // stands for extended target probes
		// 這些生成出來的 probes 的 level 要 +1

		for (Probe const &p : CSP) { // 來自 source
			double dx = p.directionX * DX;
			double dy = !(p.directionX) * DY;
			double X = dx, Y = dy;
			int levelCSP = p.level;
			
			// 往兩個方向生成新的 probe
			// 正方向
			while (1) {
				Probe positiveProbe = p.extendedProbe(X, Y, levelCSP + 1);
				// 如果這個 probe 會撞到牆，直接結束這個方向的 extend
				if (positiveProbe.hitWall(walls)) {
					break;
				}
				X += dx;
				Y += dy;
				// 如果這個 probe 已經存在 OSP 裡面，跳過這個 probe (但還是會繼續執行 extend)
				if (positiveProbe.alreadyExist(OSP)) continue; // may be time-consuming
				ESP.push_back(positiveProbe);
			}
			// 負方向
			while (1) {
				Probe negativeProbe = p.extendedProbe(-X, -Y, levelCSP + 1);
				// 如果這個 probe 會撞到牆，直接結束這個方向的 extend
				if (negativeProbe.hitWall(walls)) {
					break;
				}
				X += dx;
				Y += dy;
				// 如果這個 probe 已經存在 OSP 裡面，跳過這個 probe (但還是會繼續執行 extend)
				if (negativeProbe.alreadyExist(OSP)) continue; // may be time-consuming
				ESP.push_back(negativeProbe);
			}
		}
		
		for (Probe const &p : CTP) { // 來自 target
			double dx = p.directionX * DX;
			double dy = !(p.directionX) * DY;
			double X = dx, Y = dy;
			int levelCTP = p.level;
			
			// 往兩個方向生成新的 probe
			// 正方向
			while (1) {
				Probe positiveProbe = p.extendedProbe(X, Y, levelCTP + 1);
				// 如果這個 probe 會撞到牆，直接結束這個方向的 extend
				if (positiveProbe.hitWall(walls)) {
					break;
				}
				X += dx;
				Y += dy;
				// 如果這個 probe 已經存在 OSP 裡面，跳過這個 probe (但還是會繼續執行 extend)
				if (positiveProbe.alreadyExist(OTP)) continue; // may be time-consuming
				ETP.push_back(positiveProbe);
			}
			// 負方向
			while (1) {
				Probe negativeProbe = p.extendedProbe(-X, -Y, levelCTP + 1);
				// 如果這個 probe 會撞到牆，直接結束這個方向的 extend
				if (negativeProbe.hitWall(walls)) {
					break;
				}
				X += dx;
				Y += dy;
				// 如果這個 probe 已經存在 OSP 裡面，跳過這個 probe (但還是會繼續執行 extend)
				if (negativeProbe.alreadyExist(OTP)) continue; // may be time-consuming
				ETP.push_back(negativeProbe);
			}
		}

		// 因此我們現在獲得了全部的下一個 level 的 probes (在 extendedProbes 裡)
		CSP.swap(ESP);
		ESP.clear();
		CTP.swap(ETP);
		ETP.clear();
	}

	// step 5. backtrace
	// backtrace 的實作想法:
	// 反正我剛剛問 chatGPT 寫了一個指標的寫法，而且看起來也不難
	// 那就是利用指標的方式去一直找 現在的 Probe 的 parent
	// (也就是上一個 level extend 出這個 probe 的 probe)
	// 就可以 trace 出整條線，應該啦
}

int main(){ // 或是說 mikami 的前置作業，不一定是在 main，可能是獨立的函式
	AllZone allZone(4);

	vector<Probe> fromSource;
	vector<Probe> fromTarget;

	Point TX(10, 10);
	fromSource.push_back(Probe(TX, 0, 0));
	fromSource.push_back(Probe(TX, 1, 0));

	Point RX_0(20, 20);
	Point RX_1(30, 30);
	// 應該要用迴圈實現但隨便啦
	fromTarget.push_back(Probe(RX_0, 0, 0));
	fromTarget.push_back(Probe(RX_0, 1, 0));
	fromTarget.push_back(Probe(RX_1, 0, 0));
	fromTarget.push_back(Probe(RX_1, 1, 0));
	
	mikami(fromSource, fromTarget, allZone);
}