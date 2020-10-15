////
//// @author   liyan
//// @contact  lyan_dut@outlook.com
////
//#pragma once
//
//#include "Config.hpp"
//#include "PreCombine.hpp"
//#include "MpwBinPack.hpp"
//
//class AdaptSelect {
//public:
//
//	AdaptSelect() = delete;
//
//	AdaptSelect(const Environment &env, const Config &cfg) : _env(env), _cfg(cfg), _ins(env) {}
//
//	/// 候选宽度定义
//	struct CandidateWidth {
//		int value;
//		int iter;
//		unique_ptr<MpwBinPack> mbp_solver; // 存指针，减少排序造成的开销
//	};
//
//	void run() {
//
//		_start = clock();
//
//		vector<int> candidate_widths = cal_candidate_widths_on_interval();
//
//		// 分支初始化iter=1
//		vector<CandidateWidth> cw_objs; cw_objs.reserve(candidate_widths.size());
//		for (int bin_width : candidate_widths) {
//			cw_objs.push_back({ bin_width, 1, unique_ptr<MpwBinPack>(new MpwBinPack()) });
//			cw_objs.back().mbp_solver->random_local_search();
//			check_cwobj(cw_objs.back());
//		}
//		// 降序排列，越后面的选中概率越大
//		sort(cw_objs.begin(), cw_objs.end(), [](auto &lhs, auto &rhs) {
//			return lhs.mbp_solver->get_obj_area() > rhs.mbp_solver->get_obj_area(); });
//
//		// 初始化离散概率分布
//		vector<int> probs; probs.reserve(cw_objs.size());
//		for (int i = 1; i <= cw_objs.size(); ++i) { probs.push_back(2 * i); }
//		discrete_distribution<> discrete_dist(probs.begin(), probs.end());
//
//		// 迭代优化 
//		while ((clock() - _start) / static_cast<double>(CLOCKS_PER_SEC) < _cfg.ub_time) {
//			CandidateWidth &picked_width = cw_objs[discrete_dist(_gen)];
//			picked_width.iter = min(2 * picked_width.iter, _cfg.ub_iter);
//			picked_width.mbp_solver->random_local_search(picked_width.iter);
//			check_cwobj(picked_width);
//			sort(cw_objs.begin(), cw_objs.end(), [](auto &lhs, auto &rhs) {
//				return lhs.mbp_solver->get_obj_area() > rhs.mbp_solver->get_obj_area(); });
//		}
//	}
//
//private:
//	/// 在区间[W_min, W_max]内，等距地生成候选宽度
//	vector<int> cal_candidate_widths_on_interval(int interval = 1) {
//		vector<int> candidate_widths;
//		candidate_widths.reserve(_cfg.ub_width - _cfg.lb_width + 1);
//		for (int cw = _cfg.lb_width; cw <= _cfg.ub_width; cw += interval) {
//			if (cw * _cfg.ub_height > _ins.get_total_area()) { candidate_widths.push_back(cw); }
//		}
//		return candidate_widths;
//	}
//
//	/// 检查cw_obj的RLS结果
//	void check_cwobj(const CandidateWidth &cw_obj) {
//		if (cw_obj.mbp_solver->get_obj_area() / cw_obj.value > _cfg.ub_height) { // 当前解无效
//			cw_obj.mbp_solver->reset_obj_area();
//		}
//		if (cw_obj.mbp_solver->get_obj_area() < _obj_area) {
//			_obj_area = cw_obj.mbp_solver->get_obj_area();
//			_dst_rects = cw_obj.mbp_solver->get_dst_rects();
//			_dst_lshapes = cw_obj.mbp_solver->get_dst_lshapes();
//			_dst_tshapes = cw_obj.mbp_solver->get_dst_tshapes();
//			_best_duration = (clock() - _start) / static_cast<double>(CLOCKS_PER_SEC);
//		}
//		cw_obj.mbp_solver->visualize();
//	}
//
//private:
//	const Environment &_env;
//	const Config &_cfg;
//
//	Instance _ins;
//	default_random_engine _gen;
//	clock_t _start;
//	double _best_duration;
//
//	int _obj_area;
//	vector<rect_t> _dst_rects;
//	vector<lshape_t> _dst_lshapes;
//	vector<tshape_t> _dst_tshapes;
//};