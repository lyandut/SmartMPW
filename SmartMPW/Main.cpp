﻿// SmartMPW.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "AdaptSelect.hpp"

void run_single_instance() {
	Config cfg;
	cfg.random_seed = random_device{}();
	default_random_engine gen(cfg.random_seed);

	Environment env("polygon_area_etc_input_10");
	Instance ins(env);

	mbp::MpwBinPack mbp_solver(ins.get_polygon_ptrs(), 50, INF, gen);
	mbp_solver.random_local_search(1);
}

void run_all_instance() {
	Config cfg;
	cfg.random_seed = random_device{}();
	cfg.ub_iter = 9999;
	cfg.ub_time = 60 * 15;

	for (auto &ins : ins_list) {
		Environment env(ins);
		AdaptSelect asa(env, cfg);
		asa.run();
		asa.record_sol(env.solution_path());
		asa.record_sol(env.solution_path_with_time());
		asa.draw_html(env.html_path());
		asa.draw_html(env.html_path_with_time());
		asa.record_log(env.log_path());
	}
}

int main() {
	
	run_all_instance();

	system("pause");

	return 0;
}
