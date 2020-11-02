//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMAERMPW_CHECKER_HPP
#define SMARTMPW_CHECKER_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "Data.hpp"

using namespace std;

class Checker {
public:
	Checker(const string &sol_path) : _sol_path(sol_path) { read_solution(); }

	bool run() {
		if (_in_polygon_num != _out_polygon_num) {
			cout << "Error Polygon Number:" << endl
				<< "In Polygon: " << _in_polygon_num << endl
				<< "Out Polygon: " << _out_polygon_num << endl;
			return false;
		}
		if (_in_total_area != _out_total_area) {
			cout << "Error Polygon Area:" << endl
				<< "In Polygon: " << _in_total_area << endl
				<< "Out Polygon: " << _out_total_area << endl;
			return false;
		}
		for (size_t i = 0; i < _out_polygon_num; ++i) {
			if (*_in_polygon_ptrs[i] != *_out_polygon_ptrs[i]) {
				cout << "Error Polygon Shape:" << endl
					<< "In Polygon: " << _in_polygon_ptrs[i]->str << endl
					<< "Out Polygon: " << _out_polygon_ptrs[i]->str << endl;
				return false;
			}
		}
		coord_t bin_width = 0, bin_height = 0;
		for (auto &op : _out_polygon_ptrs) {
			for (auto& ring : _out_rings) {
				if (vis::bg::overlaps(ring, op->ring)) {
					cout << "Error Polygon Overlap:" << endl
						<< "Polygon 1: " << vis::bg::dsv(op->ring) << endl
						<< "Polygon 2: " << vis::bg::dsv(ring) << endl;
					return false;
				}
				_out_rings.push_back(op->ring);
			}
			bin_width = max(bin_width, max_element(op->ring.begin(), op->ring.end(),
				[](auto &lhs, auto &rhs) { return lhs.x() < rhs.x(); })->x());
			bin_height = max(bin_height, max_element(op->ring.begin(), op->ring.end(),
				[](auto &lhs, auto &rhs) { return lhs.y() < rhs.y(); })->y());
		}
		if (bin_width < 50 || bin_width > 400 || bin_height < 50 || bin_height > 300) {
			cout << "Error Bin Width or Height:" << endl
				<< "Bin Width: " << bin_width << endl
				<< "Bin Height: " << bin_height << endl;
			return false;
		}
		return true;
	}

private:
	void read_solution() {
		ifstream ifs(_sol_path);
		if (!ifs.is_open()) {
			cerr << "Error solution path: can not open " << _sol_path << endl;
			return;
		}

		_in_polygon_num = _out_polygon_num = 0;
		_in_total_area = _out_total_area = 0;
		size_t count = 0;
		string line;
		while (getline(ifs, line)) {
			if (line == "In Polygon:" || line == "Out Polygon:" || line == "Polygon:") { continue; }

			++count;
			stringstream ss(line);
			char l_bracket, comma, r_bracket;
			coord_t x, y;
			vector<point_t> points;
			while (ss >> l_bracket >> x >> comma >> y >> r_bracket) {
				assert(l_bracket == '(' && comma == ',' && r_bracket == ')');
				points.push_back({ x, y });
			}

			vector<segment_t> segments = transform_points_to_segments(points);

			switch (segments.size()) {
			case 4: {
				if (count % 2) {
					_in_polygon_ptrs.emplace_back(make_shared<rect_t>(_in_polygon_num++, line, points, segments));
					_in_total_area += _in_polygon_ptrs.back()->area;
				}
				else {
					_out_polygon_ptrs.emplace_back(make_shared<rect_t>(_out_polygon_num++, line, points, segments));
					_out_total_area += _out_polygon_ptrs.back()->area;
				}
				break;
			}
			case 6: {
				if (count % 2) {
					_in_polygon_ptrs.emplace_back(make_shared<lshape_t>(_in_polygon_num++, line, points, segments));
					_in_total_area += _in_polygon_ptrs.back()->area;
				}
				else {
					_out_polygon_ptrs.emplace_back(make_shared<lshape_t>(_out_polygon_num++, line, points, segments));
					_out_total_area += _out_polygon_ptrs.back()->area;
				}
				break;
			}
			case 8: {
				switch (get_shape_from_segments(segments)) {
				case Shape::T: {
					if (count % 2) {
						_in_polygon_ptrs.emplace_back(make_shared<tshape_t>(_in_polygon_num++, line, points, segments));
						_in_total_area += _in_polygon_ptrs.back()->area;
					}
					else {
						_out_polygon_ptrs.emplace_back(make_shared<tshape_t>(_out_polygon_num++, line, points, segments));
						_out_total_area += _out_polygon_ptrs.back()->area;
					}
					break;
				}
				case Shape::C: {
					if (count % 2) {
						_in_polygon_ptrs.emplace_back(make_shared<concave_t>(_in_polygon_num++, line, points, segments));
						_in_total_area += _in_polygon_ptrs.back()->area;
					}
					else {
						_out_polygon_ptrs.emplace_back(make_shared<concave_t>(_out_polygon_num++, line, points, segments));
						_out_total_area += _out_polygon_ptrs.back()->area;
					}
					break;
				}
				default: { assert(false); break; }
				}
				break;
			}
			default:
				cerr << "Error Shape: has " << segments.size() << " segments." << endl;
				assert(false);
				break;
			}
		}
	}

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
	const string &_sol_path;
	vector<polygon_ptr> _in_polygon_ptrs;
	vector<polygon_ptr> _out_polygon_ptrs;
	size_t _in_polygon_num;
	size_t _out_polygon_num;
	coord_t _in_total_area;
	coord_t _out_total_area;

	vector<vis::bg_ring_t> _out_rings;
};


#endif // !SMARTMPW_CHECKER_HPP