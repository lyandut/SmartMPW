//
// @author   liyan
// @contact  lyan_dut@outlook.com
//
#ifndef SMARTMPW_UTILS_HPP
#define SMARTMPW_UTILS_HPP

#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace utils {

	using namespace std;

	class Date {
	public:
		// 返回表示日期格式的字符串，年月日
		static string to_short_str() {
			time_t now = time(nullptr);
			char mbstr[100];
			strftime(mbstr, sizeof(mbstr), "%y%m%d", localtime(&now));
			return mbstr;
			
			//ostringstream os;
			//os << put_time(localtime(&now), "%y%m%d");
			//return os.str();
		}
		// 返回表示日期格式的字符串，年月日时分秒
		static string to_long_str() {
			time_t now = time(nullptr);
			char mbstr[100];
			strftime(mbstr, sizeof(mbstr), "%y%m%d%H%M%S", localtime(&now));
			return mbstr;
			
			//ostringstream os;
			//os << put_time(localtime(&now), "%y-%m-%e-%H_%M_%S");
			//os << put_time(localtime(&now), "%y%m%d%H%M%S");
			//return os.str();
		}
	};

	static void split_filename(const string &str, string &path, string &file) {
		size_t found1 = str.find_last_of("/\\");
		size_t found2 = str.find_last_of(".");
		path = str.substr(0, found1 + 1);
		file = str.substr(found1 + 1, found2 - found1 - 1);
	}
}

#endif // SMARTMPW_UTILS_HPP
