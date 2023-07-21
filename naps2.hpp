
class naps2
{
public:
    static std::vector<std::string> perform_scanning(const std::string &work_dir,
                                                     const std::string &profile_name, const std::string &file_extension,
                                                     std::string &console_output)
    {
        std::string saveto_dir = work_dir + get_timestamp() + "\\";
        std::string naps2_location = "\"C:\\Program Files (x86)\\NAPS2\\naps2.console\"";
        std::string profile_option = " -p " + profile_name;
        std::string output_option = " -o " + saveto_dir + "DOC$(nnn)." + file_extension;
        std::string other_options = " --FORCE --SPLIT";
        std::string naps2_command = naps2_location + profile_option + output_option + other_options;
        try
        {
            console_output = exec(naps2_command.c_str());
        }
        catch (const std::runtime_error &e)
        {
            console_output = e.what();
        }
        return make_images_list(saveto_dir);
    }

private:
    naps2() {}
    ~naps2() {}

    static std::string add_leading_zeros(const std::string &input, const int &req_length)
    {
        std::string result = input;
        size_t length = input.length();
        for (size_t i = length; i < req_length; i++)
        {
            result = "0" + result;
        }
        return result;
    }

    static std::string tm_to_string(const std::tm &time)
    {
        std::string year = std::to_string(time.tm_year + 1900);
        std::string month = add_leading_zeros(std::to_string(time.tm_mon + 1), 2);
        std::string day = add_leading_zeros(std::to_string(time.tm_mday), 2);
        std::string hour = add_leading_zeros(std::to_string(time.tm_hour), 2);
        std::string minute = add_leading_zeros(std::to_string(time.tm_min), 2);
        std::string second = add_leading_zeros(std::to_string(time.tm_sec), 2);
        return year + month + day + "_" + hour + minute + second;
    }

    static std::string get_timestamp()
    {
        struct tm *time_struct;
        time_t raw_time;
        time(&raw_time);
        time_struct = localtime(&raw_time);
        return tm_to_string(*time_struct);
    }

    static std::string exec(const char *cmd)
    {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe)
        {
            throw std::runtime_error("_popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }
        return result;
    }

    static std::vector<std::string> make_images_list(std::string saveto_dir)
    {
        std::vector<std::string> images_list;
        for (const auto &entry : std::filesystem::directory_iterator(saveto_dir))
        {
            images_list.push_back(entry.path().string());
        }
        return images_list;
    }
};