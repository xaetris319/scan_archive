#include <ctime>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <vector>

#include "messages.hpp"

std::string add_leading_zeros(const std::string& input, const int& req_length) {
	std::string result = input;
	size_t length = input.length();
	for (size_t i = length; i < req_length; i++) {
		result = "0" + result;
	}
	return result;
}

std::string tm_to_string(const std::tm& time) {
	std::string year = std::to_string(time.tm_year + 1900);
	std::string month = add_leading_zeros(std::to_string(time.tm_mon + 1), 2);
	std::string day = add_leading_zeros(std::to_string(time.tm_mday), 2);
	std::string hour = add_leading_zeros(std::to_string(time.tm_hour), 2);
	std::string minute = add_leading_zeros(std::to_string(time.tm_min), 2);
	std::string second = add_leading_zeros(std::to_string(time.tm_sec), 2);
	return year + month + day + "_" + hour + minute + second;
}

void get_timestamp(std::string& timestamp) {
	struct tm time_struct;
	__time64_t local_time;
	time(&local_time);
	errno_t err = _localtime64_s(&time_struct, &local_time);
	if (err) {
		exit(-1);
	}
	timestamp = tm_to_string(time_struct);
}

void get_directories(std::string& workdir, std::string& savetodir) {
	std::string timestamp;
	get_timestamp(timestamp);
	char* appdata_value;
	size_t len;
	errno_t err = _dupenv_s(&appdata_value, &len, "APPDATA");
	if (err || appdata_value == NULL) {
		exit(1);
	}
	std::string appdata = appdata_value;
	workdir = appdata + "\\SAP\\ScanArchive\\";
	savetodir = appdata + "\\SAP\\ScanArchive\\" + timestamp + "\\";
}

std::string exec(const char* cmd) {
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe) {
		throw std::runtime_error("_popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

int read_file(const std::string& file_location, std::string& content) {
	std::ifstream in;
	in.open(file_location);
	if (in.is_open()) {
		in >> content;
		in.close();
		return 0;
	}
	return -1;
}

void write_file(const std::string& file_location, const std::string& content) {
	std::ofstream out;
	out.open(file_location);
	if (!out.is_open()) {
		exit(1);
	}
	out << content;
	out.close();
	return;
}

int separate_profile_content(const std::string& profile_content, std::string& profilename, std::string& fileextension) {
	std::string sep = ",";
	size_t pos_start = 0, pos_end, sep_len = sep.length();
	std::string token;
	std::vector<std::string> segments;
	while ((pos_end = profile_content.find(sep, pos_start)) != std::string::npos) {
		token = profile_content.substr(pos_start, pos_end - pos_start);
		pos_start = pos_end + sep_len;
		segments.push_back(token);
	}
	if (segments.size() < 2) {
		return -1;
	}
	profilename = segments[0];
	fileextension = segments[1];
	return 0;
}

void extract_profile_content(const std::string& profile_location, const std::string& protocol_location, std::string& profile_name, std::string& file_extension) {
	std::string profile_content;
	std::string output_content;
	if (read_file(profile_location, profile_content) == -1 || profile_content == "") {
		output_content = "1;" + messages::ERROR_NO_PROFILE;
		write_file(protocol_location, output_content);
		exit(1);
	}
	if (separate_profile_content(profile_content, profile_name, file_extension) == -1) {
		output_content = "1;" + messages::ERROR_PROFILE_WRONG_CONTENT;
		write_file(protocol_location, output_content);
		exit(1);
	}
}

void run_naps2(const std::string& protocol_location, const std::string& saveto_dir, const std::string& profile_name, const std::string& file_extension) {
	std::string naps2_location = "\"C:\\Program Files (x86)\\NAPS2\\naps2.console\"";
	std::string profile_option = "-p " + profile_name;
	std::string output_option = "-o " + saveto_dir + "\\DOC$(nnn)." + file_extension;
	std::string other_options = "--FORCE --SPLIT";
	std::string naps2_command = naps2_location + " " + profile_option + " " + output_option + " " + other_options;
	std::string console_return = exec(naps2_command.c_str());
	std::string output_content;
	if (!console_return.empty()) {
		output_content = "1;" + console_return;
		write_file(protocol_location, output_content);
		exit(1);
	}
}

void save_images_list(const std::string& protocol_location, const std::string& path_location, const std::string& saveto_dir) {
	std::string output_content;
	for (const auto& entry : std::filesystem::directory_iterator(saveto_dir)) {
		if (!output_content.empty()) {
			output_content = output_content + ";";
		}
		output_content = output_content + entry.path().string();
	}
	if (output_content.empty()) {
		output_content = "1;" + messages::ERROR_NO_FILES;
		write_file(protocol_location, output_content);
		exit(1);
	}
	write_file(path_location, output_content);
}

int main() {
	std::string work_dir;
	std::string saveto_dir;
	get_directories(work_dir, saveto_dir);
	std::string profile_location = work_dir + "\\profile.txt";
	std::string protocol_location = work_dir + "\\protocol.txt";
	std::string path_location = work_dir + "\\path.txt";
	std::string profile_name;
	std::string file_extension;
	extract_profile_content(protocol_location, profile_location, profile_name, file_extension);
	run_naps2(protocol_location, saveto_dir, profile_name, file_extension);
	save_images_list(protocol_location, path_location, saveto_dir);
	getchar();
	return 0;
}
