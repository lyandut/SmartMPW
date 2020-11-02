//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMARTMPW_CHECKER_DATA_HPP
#define SMARTMPW_CHECKER_DATA_HPP

#include <string>
#include <vector>
#include <memory>
#include <cassert>

#include "Visualizer.hpp"

namespace vis = utils_visualize;

enum Shape { R, L, T, C };

enum Direction { Left, Right, Up, Down };

template<typename T>
struct Point {
	T x, y;
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

template<typename> struct Polygon;
template<typename T>
bool operator==(const Polygon<T> &lhs, const Polygon<T> &rhs) {
	return typeid(lhs) == typeid(rhs) && lhs.equal(rhs);
}
template<typename T>
bool operator!=(const Polygon<T> &lhs, const Polygon<T> &rhs) {
	return !(lhs == rhs);
}

template<typename T>
struct Polygon {
	const int id;
	const std::string str;
	vis::bg_ring_t ring;
	T area;

	friend bool operator==<T>(const Polygon&, const Polygon&);
	//template<typename T>
	//friend bool operator==(const Polygon&, const Polygon&);

	Polygon(int id_, const std::string &str_, const std::vector<Point<T>> &points) : id(id_), str(str_) {
		for (auto &point : points) { vis::bg::append(ring, vis::bg_point_t(point.x, point.y)); }
		area = abs(vis::bg::area(ring));
	}

	virtual Shape shape() = 0;
	virtual bool equal(const Polygon<T>&rhs) const = 0;
};

//template<typename T>
//bool operator==(const Polygon<T> &lhs, const Polygon<T> &rhs) {
//	return typeid(lhs) == typeid(rhs) && lhs.equal(rhs);
//}
//template<typename T>
//bool operator!=(const Polygon<T> &lhs, const Polygon<T> &rhs) {
//	return !(lhs == rhs);
//}

template<typename T>
struct Rect : public Polygon<T> {
	T width, height;

	Rect(int id, const std::string &str, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, str, points) {
		assert(segments.size() == 4);
		width = segments[0].len;
		height = segments[1].len;
		assert(width == segments[2].len && height == segments[3].len);
	}

	Shape shape() { return Shape::R; }

	bool equal(const Polygon<T> &rhs) const {
		auto r = dynamic_cast<const Rect<T>&>(rhs);
		return this->id == rhs.id &&
			(width == r.width && height == r.height || width == r.height && height == r.width);
	}
};

template<typename T>
struct LShape : public Polygon<T> {
	T hd, hm, hu; // hd=hm+hu
	T vl, vm, vr; // vl=vm+vr

	LShape(int id, const std::string &str, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, str, points) {
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
			if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Up) { // Ë³
				hd = segments[left_segs[0]].len;
				vl = segments[(left_segs[0] + 1) % segments.size()].len;
				hu = segments[(left_segs[0] + 2) % segments.size()].len;
				vm = segments[(left_segs[0] + 3) % segments.size()].len;
				hm = segments[(left_segs[0] + 4) % segments.size()].len;
				vr = segments[(left_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Left) { // Äæ
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
			if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Right) { // Ë³
				hd = segments[up_segs[0]].len;
				vl = segments[(up_segs[0] + 1) % segments.size()].len;
				hu = segments[(up_segs[0] + 2) % segments.size()].len;
				vm = segments[(up_segs[0] + 3) % segments.size()].len;
				hm = segments[(up_segs[0] + 4) % segments.size()].len;
				vr = segments[(up_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Up) { // Äæ
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
			if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Left) { // Ë³
				hd = segments[down_segs[0]].len;
				vl = segments[(down_segs[0] + 1) % segments.size()].len;
				hu = segments[(down_segs[0] + 2) % segments.size()].len;
				vm = segments[(down_segs[0] + 3) % segments.size()].len;
				hm = segments[(down_segs[0] + 4) % segments.size()].len;
				vr = segments[(down_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Down) { // Äæ
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
			if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Down) { // Ë³
				hd = segments[right_segs[0]].len;
				vl = segments[(right_segs[0] + 1) % segments.size()].len;
				hu = segments[(right_segs[0] + 2) % segments.size()].len;
				vm = segments[(right_segs[0] + 3) % segments.size()].len;
				hm = segments[(right_segs[0] + 4) % segments.size()].len;
				vr = segments[(right_segs[0] + 5) % segments.size()].len;
			}
			else if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Right) { // Äæ
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
	}

	Shape shape() { return Shape::L; }

	bool equal(const Polygon<T> &rhs) const {
		auto r = dynamic_cast<const LShape<T>&>(rhs);
		return this->id == rhs.id
			&& hd == r.hd && hm == r.hm && hu == r.hu
			&& vl == r.vl && vm == r.vm && vr == r.vr;
	}
};

template<typename T>
struct TShape : public Polygon<T> {
	T hu, hl, hr, hd; // hu+hl+hr=hd
	T vlu, vld, vru, vrd; // vlu+vld=vru+vrd

	TShape(int id, const std::string &str, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, str, points) {
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
			if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Right) { // Ë³
				hd = segments[up_segs[0]].len;
				vld = segments[(up_segs[0] + 1) % segments.size()].len;
				hl = segments[(up_segs[0] + 2) % segments.size()].len;
				vlu = segments[(up_segs[0] + 3) % segments.size()].len;
				hu = segments[(up_segs[0] + 4) % segments.size()].len;
				vru = segments[(up_segs[0] + 5) % segments.size()].len;
				hr = segments[(up_segs[0] + 6) % segments.size()].len;
				vrd = segments[(up_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Left) { // Äæ
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
			if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Left) { // Ë³
				hd = segments[down_segs[0]].len;
				vld = segments[(down_segs[0] + 1) % segments.size()].len;
				hl = segments[(down_segs[0] + 2) % segments.size()].len;
				vlu = segments[(down_segs[0] + 3) % segments.size()].len;
				hu = segments[(down_segs[0] + 4) % segments.size()].len;
				vru = segments[(down_segs[0] + 5) % segments.size()].len;
				hr = segments[(down_segs[0] + 6) % segments.size()].len;
				vrd = segments[(down_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Right) { // Äæ
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
			if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Up) { // Ë³
				hd = segments[left_segs[0]].len;
				vld = segments[(left_segs[0] + 1) % segments.size()].len;
				hl = segments[(left_segs[0] + 2) % segments.size()].len;
				vlu = segments[(left_segs[0] + 3) % segments.size()].len;
				hu = segments[(left_segs[0] + 4) % segments.size()].len;
				vru = segments[(left_segs[0] + 5) % segments.size()].len;
				hr = segments[(left_segs[0] + 6) % segments.size()].len;
				vrd = segments[(left_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Down) { // Äæ
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
			if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Down) { // Ë³
				hd = segments[right_segs[0]].len;
				vld = segments[(right_segs[0] + 1) % segments.size()].len;
				hl = segments[(right_segs[0] + 2) % segments.size()].len;
				vlu = segments[(right_segs[0] + 3) % segments.size()].len;
				hu = segments[(right_segs[0] + 4) % segments.size()].len;
				vru = segments[(right_segs[0] + 5) % segments.size()].len;
				hr = segments[(right_segs[0] + 6) % segments.size()].len;
				vrd = segments[(right_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Up) { // Äæ
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
	}

	Shape shape() { return Shape::T; }

	bool equal(const Polygon<T> &rhs) const {
		auto r = dynamic_cast<const TShape<T>&>(rhs);
		return  this->id == rhs.id
			&& hu == r.hu && hl == r.hl && hr == r.hr && hd == r.hd
			&& vlu == r.vlu && vld == r.vld && vru == r.vru && vrd == r.vrd;
	}
};

template<typename T>
struct Concave : public Polygon<T> {
	T hu, hl, hr, hd; // hu+hl+hr=hd
	T vlu, vld, vru, vrd; // vld-vlu=vrd-vru

	Concave(int id, const std::string &str, const std::vector<Point<T>> &points, const std::vector<Segment<T>> &segments)
		: Polygon<T>(id, str, points) {
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
			if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Right) { // Ë³
				hd = segments[up_segs[0]].len;
				vld = segments[(up_segs[0] + 1) % segments.size()].len;
				hl = segments[(up_segs[0] + 2) % segments.size()].len;
				vlu = segments[(up_segs[0] + 3) % segments.size()].len;
				hu = segments[(up_segs[0] + 4) % segments.size()].len;
				vru = segments[(up_segs[0] + 5) % segments.size()].len;
				hr = segments[(up_segs[0] + 6) % segments.size()].len;
				vrd = segments[(up_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(up_segs[0] + 1) % segments.size()].dir == Direction::Left) { // Äæ
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
			if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Left) { // Ë³
				hd = segments[down_segs[0]].len;
				vld = segments[(down_segs[0] + 1) % segments.size()].len;
				hl = segments[(down_segs[0] + 2) % segments.size()].len;
				vlu = segments[(down_segs[0] + 3) % segments.size()].len;
				hu = segments[(down_segs[0] + 4) % segments.size()].len;
				vru = segments[(down_segs[0] + 5) % segments.size()].len;
				hr = segments[(down_segs[0] + 6) % segments.size()].len;
				vrd = segments[(down_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(down_segs[0] + 1) % segments.size()].dir == Direction::Right) { // Äæ
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
			if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Up) { // Ë³
				hd = segments[left_segs[0]].len;
				vld = segments[(left_segs[0] + 1) % segments.size()].len;
				hl = segments[(left_segs[0] + 2) % segments.size()].len;
				vlu = segments[(left_segs[0] + 3) % segments.size()].len;
				hu = segments[(left_segs[0] + 4) % segments.size()].len;
				vru = segments[(left_segs[0] + 5) % segments.size()].len;
				hr = segments[(left_segs[0] + 6) % segments.size()].len;
				vrd = segments[(left_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(left_segs[0] + 1) % segments.size()].dir == Direction::Down) { // Äæ
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
			if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Down) { // Ë³
				hd = segments[right_segs[0]].len;
				vld = segments[(right_segs[0] + 1) % segments.size()].len;
				hl = segments[(right_segs[0] + 2) % segments.size()].len;
				vlu = segments[(right_segs[0] + 3) % segments.size()].len;
				hu = segments[(right_segs[0] + 4) % segments.size()].len;
				vru = segments[(right_segs[0] + 5) % segments.size()].len;
				hr = segments[(right_segs[0] + 6) % segments.size()].len;
				vrd = segments[(right_segs[0] + 7) % segments.size()].len;
			}
			else if (segments[(right_segs[0] + 1) % segments.size()].dir == Direction::Up) { // Äæ
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
	}

	Shape shape() { return Shape::C; }

	bool equal(const Polygon<T>& rhs) const {
		auto r = dynamic_cast<const Concave<T>&>(rhs);
		return  this->id == rhs.id
			&& hu == r.hu && hl == r.hl && hr == r.hr && hd == r.hd
			&& vlu == r.vlu && vld == r.vld && vru == r.vru && vrd == r.vrd;
	}
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

#endif // SMARTMPW_CHECKER_DATA_HPP
