//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMARTMPW_MPWBINPACK_HPP
#define SMARTMPW_MPWBINPACK_HPP

#include <list>
#include <string>
#include <unordered_set>
#include <numeric>
#include <algorithm>

#include "Data.hpp"

namespace mbp {

	using namespace std;

	class MpwBinPack {

		/// 排序规则定义
		struct SortRule {
			vector<size_t> sequence;
			coord_t target_area;

			string tabu_key_str() const {
				string key = to_string(sequence.front());
				for (size_t i = 1; i < sequence.size(); ++i) { key += "," + to_string(sequence[i]); }
				return key;
			}

			size_t tabu_key_hash() const { return hash<string>{}(tabu_key_str()); }
		};

		//using TabuTable = unordered_set<string>;
		//using TabuFunc = string(SortRule::*)()const;
		//TabuFunc tabu_key = &SortRule::tabu_key_str;

		//using TabuTable = unordered_set<size_t>;
		//using TabuFunc = size_t(SortRule::*)()const;
		//TabuFunc tabu_key = &SortRule::tabu_key_hash;

	public:

		MpwBinPack() = delete;

		MpwBinPack(const vector<polygon_ptr> &src, coord_t width, coord_t height, default_random_engine &gen) :
			_src(src), _bin_width(width), _bin_height(height), _obj_area(numeric_limits<coord_t>::max()),
			_gen(gen), _uniform_dist(0, _src.size() - 1) {
			reset();
			init_sort_rules();
		}

		const vector<polygon_ptr> &get_dst() const { return _dst; }

		coord_t get_obj_area() const { return _obj_area; }

		void set_obj_area(coord_t area) { _obj_area = area; }

		void set_bin_height(coord_t height) { _bin_height = height; } // 上界

		coord_t get_skyline_height() const { // 排版后的上边界
			return max_element(_skyline.begin(), _skyline.end(),
				[](const skylinenode_t &lhs, const skylinenode_t &rhs) { return lhs.y < rhs.y; })->y;
		}

		/// 基于bin_width进行RLS
		void random_local_search(int iter) {
			// the first time to call RLS on W_k
			if (iter == 1) {
				for (auto &rule : _sort_rules) {
					_polygons.assign(rule.sequence.begin(), rule.sequence.end());
					vector<polygon_ptr> target_dst;
					bool first_insert = insert_bottom_left_score(target_dst);
					assert(first_insert); // 第一次必能全部放下
					rule.target_area = _bin_width * get_skyline_height();
					if (rule.target_area < _obj_area) {
						_obj_area = rule.target_area;
						_dst = target_dst;
					}
				}
				// 降序排列，越后面的目标函数值越小选中概率越大
				sort(_sort_rules.begin(), _sort_rules.end(), [](const SortRule &lhs, const SortRule &rhs) {
					return lhs.target_area > rhs.target_area; });
			}
			// 迭代优化
			SortRule &picked_rule = _sort_rules[_discrete_dist(_gen)];
			for (int i = 1; i <= iter; ++i) {
				SortRule new_rule = picked_rule;
				if (iter % 4) { swap_sort_rule(new_rule); }
				else { rotate_sort_rule(new_rule); }
				//if (_tabu_table.count((new_rule.*tabu_key)())) { continue; } // 已禁忌
				//_tabu_table.insert((new_rule.*tabu_key)());
				_polygons.assign(new_rule.sequence.begin(), new_rule.sequence.end());
				vector<polygon_ptr> target_dst;
				if (!insert_bottom_left_score(target_dst)) { continue; } // 放不下
				coord_t target_height = get_skyline_height();
				new_rule.target_area = _bin_width * target_height;
				if (new_rule.target_area < picked_rule.target_area) {
					picked_rule = new_rule;
					if (picked_rule.target_area < _obj_area) {
						_obj_area = picked_rule.target_area;
						_dst = target_dst;
						_bin_height = target_height;
					}
				}
			}
			// 更新排序规则列表
			sort(_sort_rules.begin(), _sort_rules.end(), [](const SortRule &lhs, const SortRule &rhs) {
				return lhs.target_area > rhs.target_area; });
		}

		/// 基于最下最左和打分策略，贪心构造一个完整解
		bool insert_bottom_left_score(vector<polygon_ptr> &dst) {
			reset();
			dst.clear(); dst.reserve(_polygons.size());

			while (!_polygons.empty()) {
				auto bottom_skyline_iter = min_element(_skyline.begin(), _skyline.end(), [](skylinenode_t &lhs, skylinenode_t &rhs) { return lhs.y < rhs.y; });
				auto best_skyline_index = distance(_skyline.begin(), bottom_skyline_iter);

				polygon_ptr best_dst_node;
				size_t best_polygon_index;
				coord_t best_skyline_height;
				if (find_polygon_for_skyline_bottom_left(best_skyline_index, _polygons, best_dst_node, best_polygon_index, best_skyline_height)) {
					_polygons.remove(best_polygon_index);
					dst.push_back(best_dst_node);
					if (best_skyline_height > _bin_height) { return false; } // 超出_bin_height
				}
				else { // 填坑
					if (best_skyline_index == 0) { _skyline[best_skyline_index].y = _skyline[best_skyline_index + 1].y; }
					else if (best_skyline_index == _skyline.size() - 1) { _skyline[best_skyline_index].y = _skyline[best_skyline_index - 1].y; }
					else { _skyline[best_skyline_index].y = min(_skyline[best_skyline_index - 1].y, _skyline[best_skyline_index + 1].y); }
					merge_skylines(_skyline);
				}
			}

			return true;
		}

	private:
		void reset() {
			_skyline.clear();
			_skyline.push_back({ 0,0,_bin_width });
		}

		void init_sort_rules() {
			// 0_输入顺序
			vector<size_t> seq(_src.size());
			iota(seq.begin(), seq.end(), 0);
			_sort_rules.reserve(4);
			for (size_t i = 0; i < 4; ++i) { _sort_rules.push_back({ seq, numeric_limits<coord_t>::max() }); }
			//_tabu_table.insert((_sort_rules[0].*tabu_key)());
			// 1_面积递减
			sort(_sort_rules[1].sequence.begin(), _sort_rules[1].sequence.end(), [this](size_t lhs, size_t rhs) {
				return _src.at(lhs)->area > _src.at(rhs)->area; });
			//_tabu_table.insert((_sort_rules[1].*tabu_key)());
			// 2_最长边递减
			sort(_sort_rules[2].sequence.begin(), _sort_rules[2].sequence.end(), [this](size_t lhs, size_t rhs) {
				return _src.at(lhs)->max_length > _src.at(rhs)->max_length; });
			//_tabu_table.insert((_sort_rules[2].*tabu_key)());
			// 3_随机排序
			shuffle(_sort_rules[3].sequence.begin(), _sort_rules[3].sequence.end(), _gen);
			//_tabu_table.insert((_sort_rules[3].*tabu_key)());

			// 默认输入顺序
			_polygons.assign(_sort_rules[0].sequence.begin(), _sort_rules[0].sequence.end());

			// 离散概率分布初始化
			vector<int> probs; probs.reserve(_sort_rules.size());
			for (int i = 1; i <= _sort_rules.size(); ++i) { probs.push_back(2 * i); }
			_discrete_dist = discrete_distribution<>(probs.begin(), probs.end());
		}

		/// 邻域动作1：交换两个块的顺序
		void swap_sort_rule(SortRule &rule) {
			size_t a = _uniform_dist(_gen);
			size_t b = _uniform_dist(_gen);
			while (a == b) { b = _uniform_dist(_gen); }
			swap(rule.sequence[a], rule.sequence[b]);
		}

		/// 邻域动作2：连续多个块移动
		void rotate_sort_rule(SortRule &rule) {
			size_t a = _uniform_dist(_gen);
			rotate(rule.sequence.begin(), rule.sequence.begin() + a, rule.sequence.end());
		}

		/// 给最下最左的角选择最好的块
		bool find_polygon_for_skyline_bottom_left(size_t skyline_index, const list<size_t> &polygons,
			polygon_ptr &best_dst_node, size_t &best_polygon_index, coord_t &best_skyline_height) {

			int best_score = -1;
			for (size_t p : polygons) {
				switch (_src.at(p)->shape()) {
				case Shape::R: {
					auto rect = dynamic_pointer_cast<rect_t>(_src.at(p));
					coord_t x; int score;
					for (int rotate = 0; rotate <= 1; ++rotate) {
						coord_t w = rect->width, h = rect->height;
						if (rotate) { swap(w, h); }
						if (score_rect_for_skyline_bottom_left(skyline_index, w, h, x, score)) {
							if (best_score < score) {
								best_score = score;
								rect->lb_point.x = x;
								rect->lb_point.y = _skyline[skyline_index].y;
								rect->rotation = rotate ? Rotation::_90_ : Rotation::_0_;
								best_polygon_index = p;
							}
						}
					}
					break;
				}
				case Shape::L: {
					auto lshape = dynamic_pointer_cast<lshape_t>(_src.at(p));
					if (score_lshape_for_skyline_bottom_left(skyline_index, lshape, best_skyline_height)) {
						best_polygon_index = p;
						best_dst_node = make_shared<lshape_t>(*lshape);
						return true; // _skyline已被更新
					}
					break;
				}
				case Shape::T: {
					auto tshape = dynamic_pointer_cast<tshape_t>(_src.at(p));
					if (score_tshape_for_skyline_bottom_left(skyline_index, tshape, best_skyline_height)) {
						best_polygon_index = p;
						best_dst_node = make_shared<tshape_t>(*tshape);
						return true; // _skyline已被更新
					}
					break;
				}
				case Shape::C: {
					auto concave = dynamic_pointer_cast<concave_t>(_src.at(p));
					if (score_concave_for_skyline_bottom_left(skyline_index, concave, best_skyline_height)) {
						best_polygon_index = p;
						best_dst_node = make_shared<concave_t>(*concave);
						return true; // _skyline已被更新
					}
					break;
				}
				default: { assert(false); break; }
				}
			}

			if (best_score == -1) { return false; }

			// 运行到此处一定是矩形，更新_skyline
			assert(_src.at(best_polygon_index)->shape() == Shape::R);
			auto rect = dynamic_pointer_cast<rect_t>(_src.at(best_polygon_index));
			coord_t w = rect->width, h = rect->height;
			if (rect->rotation == Rotation::_90_) { swap(w, h); }
			skylinenode_t new_skyline_node{ rect->lb_point.x, rect->lb_point.y + h, w };
			if (rect->lb_point.x == _skyline[skyline_index].x) { // 靠左
				_skyline.insert(_skyline.begin() + skyline_index, new_skyline_node);
				_skyline[skyline_index + 1].x += w;
				_skyline[skyline_index + 1].width -= w;
				merge_skylines(_skyline);
			}
			else { // 靠右
				_skyline.insert(_skyline.begin() + skyline_index + 1, new_skyline_node);
				_skyline[skyline_index].width -= w;
				merge_skylines(_skyline);
			}
			best_skyline_height = new_skyline_node.y;
			best_dst_node = make_shared<rect_t>(*rect);
			return true;
		}

		/// Space定义
		struct SkylineSpace {
			coord_t x;
			coord_t y;
			coord_t width;
			coord_t hl;
			coord_t hr;
		};

		SkylineSpace skyline_nodo_to_space(size_t skyline_index) {
			coord_t hl, hr;
			if (_skyline.size() == 1) {
				hl = hr = INF - _skyline[skyline_index].y;
			}
			else if (skyline_index == 0) {
				hl = INF - _skyline[skyline_index].y;
				hr = _skyline[skyline_index + 1].y - _skyline[skyline_index].y;
			}
			else if (skyline_index == _skyline.size() - 1) {
				hl = _skyline[skyline_index - 1].y - _skyline[skyline_index].y;
				hr = INF - _skyline[skyline_index].y;
			}
			else {
				hl = _skyline[skyline_index - 1].y - _skyline[skyline_index].y;
				hr = _skyline[skyline_index + 1].y - _skyline[skyline_index].y;
			}
			return { _skyline[skyline_index].x, _skyline[skyline_index].y, _skyline[skyline_index].width, hl, hr };
		}

		/// R打分策略
		bool score_rect_for_skyline_bottom_left(size_t skyline_index, coord_t width, coord_t height, coord_t &x, int &score) {
			if (width > _skyline[skyline_index].width) { return false; }

			SkylineSpace space = skyline_nodo_to_space(skyline_index);
			if (space.hl >= space.hr) {
				if (width == space.width && height == space.hl) { score = 7; }
				else if (width == space.width && height == space.hr) { score = 6; }
				else if (width == space.width && height > space.hl) { score = 5; }
				else if (width < space.width && height == space.hl) { score = 4; }
				else if (width == space.width && height < space.hl && height > space.hr) { score = 3; }
				else if (width < space.width && height == space.hr) { score = 2; } // 靠右
				else if (width == space.width && height < space.hr) { score = 1; }
				else if (width < space.width && height != space.hl) { score = 0; }
				else { return false; }

				if (score == 2) { x = _skyline[skyline_index].x + _skyline[skyline_index].width - width; }
				else { x = _skyline[skyline_index].x; }
			}
			else { // hl < hr
				if (width == space.width && height == space.hr) { score = 7; }
				else if (width == space.width && height == space.hl) { score = 6; }
				else if (width == space.width && height > space.hr) { score = 5; }
				else if (width < space.width && height == space.hr) { score = 4; } // 靠右
				else if (width == space.width && height < space.hr && height > space.hl) { score = 3; }
				else if (width < space.width && height == space.hl) { score = 2; }
				else if (width == space.width && height < space.hl) { score = 1; }
				else if (width < space.width && height != space.hr) { score = 0; } // 靠右
				else { return false; }

				if (score == 4 || score == 0) { x = _skyline[skyline_index].x + _skyline[skyline_index].width - width; }
				else { x = _skyline[skyline_index].x; }
			}
			if (x + width > _bin_width) { return false; }

			return true;
		}

		/// L打分策略
		bool score_lshape_for_skyline_bottom_left(size_t skyline_index, lshape_ptr &lshape, coord_t &skyline_height) {
			SkylineSpace space = skyline_nodo_to_space(skyline_index);
			skyline_t skyline_0l = _skyline, skyline_0r = _skyline,
				skyline_90 = _skyline, skyline_180 = _skyline,
				skyline_270l = _skyline, skyline_270r = _skyline;
			size_t skyline_old_size = _skyline.size();
			int min_delta = numeric_limits<int>::max();

			if (lshape->hd <= space.width) { // 0&靠左
				// lb_point
				point_t lb_point_0l = { skyline_0l[skyline_index].x, skyline_0l[skyline_index].y };
				// update
				skyline_0l[skyline_index].y += lshape->vl;
				skyline_0l[skyline_index].width = lshape->hu;
				// add
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 1, {
					skyline_0l[skyline_index].x + skyline_0l[skyline_index].width,
					skyline_0l[skyline_index].y - lshape->vm,
					lshape->hm });
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 2, {
					skyline_0l[skyline_index + 1].x + skyline_0l[skyline_index + 1].width,
					skyline_0l[skyline_index + 1].y - lshape->vr,
					space.width - lshape->hd });
				// skyline_height
				skyline_height = skyline_0l[skyline_index].y;
				// merge
				merge_skylines(skyline_0l);
				// delta
				if (min_delta > skyline_0l.size() - skyline_old_size) {
					min_delta = skyline_0l.size() - skyline_old_size;
					lshape->rotation = Rotation::_0_;
					lshape->lb_point = lb_point_0l;
					_skyline = skyline_0l;
				}
			}

			if (lshape->hd < space.width) { // 0&靠右
				// lb_point
				point_t lb_point_0r = { skyline_0r[skyline_index].x + space.width - lshape->hd, skyline_0r[skyline_index].y };
				// update
				skyline_0r[skyline_index].width -= lshape->hd;
				// add
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 1, {
					skyline_0r[skyline_index].x + skyline_0r[skyline_index].width,
					skyline_0r[skyline_index].y + lshape->vl,
					lshape->hu });
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 2, {
					skyline_0r[skyline_index + 1].x + skyline_0r[skyline_index + 1].width,
					skyline_0r[skyline_index + 1].y - lshape->vm,
					lshape->hm });
				// skyline_height
				skyline_height = skyline_0r[skyline_index + 1].y;
				// merge
				merge_skylines(skyline_0r);
				// delta
				if (min_delta > skyline_0r.size() - skyline_old_size) {
					min_delta = skyline_0r.size() - skyline_old_size;
					lshape->rotation = Rotation::_0_;
					lshape->lb_point = lb_point_0r;
					_skyline = skyline_0r;
				}
			}

			if (skyline_index + 1 < skyline_90.size()
				&& lshape->vm <= skyline_90[skyline_index + 1].width
				&& lshape->hm == space.hr
				&& lshape->vr <= space.width) {
				// lb_point
				point_t lb_point_90 = { skyline_90[skyline_index].x + space.width - lshape->vr, skyline_90[skyline_index].y + lshape->hd };
				// update
				skyline_90[skyline_index].width -= lshape->vr;
				// add
				skyline_90.insert(skyline_90.begin() + skyline_index + 1, {
					skyline_90[skyline_index].x + skyline_90[skyline_index].width,
					skyline_90[skyline_index].y + lshape->hd,
					lshape->vl });
				// delete right
				skyline_90[skyline_index + 2].x += lshape->vm;
				skyline_90[skyline_index + 2].width -= lshape->vm;
				// skyline_height
				skyline_height = skyline_90[skyline_index + 1].y;
				// merge
				merge_skylines(skyline_90);
				// delta
				if (min_delta > skyline_90.size() - skyline_old_size) {
					min_delta = skyline_90.size() - skyline_old_size;
					lshape->rotation = Rotation::_90_;
					lshape->lb_point = lb_point_90;
					_skyline = skyline_90;
				}
			}

			if (skyline_index >= 1
				&& lshape->hm <= skyline_180[skyline_index - 1].width
				&& lshape->vm == space.hl
				&& lshape->hu <= space.width) {
				// lb_point
				point_t lb_point_180 = { skyline_180[skyline_index].x + lshape->hu, skyline_180[skyline_index].y + lshape->vl };
				// update
				skyline_180[skyline_index].x -= lshape->hm;
				skyline_180[skyline_index].y += lshape->vl;
				skyline_180[skyline_index].width = lshape->hd;
				// add
				skyline_180.insert(skyline_180.begin() + skyline_index + 1, {
					skyline_180[skyline_index].x + skyline_180[skyline_index].width,
					skyline_180[skyline_index].y - lshape->vl,
					space.width - lshape->hu });
				// delete left
				skyline_180[skyline_index - 1].width -= lshape->hm;
				// skyline_height
				skyline_height = skyline_180[skyline_index].y;
				// merge
				merge_skylines(skyline_180);
				// delta
				if (min_delta > skyline_180.size() - skyline_old_size) {
					min_delta = skyline_180.size() - skyline_old_size;
					lshape->rotation = Rotation::_180_;
					lshape->lb_point = lb_point_180;
					_skyline = skyline_180;
				}
			}

			if (lshape->vl <= space.width) { // 270&靠左
				// lb_point
				point_t lb_point_270l = { skyline_270l[skyline_index].x + lshape->vl, skyline_270l[skyline_index].y };
				// update
				skyline_270l[skyline_index].y += lshape->hu;
				skyline_270l[skyline_index].width = lshape->vm;
				// add
				skyline_270l.insert(skyline_270l.begin() + skyline_index + 1, {
					skyline_270l[skyline_index].x + skyline_270l[skyline_index].width,
					skyline_270l[skyline_index].y + lshape->hm,
					lshape->vr });
				skyline_270l.insert(skyline_270l.begin() + skyline_index + 2, {
					skyline_270l[skyline_index + 1].x + skyline_270l[skyline_index + 1].width,
					skyline_270l[skyline_index + 1].y - lshape->hd,
					space.width - lshape->vl });
				// skyline_height
				skyline_height = skyline_270l[skyline_index + 1].y;
				// merge
				merge_skylines(skyline_270l);
				// delta
				if (min_delta > skyline_270l.size() - skyline_old_size) {
					min_delta = skyline_270l.size() - skyline_old_size;
					lshape->rotation = Rotation::_270_;
					lshape->lb_point = lb_point_270l;
					_skyline = skyline_270l;
				}
			}

			if (lshape->vl < space.width) { // 270&靠右
				// lb_point
				point_t lb_point_270r = { skyline_270r[skyline_index].x + space.width, skyline_270r[skyline_index].y };
				// update
				skyline_270r[skyline_index].width -= lshape->vl;
				// add
				skyline_270r.insert(skyline_270r.begin() + skyline_index + 1, {
					skyline_270r[skyline_index].x + skyline_270r[skyline_index].width,
					skyline_270r[skyline_index].y + lshape->hu,
					lshape->vm });
				skyline_270r.insert(skyline_270r.begin() + skyline_index + 2, {
					skyline_270r[skyline_index + 1].x + skyline_270r[skyline_index + 1].width,
					skyline_270r[skyline_index + 1].y + lshape->hm,
					lshape->vr });
				// skyline_height
				skyline_height = skyline_270r[skyline_index + 2].y;
				// merge
				merge_skylines(skyline_270r);
				// delta
				if (min_delta > skyline_270r.size() - skyline_old_size) {
					min_delta = skyline_270r.size() - skyline_old_size;
					lshape->rotation = Rotation::_270_;
					lshape->lb_point = lb_point_270r;
					_skyline = skyline_270r;
				}
			}

			if (min_delta == numeric_limits<int>::max()) { return false; }
			return true;
		}

		/// T打分策略
		bool score_tshape_for_skyline_bottom_left(size_t skyline_index, tshape_ptr &tshape, coord_t &skyline_height) {
			SkylineSpace space = skyline_nodo_to_space(skyline_index);
			skyline_t skyline_0l = _skyline, skyline_0r = _skyline,
				skyline_90 = _skyline, skyline_180 = _skyline, skyline_270 = _skyline;
			size_t skyline_old_size = _skyline.size();
			int min_delta = numeric_limits<int>::max();

			if (tshape->hd <= space.width) { // 0&靠左
				// lb_point
				point_t lb_point_0l = { skyline_0l[skyline_index].x, skyline_0l[skyline_index].y };
				// update
				skyline_0l[skyline_index].y += tshape->vld;
				skyline_0l[skyline_index].width = tshape->hl;
				// add
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 1, {
					skyline_0l[skyline_index].x + skyline_0l[skyline_index].width,
					skyline_0l[skyline_index].y + tshape->vlu,
					tshape->hu });
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 2, {
					skyline_0l[skyline_index + 1].x + skyline_0l[skyline_index + 1].width,
					skyline_0l[skyline_index + 1].y - tshape->vru,
					tshape->hr });
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 3, {
					skyline_0l[skyline_index + 2].x + skyline_0l[skyline_index + 2].width,
					skyline_0l[skyline_index + 2].y - tshape->vrd,
					space.width - tshape->hd });
				// skyline_height
				skyline_height = skyline_0l[skyline_index + 1].y;
				// merge
				merge_skylines(skyline_0l);
				// delta
				if (min_delta > skyline_0l.size() - skyline_old_size) {
					min_delta = skyline_0l.size() - skyline_old_size;
					tshape->rotation = Rotation::_0_;
					tshape->lb_point = lb_point_0l;
					_skyline = skyline_0l;
				}
			}

			if (tshape->hd < space.width) { // 0&靠右
				// lb_point
				point_t lb_point_0r = { skyline_0r[skyline_index].x + space.width - tshape->hd, skyline_0r[skyline_index].y };
				// update
				skyline_0r[skyline_index].width -= tshape->hd;
				// add
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 1, {
					skyline_0r[skyline_index].x + skyline_0r[skyline_index].width,
					skyline_0r[skyline_index].y + tshape->vld,
					tshape->hl });
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 2, {
					skyline_0r[skyline_index + 1].x + skyline_0r[skyline_index + 1].width,
					skyline_0r[skyline_index + 1].y + tshape->vlu,
					tshape->hu });
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 3, {
					skyline_0r[skyline_index + 2].x + skyline_0r[skyline_index + 2].width,
					skyline_0r[skyline_index + 2].y - tshape->vru,
					tshape->hr });
				// skyline_height
				skyline_height = skyline_0r[skyline_index + 2].y;
				// merge
				merge_skylines(skyline_0r);
				// delta
				if (min_delta > skyline_0r.size() - skyline_old_size) {
					min_delta = skyline_0r.size() - skyline_old_size;
					tshape->rotation = Rotation::_0_;
					tshape->lb_point = lb_point_0r;
					_skyline = skyline_0r;
				}
			}

			if (skyline_index + 1 < skyline_90.size()
				&& tshape->vru <= skyline_90[skyline_index + 1].width
				&& tshape->hr == space.hr
				&& tshape->vrd <= space.width) {
				// lb_point
				point_t lb_point_90 = { skyline_90[skyline_index].x + space.width - tshape->vrd, skyline_90[skyline_index].y + tshape->hd };
				// update
				skyline_90[skyline_index].width -= tshape->vrd;
				// add
				skyline_90.insert(skyline_90.begin() + skyline_index + 1, {
					skyline_90[skyline_index].x + skyline_90[skyline_index].width,
					skyline_90[skyline_index].y + tshape->hd,
					tshape->vld });
				skyline_90.insert(skyline_90.begin() + skyline_index + 2, {
					skyline_90[skyline_index + 1].x + skyline_90[skyline_index + 1].width,
					skyline_90[skyline_index + 1].y - tshape->hl,
					tshape->vlu });
				// delete right
				skyline_90[skyline_index + 3].x += tshape->vru;
				skyline_90[skyline_index + 3].width -= tshape->vru;
				// skyline_height
				skyline_height = skyline_90[skyline_index + 1].y;
				// merge
				merge_skylines(skyline_90);
				// delta
				if (min_delta > skyline_90.size() - skyline_old_size) {
					min_delta = skyline_90.size() - skyline_old_size;
					tshape->rotation = Rotation::_90_;
					tshape->lb_point = lb_point_90;
					_skyline = skyline_90;
				}
			}

			if (skyline_index + 1 < skyline_180.size() && skyline_index >= 1
				&& tshape->hl <= skyline_180[skyline_index + 1].width
				&& tshape->hr <= skyline_180[skyline_index - 1].width
				&& tshape->vlu == space.hr
				&& tshape->vru == space.hl
				&& tshape->hu == space.width) {
				// lb_point
				point_t lb_point_180 = { skyline_180[skyline_index].x + tshape->hu + tshape->hl, skyline_180[skyline_index].y + tshape->vlu + tshape->vld };
				// update
				skyline_180[skyline_index].x -= tshape->hr;
				skyline_180[skyline_index].y += (tshape->vru + tshape->vrd);
				skyline_180[skyline_index].width = tshape->hd;
				// delete left
				skyline_180[skyline_index - 1].width -= tshape->hr;
				// delete right
				skyline_180[skyline_index + 1].x += tshape->hl;
				skyline_180[skyline_index + 1].width -= tshape->hl;
				// skyline_height
				skyline_height = skyline_180[skyline_index].y;
				// merge
				merge_skylines(skyline_180);
				// delta
				if (min_delta > skyline_180.size() - skyline_old_size) {
					min_delta = skyline_180.size() - skyline_old_size;
					tshape->rotation = Rotation::_180_;
					tshape->lb_point = lb_point_180;
					_skyline = skyline_180;
				}
			}

			if (skyline_index >= 1
				&& tshape->vlu <= skyline_270[skyline_index - 1].width
				&& tshape->hl == space.hl
				&& tshape->vld <= space.width) {
				// lb_point
				point_t lb_point_270 = { skyline_270[skyline_index].x + tshape->vld, skyline_270[skyline_index].y };
				// update
				skyline_270[skyline_index].x -= tshape->vlu;
				skyline_270[skyline_index].y += (tshape->hl + tshape->hu);
				skyline_270[skyline_index].width = tshape->vru;
				// add
				skyline_270.insert(skyline_270.begin() + skyline_index + 1, {
					skyline_270[skyline_index].x + skyline_270[skyline_index].width,
					skyline_270[skyline_index].y + tshape->hr,
					tshape->vrd });
				skyline_270.insert(skyline_270.begin() + skyline_index + 2, {
					skyline_270[skyline_index + 1].x + skyline_270[skyline_index + 1].width,
					skyline_270[skyline_index + 1].y - tshape->hd,
					space.width - tshape->vld });
				// delete left
				skyline_270[skyline_index - 1].width -= tshape->vlu;
				// skyline_height
				skyline_height = skyline_270[skyline_index + 1].y;
				// merge
				merge_skylines(skyline_270);
				// delta
				if (min_delta > skyline_270.size() - skyline_old_size) {
					min_delta = skyline_270.size() - skyline_old_size;
					tshape->rotation = Rotation::_270_;
					tshape->lb_point = lb_point_270;
					_skyline = skyline_270;
				}
			}

			if (min_delta == numeric_limits<int>::max()) { return false; }
			return true;
		}

		/// C打分策略
		bool score_concave_for_skyline_bottom_left(size_t skyline_index, concave_ptr &concave, coord_t &skyline_height) {
			SkylineSpace space = skyline_nodo_to_space(skyline_index);
			skyline_t skyline_0l = _skyline, skyline_0r = _skyline;
			size_t skyline_old_size = _skyline.size();
			int min_delta = numeric_limits<int>::max();

			if (concave->hd <= space.width) { // 0&靠左
				// lb_point
				point_t lb_point_0l = { skyline_0l[skyline_index].x, skyline_0l[skyline_index].y };
				// update
				skyline_0l[skyline_index].y += concave->vld;
				skyline_0l[skyline_index].width = concave->hl;
				// add
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 1, {
					skyline_0l[skyline_index].x + skyline_0l[skyline_index].width,
					skyline_0l[skyline_index].y - concave->vlu,
					concave->hu });
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 2, {
					skyline_0l[skyline_index + 1].x + skyline_0l[skyline_index + 1].width,
					skyline_0l[skyline_index + 1].y + concave->vru,
					concave->hr });
				skyline_0l.insert(skyline_0l.begin() + skyline_index + 3, {
					skyline_0l[skyline_index + 2].x + skyline_0l[skyline_index + 2].width,
					skyline_0l[skyline_index + 2].y - concave->vrd,
					space.width - concave->hd });
				// skyline_height
				skyline_height = max(skyline_0l[skyline_index].y, skyline_0l[skyline_index + 2].y);
				// merge
				merge_skylines(skyline_0l);
				// delta
				if (min_delta > skyline_0l.size() - skyline_old_size) {
					min_delta = skyline_0l.size() - skyline_old_size;
					concave->rotation = Rotation::_0_;
					concave->lb_point = lb_point_0l;
					_skyline = skyline_0l;
				}
			}

			if (concave->hd < space.width) { // 0&靠右
				// lb_point
				point_t lb_point_0r = { skyline_0r[skyline_index].x + space.width - concave->hd, skyline_0r[skyline_index].y };
				// update
				skyline_0r[skyline_index].width -= concave->hd;
				// add
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 1, {
					skyline_0r[skyline_index].x + skyline_0r[skyline_index].width,
					skyline_0r[skyline_index].y + concave->vld,
					concave->hl });
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 2, {
					skyline_0r[skyline_index + 1].x + skyline_0r[skyline_index + 1].width,
					skyline_0r[skyline_index + 1].y - concave->vlu,
					concave->hu });
				skyline_0r.insert(skyline_0r.begin() + skyline_index + 3, {
					skyline_0r[skyline_index + 2].x + skyline_0r[skyline_index + 2].width,
					skyline_0r[skyline_index + 2].y + concave->vru,
					concave->hr });
				// skyline_height
				skyline_height = max(skyline_0r[skyline_index + 1].y, skyline_0r[skyline_index + 3].y);
				// merge
				merge_skylines(skyline_0r);
				// delta
				if (min_delta > skyline_0r.size() - skyline_old_size) {
					min_delta = skyline_0r.size() - skyline_old_size;
					concave->rotation = Rotation::_0_;
					concave->lb_point = lb_point_0r;
					_skyline = skyline_0r;
				}
			}

			if (min_delta == numeric_limits<int>::max()) { return false; }
			return true;
		}

		/// 合并同一level的skyline节点.
		static void merge_skylines(skyline_t &skyline) {
			skyline.erase(
				remove_if(skyline.begin(), skyline.end(), [](skylinenode_t &lhs) { return lhs.width <= 0; }),
				skyline.end()
			);
			for (size_t i = 0; i < skyline.size() - 1; ++i) {
				if (skyline[i].y == skyline[i + 1].y) {
					skyline[i].width += skyline[i + 1].width;
					skyline.erase(skyline.begin() + i + 1);
					--i;
				}
			}
		}

	private:
		// 输入
		const vector<polygon_ptr> &_src;
		coord_t _bin_width;
		coord_t _bin_height;

		// 输出
		vector<polygon_ptr> _dst;
		coord_t _obj_area;

		skyline_t _skyline;
		vector<SortRule> _sort_rules; // 排序规则列表，用于RLS
		list<size_t> _polygons;		  // SortRule的sequence，相当于指针，使用list快速删除，放置完毕为空
		//TabuTable _tabu_table;        // 禁忌表
		discrete_distribution<> _discrete_dist;   // 离散概率分布，用于挑选规则(即挑选sequence赋给_polygons)
		uniform_int_distribution<> _uniform_dist; // 均匀分布，用于交换sequence顺序
		default_random_engine &_gen;
	};

}

#endif // SMARTMPW_MPWBINPACK_HPP
