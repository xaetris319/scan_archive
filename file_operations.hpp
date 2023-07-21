#include <string>
#include <fstream>

class file_operations
{
public:
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
        out << content;
        out.close();
        return;
    }

private:
    file_operations() {}
    ~file_operations() {}
};
