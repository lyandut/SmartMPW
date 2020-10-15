//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#pragma once

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace utils {

	using namespace std;

	class Date {
	public:
		// 返回表示日期格式的字符串，年月日
		static const string to_short_str() {
			ostringstream os;
			time_t now = time(0);
			os << put_time(localtime(&now), "%y%m%d");
			return os.str();
		}
		// 返回表示日期格式的字符串，年月日时分秒
		static const string to_long_str() {
			ostringstream os;
			time_t now = time(0);
			//os << put_time(localtime(&now), "%y-%m-%e-%H_%M_%S");
			os << put_time(localtime(&now), "%y%m%d%H%M%S");
			return os.str();
		}
	};

}
