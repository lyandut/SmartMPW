//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMARTMPW_ADAPTSELECT_HPP
#define SMARTMPW_ADAPTSELECT_HPP

#include "Instance.hpp"
#include "MpwBinPack.hpp"

using namespace mbp;

class AdaptSelect {

	/// 候选宽度定义
	struct CandidateWidth {
		coord_t value;
		int iter;
		unique_ptr<MpwBinPack> mbp_solver; // 存指针，减少排序造成的开销
	};

public:

	AdaptSelect() = delete;

	AdaptSelect(const Environment &env, const Config &cfg) :
		_env(env), _cfg(cfg), _ins(env), _gen(_cfg.random_seed),
		_obj_area(numeric_limits<coord_t>::max()) {}

	void run() {

		_start = clock();

		vector<coord_t> candidate_widths = cal_candidate_widths_on_interval();

		// 分支初始化iter=1
		vector<CandidateWidth> cw_objs; cw_objs.reserve(candidate_widths.size());
		for (coord_t bin_width : candidate_widths) {
			cw_objs.push_back({ bin_width, 1, unique_ptr<MpwBinPack>(
				new MpwBinPack(_ins.get_polygon_ptrs(), bin_width, INF, _gen)) });
			cw_objs.back().mbp_solver->random_local_search(1);
			check_cwobj(cw_objs.back());
		}

		// 降序排列，越后面的选中概率越大
		sort(cw_objs.begin(), cw_objs.end(), [](const CandidateWidth &lhs, const CandidateWidth &rhs) {
			return lhs.mbp_solver->get_obj_area() > rhs.mbp_solver->get_obj_area(); });

		// 初始化离散概率分布
		vector<int> probs; probs.reserve(cw_objs.size());
		for (int i = 1; i <= cw_objs.size(); ++i) { probs.push_back(2 * i); }
		discrete_distribution<> discrete_dist(probs.begin(), probs.end());

		// 迭代优化
		int curr_iter = 0; _iteration = 0;
		while ((clock() - _start) / static_cast<double>(CLOCKS_PER_SEC) < _cfg.ub_asa_time
			&& curr_iter++ - _iteration < _cfg.ub_asa_iter) {
			CandidateWidth &picked_width = cw_objs[discrete_dist(_gen)];
			picked_width.iter = min(2 * picked_width.iter, _cfg.ub_rls_iter);
			picked_width.mbp_solver->random_local_search(picked_width.iter);
			_iteration = check_cwobj(picked_width) ? curr_iter : _iteration;
			sort(cw_objs.begin(), cw_objs.end(), [](const CandidateWidth &lhs, const CandidateWidth &rhs) {
				return lhs.mbp_solver->get_obj_area() > rhs.mbp_solver->get_obj_area(); });
		}
	}

	void record_sol(const string &sol_path) const {
		ofstream sol_file(sol_path);
		for (auto &dst_node : _dst) {
			sol_file << "In Polygon:" << endl;
			for (auto &point : *dst_node->in_points) { sol_file << "(" << point.x << "," << point.y << ")"; }
			sol_file << endl << "Out Polygon:" << endl;
			vis::bg::for_each_point(dst_node->ring, [&](vis::bg_point_t &point) {
				sol_file << "(" << point.x() << "," << point.y() << ")"; });
			sol_file << endl;
		}
	}

#ifndef SUBMIT
	void draw_ins() const {
		ifstream ifs(_env.ins_html_path());
		if (ifs.good()) { return; }
		utils_visualize_drawer::Drawer html_drawer(_env.ins_html_path(), _cfg.ub_width, _cfg.ub_height);
		for (auto &src_node : _ins.get_polygon_ptrs()) {
			string polygon_str;
			vis::bg::for_each_point(src_node->ring, [&](vis::bg_point_t &point) {
				polygon_str += to_string(point.x()) + "," + to_string(point.y()) + " "; });
			html_drawer.polygon(polygon_str);
		}
	}

	void draw_sol(const string &html_path) const {
		utils_visualize_drawer::Drawer html_drawer(html_path, _cfg.ub_width, _cfg.ub_height);
		for (auto &dst_node : _dst) {
			string polygon_str;
			vis::bg::for_each_point(dst_node->ring, [&](vis::bg_point_t &point) {
				polygon_str += to_string(point.x()) + "," + to_string(point.y()) + " "; });
			html_drawer.polygon(polygon_str);
		}
	}

	void record_log() const {
		ofstream log_file(_env.log_path(), ios::app);
		log_file.seekp(0, ios::end);
		if (log_file.tellp() <= 0) {
			log_file << "Instance,"
				"InsArea,ObjArea,FillRatio,"
				"Width,Height,WHRatio,"
				"Iteration,Duration,TotalDuration,RandomSeed" << endl;
		}
		log_file << _env.instance_name() << ","
			<< _ins.get_total_area() << "," << _obj_area << "," << _fill_ratio << ","
			<< _width << "," << _height << "," << _wh_ratio << ","
			<< _iteration << "," << _duration << ","
			<< (clock() - _start) / static_cast<double>(CLOCKS_PER_SEC) << "," << _cfg.random_seed << endl;
	}
#endif // !SUBMIT

private:
	/// 在区间[lb_width, ub_width]内，等距地生成候选宽度
	vector<coord_t> cal_candidate_widths_on_interval(coord_t interval = 1) {
		vector<coord_t> candidate_widths;
		coord_t min_width = 0, max_width = 0;
		for (auto &ptr : _ins.get_polygon_ptrs()) {
			min_width = max(min_width, ptr->max_length);
			max_width += ptr->max_length;
		}
		min_width = ceil(max(min_width, _cfg.lb_width));
		max_width = ceil(min(max_width, _cfg.ub_width));

		candidate_widths.reserve(max_width - min_width + 1);
		for (coord_t cw = min_width; cw <= max_width; cw += interval) {
			if (cw * _cfg.ub_height < _ins.get_total_area()) { continue; }
			candidate_widths.push_back(cw);
		}
		return candidate_widths;
	}

	/// 检查cw_obj的RLS结果
	bool check_cwobj(const CandidateWidth &cw_obj) {
		coord_t cw_height = cw_obj.mbp_solver->get_obj_area() / cw_obj.value;
		if (cw_height > _cfg.ub_height) { // 解高度超出上界，不合法
			cw_obj.mbp_solver->set_obj_area(numeric_limits<coord_t>::max());
		}
		if (cw_height < _cfg.lb_height) { // 解高度不足下界，按下界计算
			cw_obj.mbp_solver->set_obj_area(cw_obj.value * _cfg.lb_height);
		}
		if (cw_obj.mbp_solver->get_obj_area() < _obj_area) {
			_obj_area = cw_obj.mbp_solver->get_obj_area();
			_fill_ratio = 1.0 * _ins.get_total_area() / _obj_area;
			_width = cw_obj.value;
			_height = cw_height;
			_wh_ratio = 1.0 * _width / _height;
			_dst = cw_obj.mbp_solver->get_dst();
			_duration = (clock() - _start) / static_cast<double>(CLOCKS_PER_SEC);
			return true;
		}
		return false;
	}

private:
	const Environment &_env;
	const Config &_cfg;

	const Instance _ins;
	default_random_engine _gen;
	clock_t _start;
	double _duration; // 最优解出现时间
	int _iteration;   // 最优解出现迭代次数

	coord_t _obj_area;
	double _fill_ratio;
	coord_t _width;
	coord_t _height;
	double _wh_ratio;
	vector<polygon_ptr> _dst;
};

#endif // SMARTMPW_ADAPTSELECT_HPP
