// Copyright Antony Polukhin, 2016-2025.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DLL_MY_PLUGIN_API_HPP
#define BOOST_DLL_MY_PLUGIN_API_HPP

//[plugapi
#include <boost/config.hpp>
#include <string>

class BOOST_SYMBOL_VISIBLE my_plugin_api {
public:
   virtual std::string name() const = 0;
   virtual float calculate(float x, float y) = 0;

   virtual ~my_plugin_api() = default;
};
//]
   
#endif // BOOST_DLL_MY_PLUGIN_API_HPP

