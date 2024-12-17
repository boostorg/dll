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

#if defined(_WIN64)
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("public: __cdecl foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("private: __cdecl foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("protected: __cdecl foo(void)"));

    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("public: __cdecl foo(void) __ptr64"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("private: __cdecl foo(void) __ptr64"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("protected: __cdecl foo(void) __ptr64"));
#else
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("public: __thiscall foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("private: __thiscall foo(void)"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("protected: __thiscall foo(void)"));

    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("public: __thiscall foo(void) __ptr32"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("private: __thiscall foo(void) __ptr32"));
    BOOST_TEST(parser::is_destructor_with_name{"foo(void)"}("protected: __thiscall foo(void)  __ptr32"));
#endif

    return boost::report_errors();
}
