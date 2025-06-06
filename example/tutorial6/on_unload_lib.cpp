// Copyright 2014 Renato Tegon Forti, Antony Polukhin.
// Copyright Antony Polukhin, 2015-2025.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// MinGW related workaround
#define BOOST_DLL_FORCE_ALIAS_INSTANTIATION

//[plugcpp_on_unload
#include <boost/dll/alias.hpp> // for BOOST_DLL_ALIAS
#include <functional>
#include <vector>

namespace my_namespace {

struct on_unload {
    using callback_t = std::function<void()> ;
    using this_type = on_unload;

    ~on_unload() {
        for (std::size_t i = 0; i < callbacks_.size(); ++i) {
            callback_t& function = callbacks_[i];
            function(); // calling the callback
        }
    }

    // not thread safe
    static void add(const callback_t& function) {
        static this_type instance;
        instance.callbacks_.push_back(function);
    }

private:
    std::vector<callback_t> callbacks_;
    on_unload() {} // prohibit construction outside of the `add` function
};

// Exporting the static "add" function with name "on_unload"
BOOST_DLL_ALIAS(my_namespace::on_unload::add, on_unload)

} // namespace my_namespace

//]



