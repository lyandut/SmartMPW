//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#pragma once

const int INF = 0x3f3f3f3f;

using coord_t = int;

struct Config {
	unsigned int random_seed;
	int ub_time; // ASA超时时间
	int ub_iter; // RLS最大迭代次数

	coord_t lb_width = 50, ub_width = 400;
	coord_t lb_height = 50, ub_height = 300;
};

static const char *ins_list[]{
	"polygon_area_etc_input_1",
	"polygon_area_etc_input_2",
	"polygon_area_etc_input_3",
	"polygon_area_etc_input_4",
	"polygon_area_etc_input_5",
	"polygon_area_etc_input_6",
	"polygon_area_etc_input_7",
	"polygon_area_etc_input_8",
	"polygon_area_etc_input_9"
};