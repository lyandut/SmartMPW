//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMARTMPW_CHECKER_VISUALIZER_HPP
#define SMARTMPW_CHECKER_VISUALIZER_HPP

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/geometries.hpp>

using coord_t = int;

/// 基本数据结构定义
namespace utils_visualize {

	namespace bg = boost::geometry;

	using T = coord_t;

	// n 维点坐标
	template<size_t dimension = 2>
	using bg_point_base = bg::model::point<T, dimension, bg::cs::cartesian>;

	/*****************************
	* 以下定义全部针对二维点坐标 *
	******************************/

	// 二维坐标点
	using bg_point_t = bg::model::d2::point_xy<T>;
	const bg_point_t origin_point(0, 0);  // 坐标原点

	// 曲线
	using bg_linestring_t = bg::model::linestring<bg_point_t>;

	// 多边形（包括0个或者多个内环 inner rings，逆时针，起点=终点）
	using bg_polygon_t = bg::model::polygon<bg_point_t, false, true>;

	// 环（封闭曲线，顺时针，起点≠终点）
	using bg_ring_t = bg::model::ring<bg_point_t, true, false>;
	//using bg_ring_t = bg_polygon_t::ring_type;

	// 点集合
	using bg_multi_point_t = bg::model::multi_point<bg_point_t>;

	// 曲线集合
	using bg_multi_linestring_t = bg::model::multi_linestring<bg_linestring_t>;

	// 多边形集合
	using bg_multi_polygon_t = bg::model::multi_polygon<bg_polygon_t>;

	// 矩形
	using bg_box_t = bg::model::box<bg_point_t>;

	// 线段（坐标点对）
	using bg_segment_t = bg::model::segment<bg_point_t>;
}

#endif // SMARTMPW_CHECKER_VISUALIZER_HPP
