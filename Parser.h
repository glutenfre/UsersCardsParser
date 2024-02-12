#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>

using Value = std::variant<std::nullptr_t, double, std::string>;

std::string GetDataFromFile(std::string& file_name);
bool PutDataIntoFile(std::string file_name, std::vector<std::unordered_map<std::string, Value>>& users_data);

class Parser {
public:
	Parser(std::string raw_str);

	void Parse();

	std::vector< std::unordered_map<std::string, Value> >& GetUsersData();
private:
	bool CardIsOlder(std::string& date, int year);
	bool CheckPassword(std::string& password);
	bool CheckData(std::unordered_map<std::string, Value>& card);

	bool IsNumber(std::string val);
	bool SetData(std::unordered_map<std::string, Value>* card);
private:
	std::vector<std::unordered_map<std::string, Value>> users_data_;
	std::string raw_str_;
	size_t cur_pos_ = 0;
};