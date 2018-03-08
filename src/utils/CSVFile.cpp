#include "CSVFile.h"
#include <fstream>
#include <assert.h>

// ------------------------------------------------------
// CSVLine
// ------------------------------------------------------
TextLine::TextLine(const std::string& str) : _delimiter(',') {
	set(str);
}

void TextLine::set(const std::string& str,const char delimiter) {
	_content = str;
	_num_delimiters = 0;
	_delimiter = delimiter;
	for ( int i = 0; i < _content.length();++i ) {
		if ( _content[i] == delimiter ) {
			++_num_delimiters;
		}
	}	
}

int TextLine::num_tokens() const {
	return _num_delimiters + 1;
}

void TextLine::print() const {
	printf("%s\n", _content.c_str());
}
// ------------------------------------------------------
// find pos in string for field index
// ------------------------------------------------------
int TextLine::find_pos(int field_index) const {
	if ( field_index == 0 ) {
		return 0;
	}
	int cnt = 0;
	for ( int i = 0; i < _content.length();++i ) {
		if ( _content[i] == _delimiter ) {
			++cnt;
		}
		if ( cnt == field_index ) {
			return i + 1;
		}
	}
	return -1;
}

// ------------------------------------------------------
// get int
// ------------------------------------------------------
int TextLine::get_int(int index) const {
	int idx = find_pos(index);
	if ( idx != -1 ) {
		int v;
		int npos = find_pos(index+1);
		if ( npos != -1 ) {
			std::string str = _content.substr(idx,npos-idx-1);		
			sscanf(str.c_str(),"%d",&v);
		}
		else {
			std::string str = _content.substr(idx);		
			sscanf(str.c_str(),"%d",&v);
		}
		return v;
	}
	return -1;
}

// ------------------------------------------------------
// get float
// ------------------------------------------------------
float TextLine::get_float(int index) const {
	int idx = find_pos(index);
	if (idx != -1) {
		float v;
		int npos = find_pos(index + 1);
		if (npos != -1) {
			std::string str = _content.substr(idx, npos - idx - 1);
			sscanf(str.c_str(), "%g", &v);
		}
		else {
			std::string str = _content.substr(idx);
			sscanf(str.c_str(), "%g", &v);
		}
		return v;
	}
	return -1;
}

// ------------------------------------------------------
// get string
// ------------------------------------------------------
int TextLine::get_string(int index,char* dest) const {
	int idx = find_pos(index);
	if ( idx != -1 ) {
		int npos = find_pos(index+1);
		if ( npos != -1 ) {			
			std::string str = _content.substr(idx,npos-idx-1);		
			strcpy(dest,str.c_str());
			return str.length();
		}
		else {
			std::string str = _content.substr(idx);		
			strcpy(dest,str.c_str());
			return str.length();
		}
	}
	return 0;
}

// ------------------------------------------------------
// get char
// ------------------------------------------------------
const char TextLine::get_char(int index) const {
	int idx = find_pos(index);
	if ( idx != -1 ) {
		return _content[idx];
	}
	return '-';
}

// ------------------------------------------------------
// get bool
// ------------------------------------------------------
bool TextLine::get_bool(int index) const {
	int idx = find_pos(index);
	if ( idx != -1 ) {
		char c = _content[idx];
		if ( c == 'N' || c == 'n' ) {
			return false;
		}
		return true;		
	}
	return false;
}


// ------------------------------------------------------
// CSVFile
// ------------------------------------------------------
CSVFile::CSVFile() {}


CSVFile::~CSVFile() {}

// ------------------------------------------------------
// load
// ------------------------------------------------------
bool CSVFile::load(const char* fileName,const char* directory) {
	char buffer[256];
	sprintf(buffer,"%s\\%s",directory,fileName);
	std::string line;
	std::ifstream myfile(buffer);
	_lines.clear();
	if (myfile.is_open()) {
		while (std::getline(myfile, line)) {
			if (line.find("#") == std::string::npos && line.find(",") != std::string::npos) {
				_lines.push_back(TextLine(line));					
			}
		}
		myfile.close();
		return true;
	}
	return false;
}

// ------------------------------------------------------
// get line
// ------------------------------------------------------
const TextLine& CSVFile::get(int index) const {
	assert(index >= 0 && index < _lines.size());
	return _lines[index];
}

// ------------------------------------------------------
// size
// ------------------------------------------------------
const size_t CSVFile::size() const {
	return _lines.size();
}