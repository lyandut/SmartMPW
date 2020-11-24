// SmartMPW.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <cstring>
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
		cerr << "Error parameter. See 'placement.exe /xxx/xxx/input_<id>.txt'." << endl;
	}
	else if (strcmp(argv[1], "--all") == 0) {
		cout << "Run all instances..." << endl;
		run_all_instances();
	}
	else {
		run_single_instance(argv[1]);
	}

	return 0;
}
