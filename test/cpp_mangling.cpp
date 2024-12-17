// Copyright 2024 Antony Polukhin
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org

#include <boost/core/lightweight_test.hpp>

#if (__cplusplus > 201402L) || (defined(_MSVC_LANG) && _MSVC_LANG > 201402L)
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


    boost::dll::detail::mangled_storage_impl ms;
    {
        void(*ptr1)(int) = nullptr;
        BOOST_TEST_EQ(parser::find_arg_list(ms, "int", ptr1), 3);
    }
    {
        void(*ptr2)() = nullptr;
        BOOST_TEST_EQ(parser::find_arg_list(ms, "void", ptr2), 4);
    }
    {
        void(*ptr3)(int,int) = nullptr;
        BOOST_TEST_EQ(parser::find_arg_list(ms, "int,int", ptr3), 7);
    }
    {
        void(*ptr4)(int,short,long) = nullptr;
        BOOST_TEST_EQ(parser::find_arg_list(ms, "int,short,long", ptr4), 14);
    }


    BOOST_TEST((
        parser::is_constructor_with_name<void(*)()>{"some_space::some_class::some_class", ms}
            ("public: __cdecl some_space::some_class::some_class(void) __ptr64")
    ));
    BOOST_TEST((
        parser::is_constructor_with_name<void(*)(int)>{"some_space::some_class::some_class", ms}
            ("private: __cdecl some_space::some_class::some_class(int)")
    ));
    BOOST_TEST((
        parser::is_constructor_with_name<void(*)(int,int)>{"some_space::some_class::some_class", ms}
            ("private: __cdecl some_space::some_class::some_class(int,int)")
    ));


    BOOST_TEST((
        parser::is_variable_with_name<int>{"some_space::some_class::value", ms}
            ("public: static int some_space::some_class::value")
    ));

    BOOST_TEST((
        parser::is_variable_with_name<int>{"some_space::some_class::value", ms}
            ("int some_space::some_class::value")
    ));

    BOOST_TEST((
        parser::is_variable_with_name<double>{"some_space::variable", ms}
            ("public: static int some_space::variable")
    ));

    BOOST_TEST((
        parser::is_variable_with_name<const double>{"unscoped_c_var", ms}
            ("double const unscoped_c_var")
    ));

    return boost::report_errors();
}
#else
int main() {return 0;}
#endif