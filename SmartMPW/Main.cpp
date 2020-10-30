// SmartMPW.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "AdaptSelect.hpp"

void run_single_instance(const string &ins_str) {
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
	for (auto &ins : ins_list) {
		cout << "load instance " << ins << endl;
		run_single_instance(ins);
	}
}

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cerr << "Error parameter. See 'placement.exe /xxx/xxx/input.txt'." << endl;
		return 0;
	}

	if (strcmp(argv[1], "--all") == 0) {
		cout << "Run all instances..." << endl;
		run_all_instances();
		return 0;
	}

	run_single_instance(argv[1]);

	return 0;
}
