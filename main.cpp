#include <array>
#include <codecvt>
#include <deque>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/*
 * Зависимости от zxing
 */
#include "zxing-cpp-master/core/src/ReadBarcode.h"
#include "zxing-cpp-master/core/src/GTIN.h"
#include "zxing-cpp-master/core/ZXVersion.h.in"

#include <cctype>
#include <chrono>
#include <cstring>
// #include <iostream>
#include <iterator>
#include <memory>
// #include <string>
// #include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb-master/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb-master/stb_image_write.h"
/*
 * Зависимости от zxing
 */

#include "messages.hpp"
#include "os_operations.hpp"
#include "naps2.hpp"
#include "zxing_decoder.hpp"

class _main_
{
public:
    static void entry()
    {
        // получаем рабочую директорию в AppData юзера
        set_work_dir();
        // получаем содержимое входящего файла с данными для программы
        std::string input_content = get_input_content();
        // 1 - запуск сканера, 2 - простое декодирование файлов в папке
        int mode = get_mode_from_input(input_content);
        // вектор путей к конкретным изображениям
        std::vector<std::string> images_list;
        // параметры для сканирования, + путь к папке с изображениями
        std::string profile_name, file_extension, saveto_dir, console_output;
        switch (mode)
        {
        case 1:
            // случай сканирования, получаем название профиля NAPS2, расширение файлов
            get_profile_info_from_input(input_content, profile_name, file_extension);
            // на выходе имеется вектор изображений, папка сохранения, вывод консольки
            images_list = naps2::perform_scanning(work_dir, profile_name, file_extension, saveto_dir, console_output);
            if (!console_output.empty())
            {
                // случай, когда NAPS2 сработал с ошибкой
                make_output_file(message_type::ERROR, console_output, false);
            }
            break;
        case 2:
            // случай загрузки изображений из папки, получаем вектор и папку сохранения
            images_list = get_images_list_from_input(input_content, saveto_dir);
            break;
        default:
            break;
        }
        if (images_list.empty())
        {
            // если вектор изображений пустой - что-то не так
            make_output_file(message_type::ERROR, message::NO_FILES, false);
        }

        barcodes_list = zxing_decoder::decode_images(saveto_dir, images_list);

        // for (std::string const &image : images_list)
        // {
        //     std::string barcodes_entry;
        //     zxing::decode_image(work_dir, image, barcodes_entry);
        //     barcodes_list.push_back(barcodes_entry);
        // }
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
        if (os_operations::read_file(input_location, input_content) != 0)
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
        size_t pos_start = 0, pos_end, sep_len = sep.length();
        int counter = 0;
        while ((pos_end = input_content.find(sep, pos_start)) != std::string::npos)
        {
            if (counter > 2)
            {
                break;
            }
            else if (counter == 1)
            {
                profile_name = input_content.substr(pos_start, pos_end - pos_start);
            }
            else if (counter == 2)
            {
                file_extension = input_content.substr(pos_start, pos_end - pos_start);
            }
            pos_start = pos_end + sep_len;
            counter++;
        }
        if (profile_name.empty() || file_extension.empty())
        {
            make_output_file(message_type::ERROR, message::WRONG_INPUT, false);
        }
    }

    static std::vector<std::string> get_images_list_from_input(const std::string &input_content,
                                                               std::string &saveto_dir)
    {
        std::vector<std::string> images_list;
        size_t pos_start = 0, pos_end, sep_len = sep.length();
        std::string token;
        int segment = 0;
        while ((pos_end = input_content.find(sep, pos_start)) != std::string::npos)
        {
            if (segment < 2)
            {
                segment++;
                if (segment == 2)
                {
                    saveto_dir = input_content.substr(pos_start, pos_end - pos_start);
                }
                pos_start = pos_end + sep_len;
                continue;
            }
            token = input_content.substr(pos_start, pos_end - pos_start);
            pos_start = pos_end + sep_len;
            images_list.push_back(token);
        }
        return images_list;
    }

    static void make_output_file(const int &message_code, const std::string &message, const bool &successful)
    {
        std::string _sep = {sep.begin(), sep.end()};
        std::string output_content = std::to_string(message_code) + _sep + message + _sep;
        if (successful && !barcodes_list.empty())
        {
            for (std::string const &barcode_entry : barcodes_list)
            {
                output_content = output_content + barcode_entry + _sep;
            }
        }
        std::string output_location = work_dir + "output.txt";
        os_operations::write_file(output_location, output_content);
        exit(0);
    }
};

int main()
{
    setlocale(LC_ALL, "en-US");
    _main_::entry();
    return 0;
}