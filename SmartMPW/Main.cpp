// SmartMPW.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "Instance.hpp"
#include "Config.hpp"
#include "MpwBinPack.hpp"

int main() {
	Environment env("polygon_area_etc_input_1");
	Instance ins(env);

	Config cfg;
	cfg.random_seed = random_device{}();
	default_random_engine gen(cfg.random_seed);

	mbp::MpwBinPack mbp_solver(ins, cfg.ub_width, INF, gen);
	vector<polygon_ptr> dst;
	mbp_solver.insert_bottom_left_score(dst);

	system("pause");
}
