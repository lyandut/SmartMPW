//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMARTMPW_INSTANCE_HPP
#define SMARTMPW_INSTANCE_HPP

#include <iostream>
#include <list>
#include <string>
#include <fstream>

#include "Data.hpp"
#include "Utils.hpp"

using namespace std;

class Environment {
public:
	Environment(const string &ins_str) : _ins_path(ins_str) { utils::split_filename(_ins_path, _ins_dir, _ins_name, _ins_id); }

#ifndef SUBMIT
public:
	const string& instance_name() const { return _ins_name; }
	string instance_path() const { return instance_dir() + _ins_name + ".txt"; }
	string solution_path() const { return solution_dir() + _ins_name + ".txt"; }
	string solution_path_with_time() const { return solution_dir() + _ins_name + "." + utils::Date::to_long_str() + ".txt"; }
	string ins_html_path() const { return instance_dir() + _ins_name + ".html"; }
	string sol_html_path() const { return solution_dir() + _ins_name + ".html"; }
	string sol_html_path_with_time() const { return solution_dir() + _ins_name + "." + utils::Date::to_long_str() + ".html"; }
	string log_path() const { return solution_dir() + "log.csv"; }
	string characteristic_path() const { return instance_dir() + "characteristic.csv"; }
private:
	static string instance_dir() { return "Instance/"; }
	static string solution_dir() { return "Solution/"; }
#else
public:
	const string& instance_path() const { return _ins_path; }
	string solution_path() const { return solution_dir() + "result" + _ins_id + ".txt"; }
private:
	static string instance_dir() { return "/home/mpw/inputFiles/"; }
	static string solution_dir() { return "/home/eda20315/project/verify_results/"; }
#endif // !SUBMIT

private:
	string _ins_path;
	string _ins_dir;
	string _ins_name;
	string _ins_id;
};

class Instance {
public:
	Instance(const Environment &env) : _env(env) { read_instance(); }

	coord_t get_total_area() const { return _total_area; }

	int get_polygon_num() const { return _polygon_num; }

	int get_polygon_num(Shape shape) const {
		switch (shape) {
		case Shape::R: return _rect_num;
		case Shape::L: return _lshape_num;
		case Shape::T: return _tshape_num;
		case Shape::C: return _concave_num;
		default: return _polygon_num;
		}
		return _polygon_num;
	}

	const vector<polygon_ptr>& get_polygon_ptrs()  const { return _polygon_ptrs; }

	// [todo] 预处理，合并部分L/T成矩形
	void pre_combine() {}

private:
	void read_instance() {
		ifstream ifs(_env.instance_path());
		if (!ifs.is_open()) {
			cerr << "Error instance path: can not open " << _env.instance_path() << endl;
			return;
		}

		_polygon_num = _rect_num = _lshape_num = _tshape_num = _concave_num = 0;
		_total_area = 0;
		string line;
		while (getline(ifs, line)) {
			if (line.front() != '(') { continue; }

			stringstream ss(line);
			char l_bracket, comma, r_bracket;
			coord_t x, y;
			vector<point_t> in_points;
			while (ss >> l_bracket >> x >> comma >> y >> r_bracket) {
				assert(l_bracket == '(' && comma == ',' && r_bracket == ')');
				in_points.emplace_back(x, y);
			}

			vector<segment_t> in_segments = transform_points_to_segments(in_points);

			switch (in_segments.size()) {
			case 4: {
				//_polygon_ptrs.emplace_back(make_shared<rect_t>(_polygon_num++, in_points, in_segments));
				//_rects.push_back(*dynamic_pointer_cast<rect_t>(_polygon_ptrs.back()));
				_rects.emplace_back(_polygon_num++, in_points, in_segments);
				_polygon_ptrs.emplace_back(make_shared<rect_t>(_rects.back()));
				_total_area += _rects.back().area;
				_rect_num++;
				break;
			}
			case 6: {
				//_polygon_ptrs.emplace_back(make_shared<lshape_t>(_polygon_num++, in_points, in_segments));
				//_lshapes.push_back(*dynamic_pointer_cast<lshape_t>(_polygon_ptrs.back()));
				_lshapes.emplace_back(_polygon_num++, in_points, in_segments);
				_polygon_ptrs.emplace_back(make_shared<lshape_t>(_lshapes.back()));
				_total_area += _lshapes.back().area;
				_lshape_num++;
				break;
			}
			case 8: {
				switch (get_shape_from_segments(in_segments)) {
				case Shape::T: {
					//_polygon_ptrs.emplace_back(make_shared<tshape_t>(_polygon_num++, in_points, in_segments));
					//_tshapes.push_back(*dynamic_pointer_cast<tshape_t>(_polygon_ptrs.back()));
					_tshapes.emplace_back(_polygon_num++, in_points, in_segments);
					_polygon_ptrs.emplace_back(make_shared<tshape_t>(_tshapes.back()));
					_total_area += _tshapes.back().area;
					_tshape_num++;
					break;
				}
				case Shape::C: {
					//_polygon_ptrs.emplace_back(make_shared<concave_t>(_polygon_num++, in_points, in_segments));
					//_concaves.push_back(*dynamic_pointer_cast<concave_t>(_polygon_ptrs.back()));
					_concaves.emplace_back(_polygon_num++, in_points, in_segments);
					_polygon_ptrs.emplace_back(make_shared<concave_t>(_concaves.back()));
					_total_area += _concaves.back().area;
					_concave_num++;
					break;
				}
				default: { assert(false); break; }
				}
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
		for (size_t i = 0; i < points.size(); ++i)
			segments.emplace_back(points[i], points[(i + 1) % points.size()]);
		return segments;
	}

	Shape get_shape_from_segments(const vector<segment_t> &segments) {
		int turn_count = 0;
		Direction curr_turn = segments.back().next_turn(segments.front());
		for (size_t i = 0; i < segments.size(); ++i) {
			Direction next_turn = segments[i].next_turn(segments[(i + 1) % segments.size()]);
			if (curr_turn != next_turn) {
				++turn_count;
				curr_turn = next_turn;
			}
		}
		if (turn_count == 4) return Shape::T;
		else assert(turn_count == 2); return Shape::C;
	}

private:
	const Environment &_env;

	vector<polygon_ptr> _polygon_ptrs;

	list<rect_t> _rects;
	list<lshape_t> _lshapes;
	list<tshape_t> _tshapes;
	list<concave_t> _concaves;

	coord_t _total_area;
	int _polygon_num;
	int _rect_num;
	int _lshape_num;
	int _tshape_num;
	int _concave_num;
};

#endif // SMARTMPW_INSTANCE_HPP
