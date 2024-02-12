#include "Parser.h"
#include <fstream>
#include <iterator>
#include <locale>

std::string GetDataFromFile(std::string& file_name)
{
	std::ifstream is(file_name);
	std::string res(std::istreambuf_iterator<char>{is},
		std::istreambuf_iterator<char>{});
	return res;
}

bool PutDataIntoFile(std::string file_name, std::vector<std::unordered_map<std::string, Value>>& users_data)
{
	std::ofstream os(file_name);
	if (os.good()) {
		for (const auto& m : users_data) {
			int i = 0;
			os << "[\n\t{\n";
			for (const auto& pair : m) {
				if (i != 0) {
					os << ",\n";
				}
				os << "\t\t";
				os << "\"" << pair.first << "\": ";
				if (std::holds_alternative<double>(pair.second)) {
					os << std::to_string(std::get<double>(pair.second));
				}
				else if (std::holds_alternative<std::string>(pair.second)) {
					os << std::get<std::string>(pair.second);
				}
				else {
					os << "ERROR";
				}
				i++;
			}
			os << "\n\t}\n]\n";
		}
		return true;
	}
	return false;
}

Parser::Parser(std::string raw_str) :
	raw_str_(std::move(raw_str))
{
}

bool Parser::CardIsOlder(std::string& date, int year)
{
	size_t year_delim = date.find_last_of('.');
	int card_year = std::stoi(date.substr(year_delim + 1, date.size() - year_delim));
	return (card_year < year);
}

bool Parser::CheckPassword(std::string& password)
{
	bool specsimvol = false;
	bool low_register = false;
	bool high_register = false;
	bool cyrilic = false;
	for (const auto& c : password) {
		if ((c == 33) 
			|| ((c >= 35) && (c <= 47)) 
			|| ((c >= 58) && (c <= 64))
			|| ((c >= 91) && (c <= 96))
			|| ((c >= 123) && (c <= 126))) {
			specsimvol = true;
		}
		else if ((c >= 65) && (c <= 90)) {
			high_register = true;
		}
		else if ((c >= 97) && (c <= 122)) {
			low_register = true;
		}
		else if (((c >= 128) && (c <= 175))
		|| ((c >= 224) && (c <= 243))){
			cyrilic = true;
		}
	}
	return (specsimvol& low_register& high_register&(!cyrilic));
}

bool Parser::CheckData(std::unordered_map<std::string, Value>& card)
{
	const auto end_it = card.end();
	return ((card.find("UserID") != end_it) 
		&& (card.find("UserName") != end_it) 
		&& (card.find("UserSurname") != end_it) 
		&& (card.find("RegistrationDate") != end_it) 
		&& (card.find("Password") != end_it)
		&& (card.size() == 5));
}

bool Parser::SetData(std::unordered_map<std::string, Value>* card)
{
	size_t end_key = raw_str_.find('"', cur_pos_);
	size_t start_dict = raw_str_.find('{', cur_pos_);
	size_t start_array = raw_str_.find('[', cur_pos_);
	size_t end_dict = raw_str_.find('}', cur_pos_);
	size_t end_array = raw_str_.find(']', cur_pos_);

	if ((end_key != std::string::npos)
		&& (end_dict != std::string::npos) 
		&& (end_array != std::string::npos) 
		&& (end_key < end_dict) 
		&& (end_key < end_array)) {

		if (((start_dict != std::string::npos)
			&& (start_array != std::string::npos)
			&& (end_key < start_dict)
			&& (end_key < start_array)
			&& (start_dict > end_dict)
			&& (start_array > end_array)) 
			|| ((start_dict == std::string::npos)
				&& (start_array == std::string::npos))) {

			//Getting key
			std::string key = raw_str_.substr(cur_pos_, end_key - cur_pos_);
			cur_pos_ = end_key + 1;
			if (raw_str_[cur_pos_] == ':') {
				// Getting value
				Value v;
				cur_pos_++;
				std::string val;
				size_t end_val = raw_str_.find(',', cur_pos_);
				if (end_val != std::string::npos) {
					if (end_val < end_dict) {
						val = raw_str_.substr(cur_pos_, end_val - cur_pos_);
						if (val[0] != '\"') {
							v = std::stod(val);
						}
						else {
							v = val;
						}
					}
					else {
						val = raw_str_.substr(cur_pos_, end_dict - cur_pos_);
						if (val[0] != '\"') {
							v = std::stod(val);
						}
						else {
							v = val;
						}
					}
					if (((key == "Password") && (CheckPassword(val))) 
						|| ((key == "RegistrationDate") && (!CardIsOlder(val, 2023)))
						|| ((key != "Password") && (key != "RegistrationDate"))) {
						card->insert(std::make_pair(key, v));
						cur_pos_ += val.size();
						return true;
					}
				}
				else if (end_dict != std::string::npos) {
					val = raw_str_.substr(cur_pos_, end_dict - cur_pos_);
					if (val[0] != '\"') {
						v = std::stod(val);
					}
					else {
						v = val;
					}
					card->insert(std::make_pair(key, v));
					cur_pos_+= val.size();
					return true;
				}
			}
		}
	}
	return false;
}

void Parser::Parse()
{
	if (!raw_str_.empty()) {
		while (cur_pos_ < raw_str_.size() - 3) {
			if (raw_str_[cur_pos_] == L'[') {
				cur_pos_++;
				if (raw_str_[cur_pos_] == L'{') {
					cur_pos_++;
					if (raw_str_[cur_pos_] == L'\"') {
						cur_pos_++;
						std::unordered_map<std::string, Value> card;
						if (SetData(&card)) {
							bool f = true;
							while (raw_str_[cur_pos_] == ',') {
								cur_pos_+=2;
								f *= SetData(&card);
							}
							if ((f) && (CheckData(card))) {
								if ((CheckPassword(std::get<std::string>(card["Password"]))) 
									&& (!CardIsOlder(std::get<std::string>(card["RegistrationDate"]), 2023))) {
									users_data_.push_back(card);
								}
							}
						}
					}
				}
			}
			else {
				cur_pos_++;
			}
		}
	}
}

std::vector<std::unordered_map<std::string, Value>>& Parser::GetUsersData()
{
	return users_data_;
}



