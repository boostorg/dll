// Copyright 2024 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/core/lightweight_test.hpp>

#include <boost/dll/detail/demangling/msvc.hpp>

int main(int argc, char* argv[])
{
    namespace parser = boost::dll::detail::parser;

    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("public: __cdecl foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("private: __cdecl foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("protected: __cdecl foo::~foo(void)"));

    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("public: virtual __cdecl foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("private: virtual __cdecl foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("protected: virtual __cdecl foo::~foo(void)"));

    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("public: __cdecl foo::~foo(void) __ptr64"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("private: __cdecl foo::~foo(void) __ptr64"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("protected: __cdecl foo::~foo(void) __ptr64"));

    BOOST_TEST(parser::is_destructor_with_name{"some_space::some_father::~some_father(void)"}("public: __cdecl some_space::some_father::~some_father(void) __ptr64"));

    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("public: __thiscall foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("private: __thiscall foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("protected: __thiscall foo::~foo(void)"));

    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("public: virtual __thiscall foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("private: virtual __thiscall foo::~foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("protected: virtual __thiscall foo::~foo(void)"));

    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("public: __thiscall foo::~foo(void) __ptr32"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("private: __thiscall foo::~foo(void) __ptr32"));
    BOOST_TEST(parser::is_destructor_with_name{"foo::~foo(void)"}("protected: __thiscall foo::~foo(void)  __ptr32"));

    BOOST_TEST(parser::is_destructor_with_name{"some_space::some_father::~some_father(void)"}("public: __thiscall some_space::some_father::~some_father(void) __ptr64"));


    return boost::report_errors();
}
