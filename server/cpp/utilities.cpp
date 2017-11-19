#include "utilities.h"
#include <sstream>
using namespace std;

string intToStr(int input)
{
	stringstream ss;
	ss << input;
	return ss.str();
}

int strToInt(const std::string& input)
{
	stringstream ss(input);
	int x = 0;
	ss >> x;
	return x;
}
