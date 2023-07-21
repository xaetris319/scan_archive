#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "messages.hpp"
#include "file_operations.hpp"
#include "naps2.hpp"
#include "zxing.hpp"

class _main_
{
public:
    static void entry()
    {
        set_work_dir();
        std::string input_content = get_input_content();
        int mode = get_mode_from_input(input_content);
        std::vector<std::string> images_list;
        std::string profile_name, file_extension, console_output;
        switch (mode)
        {
        case 1:
            get_profile_info_from_input(input_content, profile_name, file_extension);
            images_list = naps2::perform_scanning(work_dir, profile_name, file_extension, console_output);
            if (!console_output.empty())
            {
                make_output_file(message_type::ERROR, console_output, false);
            }
            break;
        case 2:
            images_list = get_images_list_from_input(input_content);
            break;
        default:
            break;
        }
        if (images_list.empty())
        {
            make_output_file(message_type::ERROR, message::NO_FILES, false);
        }
        for (std::string const &image : images_list)
        {
            std::string barcodes_entry;
            zxing::decode_image(image, barcodes_entry);
            barcodes_list.push_back(barcodes_entry);
        }
        make_output_file(message_type::NORMAL, message::EMPTY, true);
    }

private:
    inline static std::string work_dir;
    inline static std::vector<std::string> barcodes_list;
    constexpr static std::string_view sep = ";";

    _main_() {}
    ~_main_() {}

    static void set_work_dir()
    {
        char *appdata_value;
        appdata_value = std::getenv("APPDATA");
        if (appdata_value == NULL)
        {
            exit(1);
        }
        std::string appdata = appdata_value;
        work_dir = appdata + "\\SAP\\ScanArchive\\";
    }

    static std::string get_input_content()
    {
        std::string input_location = work_dir + "input.txt";
        std::string input_content;
        if (file_operations::read_file(input_location, input_content) != 0)
        {
            make_output_file(message_type::ERROR, message::NO_INPUT, false);
        }
        return input_content;
    }

    static int get_mode_from_input(const std::string &input_content)
    {
        int mode;
        size_t pos_end = input_content.find(";", 0);
        if (pos_end == std::string::npos)
        {
            make_output_file(message_type::ERROR, message::WRONG_INPUT, false);
        }
        try
        {
            mode = stoi(input_content.substr(0, pos_end));
        }
        catch (const std::invalid_argument &e)
        {
            mode = 0;
        }
        catch (const std::out_of_range &e)
        {
            mode = 0;
        }
        if (mode != 1 && mode != 2)
        {
            make_output_file(message_type::ERROR, message::WRONG_INPUT, false);
        }
        return mode;
    }

    static void get_profile_info_from_input(const std::string &input_content,
                                            std::string &profile_name, std::string &file_extension)
    {
        size_t pos_start = 0, pos_end, content_len = input_content.length(), sep_len = sep.length();
        for (int i = 1; pos_end = input_content.find(sep, pos_start) != std::string::npos; i++)
        {
            if (i < 2)
            {
                pos_start = pos_end + sep_len;
            }
            else if (i == 2)
            {
                profile_name = input_content.substr(pos_start, pos_end - pos_start);
                pos_start = pos_end + sep_len;
            }
            else if (i == 3)
            {
                file_extension = input_content.substr(pos_start, pos_end - pos_start);
                break;
            }
        }
        if (profile_name.empty() || file_extension.empty())
        {
            make_output_file(message_type::ERROR, message::WRONG_INPUT, false);
        }
    }

    static std::vector<std::string> get_images_list_from_input(const std::string &input_content)
    {
        std::vector<std::string> images_list;
        size_t pos_start = 0, pos_end, content_len = input_content.length(), sep_len = sep.length();
        std::string token;
        bool first_segment = true;
        while ((pos_end = input_content.find(sep, pos_start)) != std::string::npos)
        {
            if (first_segment)
            {
                first_segment = false;
                pos_start = pos_end + sep_len;
                continue;
            }
            token = input_content.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + sep_len;
            images_list.push_back(token);
        }
        token = input_content.substr(pos_start, content_len - pos_start);
        images_list.push_back(token);
        return images_list;
    }

    static void make_output_file(const int &message_code, const std::string &message, const bool &successful)
    {
        std::string sep = ";";
        std::string output_content = std::to_string(message_code) + sep + message;
        if (successful && !barcodes_list.empty())
        {
            for (std::string const &barcode_entry : barcodes_list)
            {
                output_content = output_content + sep + barcode_entry;
            }
        }
        std::string output_location = work_dir + "output.txt";
        file_operations::write_file(output_location, output_content);
        exit(0);
    }
};

int main()
{
    _main_::entry();
    return 0;
}