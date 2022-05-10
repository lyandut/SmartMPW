// SmartMPW.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <cstring>
#include "AdaptSelect.hpp"
#include "RandomCase.hpp"

void run_single_instance(const string& ins_str) {
	Environment env(ins_str);
	AdaptSelect asa(env, cfg);
	asa.run();
	asa.record_sol(env.solution_path());

#ifndef SUBMIT
	asa.draw_ins();
	asa.record_sol(env.solution_path_with_time());
	asa.draw_sol(env.sol_html_path());
	asa.draw_sol(env.sol_html_path_with_time());
	asa.record_log();
#endif // !SUBMIT
}

void run_all_instances() {
	const char* ins_list[]{
		"polygon_area_etc_input_1",
		"polygon_area_etc_input_2",
		"polygon_area_etc_input_3",
		"polygon_area_etc_input_4",
		"polygon_area_etc_input_5",
		"polygon_area_etc_input_6",
		"polygon_area_etc_input_7",
		"polygon_area_etc_input_8",
		"polygon_area_etc_input_9",
		"polygon_area_etc_input_10",
		//"ami33_lt_Ma",
		//"ami49_lt_Xu_1",
		//"ami49_lt_Xu_2"
	};
	for (auto& ins : ins_list) {
		cout << "load instance " << ins << endl;
		run_single_instance(ins);
	}

	unordered_map<Shape, string> mp{ {Shape::L, "L"}, {Shape::T, "T"}, {Shape::C, "U"} };
	for (int s = 1; s <= 3; ++s) {
		for (int n = 200; n <= 1000; n += 200) {
			for (int r = 0; r <= 80; r += 20) {
				run_single_instance(mp[Shape(s)] + "n" + to_string(n) + "r" + to_string(r));
			}
		}
	}
}

void create_random_cases() {
	for (int s = 1; s <= 3; ++s) {
		for (int n = 200; n <= 1000; n += 200) {
			for (int r = 0; r <= 80; r += 20) {
				random_case(Shape(s), n, r);
			}
		}
	}
}

int main(int argc, char* argv[]) {

	if (argc != 2) {
		cerr << "Error parameter. See 'placement.exe /xxx/xxx/input_<id>.txt'." << endl;
	}
	else if (strcmp(argv[1], "--all") == 0) {
		cout << "Run all instances..." << endl;
		run_all_instances();
	}
	else {
		run_single_instance(argv[1]);
	}

	//create_random_cases();

	return 0;
}
