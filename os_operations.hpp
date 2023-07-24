class os_operations
{
public:
    static std::string add_leading_zeros(const std::string &input, const size_t &req_length)
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

    static int read_file(const std::string &file_location, std::string &content)
    {
        std::ifstream in;
        in.open(file_location);
        if (in.is_open())
        {
            in >> content;
            in.close();
            return 0;
        }
        return -1;
    }

    static void write_file(const std::string &file_location, const std::string &content)
    {
        std::ofstream out;
        out.open(file_location);
        if (!out.is_open())
        {
            exit(1);
        }
        size_t content_len = content.length();
        std::cout << "\n\nPrinting content by characters:\n";
        for (size_t i = 0; i < content_len; i++)
        {
            std::cout << content[i] << "\t=\t" << (static_cast<int>(content[i]) + 128) << std::endl;
        }
        std::cout << "End of printed content\n";
        out << content;
        out.close();

        // std::ofstream out;
        // out.open(file_location, std::ios::out | std::ios::binary);

        // unsigned char smarker[3];
        // smarker[0] = 0xEF;
        // smarker[1] = 0xBB;
        // smarker[2] = 0xBF;

        // out << smarker;
        // out.close();

        // std::ofstream out2;
        // out2.open(file_location, std::ios::out|std::ios::app);
        // out2 << content;
        // out2.close();

        // std::wofstream wout;
        // wout.open(file_location, std::ios::out | std::ios::app);
        // // std::locale utf8_locale(std::locale(), new utf8cvt<false>);
        // std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);
        // wout.imbue(utf8_locale);
        // wout << std::wstring(content.begin(), content.end());
        // wout.close();

        // const std::locale utf8_locale = std::locale(std::locale(), new std::codecvt_utf8<wchar_t>);
        // std::wstring wcontent(content.begin(), content.end());
        // std::wofstream out;
        // out.open(file_location);
        // if (!out.is_open())
        // {
        //     exit(1);
        // }
        // out.imbue(utf8_locale);
        // out << wcontent;
        // out.close();
        return;
    }

    static void cp866_cp1251(std::string &s)
    {
        for (char *c = (char *)s.c_str(); *c != 0; ++c)
        {
            if (*c > -129 && *c < -80)
                *c += 64;
            else if (*c > -33 && *c < -16)
                *c += 16;
        }
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
        cp866_cp1251(result);
        return result;
    }

private:
    os_operations() {}
    ~os_operations() {}
};
