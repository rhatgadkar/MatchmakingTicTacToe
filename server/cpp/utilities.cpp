#include "utilities.h"
#include <sstream>
using namespace std;

string intToStr(int input)
{
	stringstream ss;
	ss << input;
	return ss.str();
}
