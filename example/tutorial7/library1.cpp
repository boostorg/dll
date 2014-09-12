// Copyright 2014 Renato Tegon Forti, Antony Polukhin.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org


//[plugcpp_tutorial7_library1
#include <iostream>
#include <boost/dll.hpp>

void print(const std::string& s) {
    std::cout << "Hello, " << s << '!' << std::endl;
}

BOOST_DLL_ALIAS_SECTIONED(print, print_hello, Anna)
//]