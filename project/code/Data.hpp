//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMARTMPW_DATA_HPP
#define SMARTMPW_DATA_HPP

#include <vector>
#include <memory>
#include <cassert>

#include "Config.hpp"

enum Shape { R, L, T, C };

enum Rotation { _0_, _90_, _180_, _270_ };

enum Direction { Left, Right, Up, Down };

template<typename T>
struct Point {
	T x, y;

	Point(T x_, T y_) : x(x_), y(y_) {}
	bool operator==(const Point& p) { return x == p.x && y == p.y; }
};

template<typename T>
struct Segment {
	Point<T> beg;
	Point<T> end;
	Direction dir;
	T len;

	Segment(const Point<T> &begin_, const Point<T> &end_)
		: beg(begin_), end(end_) {

		if (beg.x == end.x && beg.y < end.y) {
			len = end.y - beg.y;
			dir = Direction::Up;
		}
		else if (beg.x == end.x && beg.y > end.y) {
			len = beg.y - end.y;
			dir = Direction::Down;
		}
		else if (beg.y == end.y && beg.x < end.x) {
			len = end.x - beg.x;
			dir = Direction::Right;
		}
		else if (beg.y == end.y && beg.x > end.x) {
			len = beg.x - end.x;
			dir = Direction::Left;
		}
		else { assert(false); }
	}

	bool is_vertical(const Segment &seg) const {
		if (dir == Direction::Up || dir == Direction::Down)
			return (seg.dir == Direction::Left || seg.dir == Direction::Right);
		else
			return (seg.dir == Direction::Up || seg.dir == Direction::Down);
	}

	Direction next_turn(const Segment &next_seg) const {
		if (dir == Direction::Up && next_seg.dir == Direction::Left) return Direction::Left;
		else if (dir == Direction::Up && next_seg.dir == Direction::Right) return Direction::Right;
		else if (dir == Direction::Down && next_seg.dir == Direction::Right) return Direction::Left;
		else if (dir == Direction::Down && next_seg.dir == Direction::Left) return Direction::Right;
		else if (dir == Direction::Left && next_seg.dir == Direction::Down) return Direction::Left;
		else if (dir == Direction::Left && next_seg.dir == Direction::Up) return Direction::Right;
		else if (dir == Direction::Right && next_seg.dir == Direction::Up) return Direction::Left;
		else assert(dir == Direction::Right && next_seg.dir == Direction::Down); return Direction::Right;
	}
};

template<typename T>
struct Polygon {
	const int id;
	const std::shared_ptr<const std::vector<Point<T>>> in_points; // 输入坐标序列
	T area;
	T max_length;
	Point<T> lb_point; // 参考坐标（求解）
	Rotation rotation; // 旋转角度（求解）
	std::vector<Point<T>> out_points; // 输出坐标序列

	Polygon(int id_, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments) :
		id(id_), in_points(std::make_shared<const std::vector<Point<T>>>(points)),
		lb_point(0, 0), rotation(Rotation::_0_),
		max_length(max_element(segments.begin(), segments.end(),
			[](const Segment<T> &lhs, const Segment<T> &rhs) { return lhs.len < rhs.len; })->len) {}

	virtual Shape shape() = 0;
	virtual void to_out_points() = 0; // 根据lb_point和rotation确定out_points，仅写sol文件时调用即可
};

template<typename T>
struct Rect : public Polygon<T> {
	T width, height;

	Rect(int id, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, points, segments) {
		assert(segments.size() == 4);
		width = segments[0].len;
		height = segments[1].len;
		assert(width == segments[2].len && height == segments[3].len);
		this->area = width * height;
	}

	Shape shape() { return Shape::R; }

	void to_out_points() {
		this->out_points.clear();
		this->out_points.reserve(4);
		coord_t w = width, h = height;
		if (this->rotation == Rotation::_90_) { std::swap(w, h); }
		this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
		this->out_points.emplace_back(this->lb_point.x, this->lb_point.y + h);
		this->out_points.emplace_back(this->lb_point.x + w, this->lb_point.y + h);
		this->out_points.emplace_back(this->lb_point.x + w, this->lb_point.y);
	}
};

template<typename T>
struct LShape : public Polygon<T> {
	T hd, hm, hu; // hd=hm+hu
	T vl, vm, vr; // vl=vm+vr

	LShape(int id, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, points, segments) {
		std::vector<size_t> up_segs; up_segs.reserve(2);
		std::vector<size_t> down_segs; down_segs.reserve(2);
		std::vector<size_t> left_segs; left_segs.reserve(2);
		std::vector<size_t> right_segs; right_segs.reserve(2);

		for (size_t i = 0; i < segments.size(); ++i) {
			switch (segments[i].dir) {
			case Direction::Up:    up_segs.push_back(i);    break;
			case Direction::Down:  down_segs.push_back(i);  break;
			case Direction::Left:  left_segs.push_back(i);  break;
			case Direction::Right: right_segs.push_back(i); break;
			default:               assert(false);           break;
			}
		}

		if (up_segs.size() == 1 && left_segs.size() == 1) {
			assert(down_segs.size() == 2 && right_segs.size() == 2);
			if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Up) { // 顺
				hd = segments[left_segs[0]].len;
				vl = segments[(left_segs[0] + 1) % segments.size()].len;
				hu = segments[(left_segs[0] + 2) % segments.size()].len;
				vm = segments[(left_segs[0] + 3) % segments.size()].len;
				hm = segments[(left_segs[0] + 4) % segments.size()].len;
				vr = segments[(left_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Left) { // 逆
				hd = segments[left_segs[0]].len;
				vr = segments[(left_segs[0] + 1) % segments.size()].len;
				hm = segments[(left_segs[0] + 2) % segments.size()].len;
				vm = segments[(left_segs[0] + 3) % segments.size()].len;
				hu = segments[(left_segs[0] + 4) % segments.size()].len;
				vl = segments[(left_segs[0] + 5) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (up_segs.size() == 1 && right_segs.size() == 1) {
			assert(down_segs.size() == 2 && left_segs.size() == 2);
			if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Right) { // 顺
				hd = segments[up_segs[0]].len;
				vl = segments[(up_segs[0] + 1) % segments.size()].len;
				hu = segments[(up_segs[0] + 2) % segments.size()].len;
				vm = segments[(up_segs[0] + 3) % segments.size()].len;
				hm = segments[(up_segs[0] + 4) % segments.size()].len;
				vr = segments[(up_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Up) { // 逆
				hd = segments[up_segs[0]].len;
				vr = segments[(up_segs[0] + 1) % segments.size()].len;
				hm = segments[(up_segs[0] + 2) % segments.size()].len;
				vm = segments[(up_segs[0] + 3) % segments.size()].len;
				hu = segments[(up_segs[0] + 4) % segments.size()].len;
				vl = segments[(up_segs[0] + 5) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (down_segs.size() == 1 && left_segs.size() == 1) {
			assert(up_segs.size() == 2 && right_segs.size() == 2);
			if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Left) { // 顺
				hd = segments[down_segs[0]].len;
				vl = segments[(down_segs[0] + 1) % segments.size()].len;
				hu = segments[(down_segs[0] + 2) % segments.size()].len;
				vm = segments[(down_segs[0] + 3) % segments.size()].len;
				hm = segments[(down_segs[0] + 4) % segments.size()].len;
				vr = segments[(down_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Down) { // 逆
				hd = segments[down_segs[0]].len;
				vr = segments[(down_segs[0] + 1) % segments.size()].len;
				hm = segments[(down_segs[0] + 2) % segments.size()].len;
				vm = segments[(down_segs[0] + 3) % segments.size()].len;
				hu = segments[(down_segs[0] + 4) % segments.size()].len;
				vl = segments[(down_segs[0] + 5) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (down_segs.size() == 1 && right_segs.size() == 1) {
			assert(up_segs.size() == 2 && left_segs.size() == 2);
			if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Down) { // 顺
				hd = segments[right_segs[0]].len;
				vl = segments[(right_segs[0] + 1) % segments.size()].len;
				hu = segments[(right_segs[0] + 2) % segments.size()].len;
				vm = segments[(right_segs[0] + 3) % segments.size()].len;
				hm = segments[(right_segs[0] + 4) % segments.size()].len;
				vr = segments[(right_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Right) { // 逆
				hd = segments[right_segs[0]].len;
				vr = segments[(right_segs[0] + 1) % segments.size()].len;
				hm = segments[(right_segs[0] + 2) % segments.size()].len;
				vm = segments[(right_segs[0] + 3) % segments.size()].len;
				hu = segments[(right_segs[0] + 4) % segments.size()].len;
				vl = segments[(right_segs[0] + 5) % segments.size()].len;
			}
			else { assert(false); }
		}
		else { assert(false); }

		assert(hd == hu + hm && vl == vm + vr);
		this->area = vl * hu + vr * hm;
	}

	Shape shape() { return Shape::L; }

	void to_out_points() {
		this->out_points.clear();
		this->out_points.reserve(6);
		switch (this->rotation) {
		case Rotation::_0_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y + vl);
			this->out_points.emplace_back(this->lb_point.x + hu, this->lb_point.y + vl);
			this->out_points.emplace_back(this->lb_point.x + hu, this->lb_point.y + vr);
			this->out_points.emplace_back(this->lb_point.x + hd, this->lb_point.y + vr);
			this->out_points.emplace_back(this->lb_point.x + hd, this->lb_point.y);
			break;
		case Rotation::_90_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x + vl, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x + vl, this->lb_point.y - hu);
			this->out_points.emplace_back(this->lb_point.x + vr, this->lb_point.y - hu);
			this->out_points.emplace_back(this->lb_point.x + vr, this->lb_point.y - hd);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y - hd);
			break;
		case Rotation::_180_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y - vl);
			this->out_points.emplace_back(this->lb_point.x - hu, this->lb_point.y - vl);
			this->out_points.emplace_back(this->lb_point.x - hu, this->lb_point.y - vr);
			this->out_points.emplace_back(this->lb_point.x - hd, this->lb_point.y - vr);
			this->out_points.emplace_back(this->lb_point.x - hd, this->lb_point.y);
			break;
		case Rotation::_270_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x - vl, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x - vl, this->lb_point.y + hu);
			this->out_points.emplace_back(this->lb_point.x - vr, this->lb_point.y + hu);
			this->out_points.emplace_back(this->lb_point.x - vr, this->lb_point.y + hd);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y + hd);
			break;
		default:
			assert(false);
			break;
		}
	}
};

template<typename T>
struct TShape : public Polygon<T> {
	T hu, hl, hr, hd; // hu+hl+hr=hd
	T vlu, vld, vru, vrd; // vlu+vld=vru+vrd

	TShape(int id, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, points, segments) {
		std::vector<size_t> up_segs; up_segs.reserve(3);
		std::vector<size_t> down_segs; down_segs.reserve(3);
		std::vector<size_t> left_segs; left_segs.reserve(3);
		std::vector<size_t> right_segs; right_segs.reserve(3);

		for (size_t i = 0; i < segments.size(); ++i) {
			switch (segments[i].dir) {
			case Direction::Up:    up_segs.push_back(i);    break;
			case Direction::Down:  down_segs.push_back(i);  break;
			case Direction::Left:  left_segs.push_back(i);  break;
			case Direction::Right: right_segs.push_back(i); break;
			default:               assert(false);           break;
			}
		}

		if (up_segs.size() == 1) {
			assert(down_segs.size() == 3 && left_segs.size() == 2 && right_segs.size() == 2);
			if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Right) { // 顺
				hd = segments[up_segs[0]].len;
				vld = segments[(up_segs[0] + 1) % segments.size()].len;
				hl = segments[(up_segs[0] + 2) % segments.size()].len;
				vlu = segments[(up_segs[0] + 3) % segments.size()].len;
				hu = segments[(up_segs[0] + 4) % segments.size()].len;
				vru = segments[(up_segs[0] + 5) % segments.size()].len;
				hr = segments[(up_segs[0] + 6) % segments.size()].len;
				vrd = segments[(up_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Left) { // 逆
				hd = segments[up_segs[0]].len;
				vrd = segments[(up_segs[0] + 1) % segments.size()].len;
				hr = segments[(up_segs[0] + 2) % segments.size()].len;
				vru = segments[(up_segs[0] + 3) % segments.size()].len;
				hu = segments[(up_segs[0] + 4) % segments.size()].len;
				vlu = segments[(up_segs[0] + 5) % segments.size()].len;
				hl = segments[(up_segs[0] + 6) % segments.size()].len;
				vld = segments[(up_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (down_segs.size() == 1) {
			assert(up_segs.size() == 3 && left_segs.size() == 2 && right_segs.size() == 2);
			if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Left) { // 顺
				hd = segments[down_segs[0]].len;
				vld = segments[(down_segs[0] + 1) % segments.size()].len;
				hl = segments[(down_segs[0] + 2) % segments.size()].len;
				vlu = segments[(down_segs[0] + 3) % segments.size()].len;
				hu = segments[(down_segs[0] + 4) % segments.size()].len;
				vru = segments[(down_segs[0] + 5) % segments.size()].len;
				hr = segments[(down_segs[0] + 6) % segments.size()].len;
				vrd = segments[(down_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Right) { // 逆
				hd = segments[down_segs[0]].len;
				vrd = segments[(down_segs[0] + 1) % segments.size()].len;
				hr = segments[(down_segs[0] + 2) % segments.size()].len;
				vru = segments[(down_segs[0] + 3) % segments.size()].len;
				hu = segments[(down_segs[0] + 4) % segments.size()].len;
				vlu = segments[(down_segs[0] + 5) % segments.size()].len;
				hl = segments[(down_segs[0] + 6) % segments.size()].len;
				vld = segments[(down_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (left_segs.size() == 1) {
			assert(right_segs.size() == 3 && up_segs.size() == 2 && down_segs.size() == 2);
			if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Up) { // 顺
				hd = segments[left_segs[0]].len;
				vld = segments[(left_segs[0] + 1) % segments.size()].len;
				hl = segments[(left_segs[0] + 2) % segments.size()].len;
				vlu = segments[(left_segs[0] + 3) % segments.size()].len;
				hu = segments[(left_segs[0] + 4) % segments.size()].len;
				vru = segments[(left_segs[0] + 5) % segments.size()].len;
				hr = segments[(left_segs[0] + 6) % segments.size()].len;
				vrd = segments[(left_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Down) { // 逆
				hd = segments[left_segs[0]].len;
				vrd = segments[(left_segs[0] + 1) % segments.size()].len;
				hr = segments[(left_segs[0] + 2) % segments.size()].len;
				vru = segments[(left_segs[0] + 3) % segments.size()].len;
				hu = segments[(left_segs[0] + 4) % segments.size()].len;
				vlu = segments[(left_segs[0] + 5) % segments.size()].len;
				hl = segments[(left_segs[0] + 6) % segments.size()].len;
				vld = segments[(left_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (right_segs.size() == 1) {
			assert(left_segs.size() == 3 && up_segs.size() == 2 && down_segs.size() == 2);
			if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Down) { // 顺
				hd = segments[right_segs[0]].len;
				vld = segments[(right_segs[0] + 1) % segments.size()].len;
				hl = segments[(right_segs[0] + 2) % segments.size()].len;
				vlu = segments[(right_segs[0] + 3) % segments.size()].len;
				hu = segments[(right_segs[0] + 4) % segments.size()].len;
				vru = segments[(right_segs[0] + 5) % segments.size()].len;
				hr = segments[(right_segs[0] + 6) % segments.size()].len;
				vrd = segments[(right_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Up) { // 逆
				hd = segments[right_segs[0]].len;
				vrd = segments[(right_segs[0] + 1) % segments.size()].len;
				hr = segments[(right_segs[0] + 2) % segments.size()].len;
				vru = segments[(right_segs[0] + 3) % segments.size()].len;
				hu = segments[(right_segs[0] + 4) % segments.size()].len;
				vlu = segments[(right_segs[0] + 5) % segments.size()].len;
				hl = segments[(right_segs[0] + 6) % segments.size()].len;
				vld = segments[(right_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else { assert(false); }

		assert(hu + hl + hr == hd && vlu + vld == vru + vrd);
		this->area = hl * vld + hr * vrd + hu * (vru + vrd);
	}

	Shape shape() { return Shape::T; }

	void to_out_points() {
		this->out_points.clear();
		this->out_points.reserve(8);
		switch (this->rotation) {
		case Rotation::_0_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y + vld);
			this->out_points.emplace_back(this->lb_point.x + hl, this->lb_point.y + vld);
			this->out_points.emplace_back(this->lb_point.x + hl, this->lb_point.y + vld + vlu);
			this->out_points.emplace_back(this->lb_point.x + hl + hu, this->lb_point.y + vld + vlu);
			this->out_points.emplace_back(this->lb_point.x + hl + hu, this->lb_point.y + vrd);
			this->out_points.emplace_back(this->lb_point.x + hd, this->lb_point.y + vrd);
			this->out_points.emplace_back(this->lb_point.x + hd, this->lb_point.y);
			break;
		case Rotation::_90_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x + vld, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x + vld, this->lb_point.y - hl);
			this->out_points.emplace_back(this->lb_point.x + vld + vlu, this->lb_point.y - hl);
			this->out_points.emplace_back(this->lb_point.x + vld + vlu, this->lb_point.y - hl - hu);
			this->out_points.emplace_back(this->lb_point.x + vrd, this->lb_point.y - hl - hu);
			this->out_points.emplace_back(this->lb_point.x + vrd, this->lb_point.y - hd);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y - hd);
			break;
		case Rotation::_180_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y - vld);
			this->out_points.emplace_back(this->lb_point.x - hl, this->lb_point.y - vld);
			this->out_points.emplace_back(this->lb_point.x - hl, this->lb_point.y - vld - vlu);
			this->out_points.emplace_back(this->lb_point.x - hl - hu, this->lb_point.y - vld - vlu);
			this->out_points.emplace_back(this->lb_point.x - hl - hu, this->lb_point.y - vrd);
			this->out_points.emplace_back(this->lb_point.x - hd, this->lb_point.y - vrd);
			this->out_points.emplace_back(this->lb_point.x - hd, this->lb_point.y);
			break;
		case Rotation::_270_:
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x - vld, this->lb_point.y);
			this->out_points.emplace_back(this->lb_point.x - vld, this->lb_point.y + hl);
			this->out_points.emplace_back(this->lb_point.x - vld - vlu, this->lb_point.y + hl);
			this->out_points.emplace_back(this->lb_point.x - vld - vlu, this->lb_point.y + hl + hu);
			this->out_points.emplace_back(this->lb_point.x - vrd, this->lb_point.y + hl + hu);
			this->out_points.emplace_back(this->lb_point.x - vrd, this->lb_point.y + hd);
			this->out_points.emplace_back(this->lb_point.x, this->lb_point.y + hd);
			break;
		default:
			assert(false);
			break;
		}
	}
};

template<typename T>
struct Concave : public Polygon<T> {
	T hu, hl, hr, hd; // hu+hl+hr=hd
	T vlu, vld, vru, vrd; // vld-vlu=vrd-vru

	Concave(int id, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, points, segments) {
		std::vector<size_t> up_segs; up_segs.reserve(3);
		std::vector<size_t> down_segs; down_segs.reserve(3);
		std::vector<size_t> left_segs; left_segs.reserve(3);
		std::vector<size_t> right_segs; right_segs.reserve(3);

		for (size_t i = 0; i < segments.size(); ++i) {
			switch (segments[i].dir) {
			case Direction::Up:    up_segs.push_back(i);    break;
			case Direction::Down:  down_segs.push_back(i);  break;
			case Direction::Left:  left_segs.push_back(i);  break;
			case Direction::Right: right_segs.push_back(i); break;
			default:               assert(false);           break;
			}
		}

		if (up_segs.size() == 1) {
			assert(down_segs.size() == 3 && left_segs.size() == 2 && right_segs.size() == 2);
			if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Right) { // 顺
				hd = segments[up_segs[0]].len;
				vld = segments[(up_segs[0] + 1) % segments.size()].len;
				hl = segments[(up_segs[0] + 2) % segments.size()].len;
				vlu = segments[(up_segs[0] + 3) % segments.size()].len;
				hu = segments[(up_segs[0] + 4) % segments.size()].len;
				vru = segments[(up_segs[0] + 5) % segments.size()].len;
				hr = segments[(up_segs[0] + 6) % segments.size()].len;
				vrd = segments[(up_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Left) { // 逆
				hd = segments[up_segs[0]].len;
				vrd = segments[(up_segs[0] + 1) % segments.size()].len;
				hr = segments[(up_segs[0] + 2) % segments.size()].len;
				vru = segments[(up_segs[0] + 3) % segments.size()].len;
				hu = segments[(up_segs[0] + 4) % segments.size()].len;
				vlu = segments[(up_segs[0] + 5) % segments.size()].len;
				hl = segments[(up_segs[0] + 6) % segments.size()].len;
				vld = segments[(up_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (down_segs.size() == 1) {
			assert(up_segs.size() == 3 && left_segs.size() == 2 && right_segs.size() == 2);
			if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Left) { // 顺
				hd = segments[down_segs[0]].len;
				vld = segments[(down_segs[0] + 1) % segments.size()].len;
				hl = segments[(down_segs[0] + 2) % segments.size()].len;
				vlu = segments[(down_segs[0] + 3) % segments.size()].len;
				hu = segments[(down_segs[0] + 4) % segments.size()].len;
				vru = segments[(down_segs[0] + 5) % segments.size()].len;
				hr = segments[(down_segs[0] + 6) % segments.size()].len;
				vrd = segments[(down_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Right) { // 逆
				hd = segments[down_segs[0]].len;
				vrd = segments[(down_segs[0] + 1) % segments.size()].len;
				hr = segments[(down_segs[0] + 2) % segments.size()].len;
				vru = segments[(down_segs[0] + 3) % segments.size()].len;
				hu = segments[(down_segs[0] + 4) % segments.size()].len;
				vlu = segments[(down_segs[0] + 5) % segments.size()].len;
				hl = segments[(down_segs[0] + 6) % segments.size()].len;
				vld = segments[(down_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (left_segs.size() == 1) {
			assert(right_segs.size() == 3 && up_segs.size() == 2 && down_segs.size() == 2);
			if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Up) { // 顺
				hd = segments[left_segs[0]].len;
				vld = segments[(left_segs[0] + 1) % segments.size()].len;
				hl = segments[(left_segs[0] + 2) % segments.size()].len;
				vlu = segments[(left_segs[0] + 3) % segments.size()].len;
				hu = segments[(left_segs[0] + 4) % segments.size()].len;
				vru = segments[(left_segs[0] + 5) % segments.size()].len;
				hr = segments[(left_segs[0] + 6) % segments.size()].len;
				vrd = segments[(left_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Down) { // 逆
				hd = segments[left_segs[0]].len;
				vrd = segments[(left_segs[0] + 1) % segments.size()].len;
				hr = segments[(left_segs[0] + 2) % segments.size()].len;
				vru = segments[(left_segs[0] + 3) % segments.size()].len;
				hu = segments[(left_segs[0] + 4) % segments.size()].len;
				vlu = segments[(left_segs[0] + 5) % segments.size()].len;
				hl = segments[(left_segs[0] + 6) % segments.size()].len;
				vld = segments[(left_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else if (right_segs.size() == 1) {
			assert(left_segs.size() == 3 && up_segs.size() == 2 && down_segs.size() == 2);
			if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Down) { // 顺
				hd = segments[right_segs[0]].len;
				vld = segments[(right_segs[0] + 1) % segments.size()].len;
				hl = segments[(right_segs[0] + 2) % segments.size()].len;
				vlu = segments[(right_segs[0] + 3) % segments.size()].len;
				hu = segments[(right_segs[0] + 4) % segments.size()].len;
				vru = segments[(right_segs[0] + 5) % segments.size()].len;
				hr = segments[(right_segs[0] + 6) % segments.size()].len;
				vrd = segments[(right_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Up) { // 逆
				hd = segments[right_segs[0]].len;
				vrd = segments[(right_segs[0] + 1) % segments.size()].len;
				hr = segments[(right_segs[0] + 2) % segments.size()].len;
				vru = segments[(right_segs[0] + 3) % segments.size()].len;
				hu = segments[(right_segs[0] + 4) % segments.size()].len;
				vlu = segments[(right_segs[0] + 5) % segments.size()].len;
				hl = segments[(right_segs[0] + 6) % segments.size()].len;
				vld = segments[(right_segs[0] + 7) % segments.size()].len;
			}
			else { assert(false); }
		}
		else { assert(false); }

		assert(hu + hl + hr == hd && vld - vlu == vrd - vru);
		this->area = hl * vld + hr * vrd + hu * (vrd - vru);
	}

	Shape shape() { return Shape::C; }

	void to_out_points() {
		this->out_points.clear();
		this->out_points.reserve(8);
		assert(this->rotation == Rotation::_0_);
		this->out_points.emplace_back(this->lb_point.x, this->lb_point.y);
		this->out_points.emplace_back(this->lb_point.x, this->lb_point.y + vld);
		this->out_points.emplace_back(this->lb_point.x + hl, this->lb_point.y + vld);
		this->out_points.emplace_back(this->lb_point.x + hl, this->lb_point.y + vld - vlu);
		this->out_points.emplace_back(this->lb_point.x + hl + hu, this->lb_point.y + vld - vlu);
		this->out_points.emplace_back(this->lb_point.x + hl + hu, this->lb_point.y + vrd);
		this->out_points.emplace_back(this->lb_point.x + hd, this->lb_point.y + vrd);
		this->out_points.emplace_back(this->lb_point.x + hd, this->lb_point.y);
	}
};

template<typename T>
struct SkyLineNode {
	T x, y;
	T width;
};

using point_t = Point<coord_t>;

using segment_t = Segment<coord_t>;

using polygon_t = Polygon<coord_t>;

using rect_t = Rect<coord_t>;

using lshape_t = LShape<coord_t>;

using tshape_t = TShape<coord_t>;

using concave_t = Concave<coord_t>;

using polygon_ptr = std::shared_ptr<polygon_t>;

using rect_ptr = std::shared_ptr<rect_t>;

using lshape_ptr = std::shared_ptr<lshape_t>;

using tshape_ptr = std::shared_ptr<tshape_t>;

using concave_ptr = std::shared_ptr<concave_t>;

using skylinenode_t = SkyLineNode<coord_t>;

using skyline_t = std::vector<skylinenode_t>;

#endif // SMARTMPW_DATA_HPP
