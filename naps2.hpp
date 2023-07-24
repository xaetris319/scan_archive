
class naps2
{
public:
    static std::vector<std::string> perform_scanning(const std::string &work_dir,
                                                     const std::string &profile_name,
                                                     const std::string &file_extension,
                                                     std::string &saveto_dir,
                                                     std::string &console_output)
    {
        saveto_dir = work_dir + os_operations::get_timestamp() + "\\";
        std::string naps2_location = "\"C:\\Program Files (x86)\\NAPS2\\naps2.console\"";
        std::string profile_option = " -p " + profile_name;
        std::string output_option = " -o " + saveto_dir + "DOC$(nnn)." + file_extension;
        std::string other_options = " --FORCE --SPLIT";
        std::string naps2_command = naps2_location + profile_option + output_option + other_options;
        try
        {
            console_output = os_operations::exec(naps2_command.c_str());
        }
        catch (const std::runtime_error &e)
        {
            console_output = e.what();
        }
        return make_images_list(saveto_dir, console_output);
    }

private:
    naps2() {}
    ~naps2() {}

    static std::vector<std::string> make_images_list(const std::string &saveto_dir, const std::string &command_output)
    {
        std::vector<std::string> images_list;
        if (command_output.empty())
        {
            try
            {
                for (const auto &entry : std::filesystem::directory_iterator(saveto_dir))
                {
                    images_list.push_back(entry.path().string());
                }
            }
            catch (const std::filesystem::__cxx11::filesystem_error &e)
            {
            }
        }
        return images_list;
    }
};