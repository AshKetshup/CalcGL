#include "functionReader.hpp"
#include <fstream>

bool reader::isFunctionFile(const string fn) {
	string ext = fn.substr(fn.find_last_of(".") + 1);
	return ext == "func" || ext == "function";
}

vector<string> reader::readFunctionFile(const string fn) {
	vector<string> strList;
	
	if (!isFunctionFile) {
		cerr << "File is not \".func\" or \".function\" extension\n";
		return strList;
	}

	ifstream file("input.txt");
	
	string s;
	while (getline(file, s))
		strList.push_back(s);

	file.close();

	return strList;
}
