// Copyright 2014 Renato Tegon Forti, Antony Polukhin.
// Copyright Antony Polukhin, 2015-2025.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

//[plugcpp_my_plugin_refcounting_api
#include "../tutorial_common/my_plugin_api.hpp"
#include <boost/dll/config.hpp>

class my_refcounting_api: public my_plugin_api {
public:
    // Returns path to shared object that holds a plugin.
    // Must be instantiated in plugin.
    virtual boost::dll::fs::path location() const = 0;
};
//]


//[plugcpp_library_holding_deleter_api_bind
#include <memory>
#include <boost/dll/shared_library.hpp>

struct library_holding_deleter {
    std::shared_ptr<boost::dll::shared_library> lib_;

    void operator()(my_refcounting_api* p) const {
        delete p;
    }
};

inline std::shared_ptr<my_refcounting_api> bind(my_refcounting_api* plugin) {
    // getting location of the shared library that holds the plugin
    boost::dll::fs::path location = plugin->location();

    // `make_shared` is an efficient way to create a shared pointer
    auto lib = std::make_shared<boost::dll::shared_library>(location);

    library_holding_deleter deleter;
    deleter.lib_ = lib;

    return std::shared_ptr<my_refcounting_api>(
        plugin, deleter
    );
}
//]

//[plugcpp_get_plugin_refcounting
#include <boost/dll/import.hpp>
inline std::shared_ptr<my_refcounting_api> get_plugin(
    boost::dll::fs::path path, const char* func_name)
{
    using func_t = my_refcounting_api*();
    auto creator = boost::dll::import_alias<func_t>(
        path, 
        func_name, 
        boost::dll::load_mode::append_decorations   // will be ignored for executable
    );

    // `plugin` does not hold a reference to shared library. If `creator` will go out of scope, 
    // then `plugin` can not be used.
    my_refcounting_api* plugin = creator();

    // Returned variable holds a reference to 
    // shared_library and it is safe to use it.
    return bind( plugin );

    // `creator` goes out of scope here and will be destroyed.
}

//]


