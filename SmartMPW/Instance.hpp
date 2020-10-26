#pragma once

#include <iostream>
#include <list>
#include <string>
#include <fstream>

#include "Data.hpp"
#include "Utils.hpp"

using namespace std;

class Environment {
public:
	Environment(const string &ins_str) { utils::split_filename(ins_str, _submit_dir, _ins_name); }

	const string& instance_name() const { return _ins_name; }
	const string& submit_dir() const { return _submit_dir; }
	string instance_path() const {
		if (submit_dir().empty()) { return instance_dir() + _ins_name + ".txt"; }
		return submit_dir() + _ins_name;
	}
	string solution_path() const {
		if (submit_dir().empty()) { return solution_dir() + _ins_name + ".txt"; }
		return submit_dir() + "result.txt";
	}
	string solution_path_with_time() const { return solution_dir() + _ins_name + "." + utils::Date::to_long_str() + ".txt"; }
	string html_path() const { return solution_dir() + _ins_name + ".html"; }
	string html_path_with_time() const { return solution_dir() + _ins_name + "." + utils::Date::to_long_str() + ".html"; }
	string log_path() const { return solution_dir() + "log.csv"; }

private:
	static string instance_dir() { return "Instance/"; }
	static string solution_dir() { return "Solution/"; }

private:
	string _ins_name;
	string _submit_dir;
};

class Instance {
public:
	Instance(const Environment &env) : _env(env) { read_instance(); }

	coord_t get_total_area() const { return _total_area; }

	size_t get_polygon_num() const { return _polygon_num; }

	const auto& get_polygon_ptrs()  const { return _polygon_ptrs; }

	// [todo] 预处理，合并部分L/T成矩形
	void pre_combine() {}

private:
	void read_instance() {
		ifstream ifs(_env.instance_path());
		if (!ifs.is_open()) {
			cerr << "Error instance path: " << _env.instance_path() << endl;
			return;
		}

		_polygon_num = 0;
		_total_area = 0;
		string line;
		while (getline(ifs, line)) {
			if (line == "Polygon:") { continue; }

			stringstream ss(line);
			char l_bracket, comma, r_bracket;
			coord_t x, y;
			vector<point_t> in_points;
			while (ss >> l_bracket >> x >> comma >> y >> r_bracket) {
				assert(l_bracket == '(' && comma == ',' && r_bracket == ')');
				in_points.push_back({ x, y });
			}

			vector<segment_t> in_segments = transform_points_to_segments(in_points);

			switch (in_segments.size()) {
			case 4: {
				//_polygon_ptrs.emplace_back(make_shared<rect_t>(_polygon_num++, in_points, in_segments));
				//_rects.push_back(*dynamic_pointer_cast<rect_t>(_polygon_ptrs.back()));
				_rects.emplace_back(_polygon_num++, in_points, in_segments);
				_polygon_ptrs.emplace_back(make_shared<rect_t>(_rects.back()));
				_total_area += _rects.back().area;
				break;
			}
			case 6: {
				//_polygon_ptrs.emplace_back(make_shared<lshape_t>(_polygon_num++, in_points, in_segments));
				//_lshapes.push_back(*dynamic_pointer_cast<lshape_t>(_polygon_ptrs.back()));
				_lshapes.emplace_back(_polygon_num++, in_points, in_segments);
				_polygon_ptrs.emplace_back(make_shared<lshape_t>(_lshapes.back()));
				_total_area += _lshapes.back().area;
				break;
			}
			case 8: {
				//_polygon_ptrs.emplace_back(make_shared<tshape_t>(_polygon_num++, in_points, in_segments));
				//_tshapes.push_back(*dynamic_pointer_cast<tshape_t>(_polygon_ptrs.back()));
				_tshapes.emplace_back(_polygon_num++, in_points, in_segments);
				_polygon_ptrs.emplace_back(make_shared<tshape_t>(_tshapes.back()));
				_total_area += _tshapes.back().area;
				break;
			}
			default:
				cerr << "Error Shape: has " << in_segments.size() << " segments." << endl;
				assert(false);
				break;
			}
		}
	}

	// [todo] 修正重复和共线的坐标点
	vector<segment_t> transform_points_to_segments(const vector<point_t> &points) {
		vector<segment_t> segments;
		segments.reserve(8);
		for (size_t i = 0; i < points.size(); ++i) {
			if (i == points.size() - 1)
				segments.emplace_back(points.back(), points.front());
			else
				segments.emplace_back(points[i], points[i + 1]);
		}
		return segments;
	}

private:
	const Environment &_env;

	vector<polygon_ptr> _polygon_ptrs;

	list<rect_t> _rects;
	list<lshape_t> _lshapes;
	list<tshape_t> _tshapes;

	size_t _polygon_num;
	coord_t _total_area;
};

