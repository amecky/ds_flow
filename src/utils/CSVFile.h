#pragma once
#include <string>
#include <vector>

// ------------------------------------------------------
// TextLine
// ------------------------------------------------------
class TextLine {

public:
	TextLine() {}
	TextLine(const std::string& str);
	~TextLine() {}
	void set(const std::string& str,const char delimiter = ',');
	int find_pos(int field_index) const;
	int get_int(int index) const;
	float get_float(int index) const;
	const char get_char(int index) const;
	bool get_bool(int index) const;
	int get_string(int index,char* dest) const;
	int num_tokens() const;
	void print() const;
private:
	int _num_delimiters;
	std::string _content;
	char _delimiter;
};

// ------------------------------------------------------
// CSVFile
// ------------------------------------------------------
class CSVFile {

typedef std::vector<TextLine> Lines;

public:
	CSVFile();
	~CSVFile();
	bool load(const char* fileName,const char* directory);
	const TextLine& get(int index) const;
	const size_t size() const;
private:
	Lines _lines;
};

