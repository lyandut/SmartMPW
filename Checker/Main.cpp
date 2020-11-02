// Checker.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "Checker.hpp"

static const char *sol_list[]{
	"polygon_area_etc_input_1",
	"polygon_area_etc_input_2",
	"polygon_area_etc_input_3",
	"polygon_area_etc_input_4",
	"polygon_area_etc_input_5",
	"polygon_area_etc_input_6",
	"polygon_area_etc_input_7",
	"polygon_area_etc_input_8",
	"polygon_area_etc_input_9",
	"polygon_area_etc_input_10"
};

int main() {

	for (auto &sol : sol_list) {
		cout << "load solution " << sol << endl;
		Checker checker("Solution/" + string(sol) + ".txt");
		checker.run();
	}

	system("pause");

	return 0;
}