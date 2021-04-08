#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

/* C++ STL HEADER FILES */
#include <stdexcept>
#include <string>

#include <iostream>

namespace cpet{
    class value_error : std::runtime_error {
        public:
            explicit value_error(const std::string& what_arg) : std::runtime_error(what_arg){std::cerr << what_arg;}

            explicit value_error(const char * what_arg) : std::runtime_error(what_arg){std::cerr << what_arg;}
    };

    class value_not_found : std::runtime_error {
        public:
            explicit value_not_found(const std::string& what_arg) : std::runtime_error(what_arg){}

            explicit value_not_found(const char * what_arg) : std::runtime_error(what_arg){}
    };

    class io_error : std::runtime_error {
        public:
            explicit io_error(const std::string& what_arg) : std::runtime_error(what_arg){}

            explicit io_error(const char * what_arg) : std::runtime_error(what_arg){}
    };



}

#endif //EXCEPTIONS_H
