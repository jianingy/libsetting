#include <vector>
#include <iostream>

#include "text_config.h"

int main()
{
    std::vector<std::string> vec;
    std::string s;
    TextConfig cfg("sample.cfg");

    std::cout <<  "int      => " << cfg.get_int("int") << std::endl
              <<  "long     => " << cfg.get_long("long") << std::endl
              <<  "dobule   => " << cfg.get_double("double") << std::endl
              <<  "string   => " << cfg.get_cstr("string") << std::endl;

    std::cout <<  "vector   => ";
    cfg.get_vector("vector", &vec);
    for (int i = 0; i < vec.size(); i++)
        std::cout << "'" << vec[i] << "' ";
    std::cout << std::endl;
    std::cout <<  "cite     => " << cfg.get_cstr("cite") << std::endl;
    cfg << "dynamic = $int + $long";
    std::cout <<  "dynamic  => " << cfg.get_cstr("dynamic") << std::endl;
    std::string dump;
    cfg.dump(&dump);
    std::cout << "\nDump Text Config\n" << dump << std::endl;
}

// vim: ts=4 sw=4 ai cindent et
