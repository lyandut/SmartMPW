//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#pragma once
#ifndef SMARTMPW_RANDOMCASE_HPP
#define SMARTMPW_RANDOMCASE_HPP

#include <unordered_map>
#include "Instance.hpp"

vector<point_t> random_shape(Shape shape, uniform_int_distribution<coord_t>& dist, default_random_engine& gen) {
	vector<point_t> points;
	switch (shape) {
	case Shape::R: {
		coord_t w = dist(gen), h = dist(gen);
		points.reserve(4);
		points.emplace_back(0, 0);
		points.emplace_back(w, 0);
		points.emplace_back(w, h);
		points.emplace_back(0, h);
		break;
	}
	case Shape::L: {
		coord_t hu = dist(gen), hm = dist(gen), vm = dist(gen), vr = dist(gen);
		coord_t hd = hu + hm, vl = vm + vr;
		points.reserve(6);
		points.emplace_back(0, 0);
		points.emplace_back(hd, 0);
		points.emplace_back(hd, vr);
		points.emplace_back(hu, vr);
		points.emplace_back(hu, vl);
		points.emplace_back(0, vl);
		break;
	}
	case Shape::T: {
		coord_t hu = dist(gen), hl = dist(gen), hr = dist(gen);
		coord_t hd = hu + hl + hr;
		coord_t vlu = dist(gen), vld = dist(gen), vru = dist(gen);
		if (vlu + vld <= vru) { swap(vlu, vru); }
		coord_t vrd = vlu + vld - vru;
		points.reserve(8);
		points.emplace_back(0, 0);
		points.emplace_back(hd, 0);
		points.emplace_back(hd, vrd);
		points.emplace_back(hl + hu, vrd);
		points.emplace_back(hl + hu, vld + vlu);
		points.emplace_back(hl, vld + vlu);
		points.emplace_back(hl, vld);
		points.emplace_back(0, vld);
		break;
	}
	case Shape::C: {
		coord_t hu = dist(gen), hl = dist(gen), hr = dist(gen);
		coord_t hd = hu + hl + hr;
		coord_t vlu = dist(gen), vld = dist(gen), vru = dist(gen);
		if (vld < vlu) { swap(vld, vlu); }
		if (vld == vlu) { vld += dist(gen); }
		coord_t vrd = vld - vlu + vru;
		points.reserve(8);
		points.emplace_back(0, 0);
		points.emplace_back(hd, 0);
		points.emplace_back(hd, vrd);
		points.emplace_back(hl + hu, vrd);
		points.emplace_back(hl + hu, vrd - vru);
		points.emplace_back(hl, vld - vlu);
		points.emplace_back(hl, vld);
		points.emplace_back(0, vld);
		break;
	}
	default:
		assert(false);
		break;
	}

	return points;
}

/*
	shape: L, T, C
	polygon_num: 200, 400, 600, 800, 1000
	shape_ratio: 0, 20, 40, 60, 80
*/
void random_case(Shape shape, int polygon_num, int shape_ratio, const string& ins_str = "polygon_area_etc_input_5") {
	Environment env(ins_str);
	Instance ins(env);
	coord_t lb_length = (*min_element(ins.get_polygon_ptrs().begin(), ins.get_polygon_ptrs().end(),
		[](const polygon_ptr& lhs, const polygon_ptr& rhs) { return lhs->min_length < rhs->min_length; }))->min_length;
	coord_t ub_length = (*max_element(ins.get_polygon_ptrs().begin(), ins.get_polygon_ptrs().end(),
		[](const polygon_ptr& lhs, const polygon_ptr& rhs) { return lhs->max_length < rhs->max_length; }))->max_length;

	default_random_engine gen(random_device{}());
	uniform_int_distribution<> len_dist(lb_length, ub_length);
	discrete_distribution<> shape_dist({ 4, 2, 2, 1 });  // 控制多边形的数量分布

	int shape_num = round(polygon_num * shape_ratio * 0.01);
	int other_shape_num = polygon_num - shape_num;
	vector<vector<point_t>> polygons; polygons.reserve(polygon_num);

	while (shape_num--) {
		polygons.emplace_back(random_shape(shape, len_dist, gen));
	}

	while (other_shape_num--) {
		Shape s = Shape(shape_dist(gen));
		while (s == shape) { s = Shape(shape_dist(gen)); }
		polygons.emplace_back(random_shape(s, len_dist, gen));
	}

	unordered_map<Shape, string> mp{ {Shape::L, "L"}, {Shape::T, "T"}, {Shape::C, "U"} };
	ofstream case_file("Instance/" + mp[shape] + "n" + to_string(polygon_num) + "r" + to_string(shape_ratio) + ".txt");
	for (auto& polygon : polygons) {
		case_file << "Polygon:" << endl;
		for (auto& point : polygon) { case_file << '(' << point.x << ',' << point.y << ')'; }
		case_file << endl;
	}

}


#endif // SMARTMPW_RANDOMCASE_HPP
