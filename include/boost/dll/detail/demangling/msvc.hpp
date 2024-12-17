//  Copyright 2016 Klemens Morgenstern
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_DLL_DETAIL_DEMANGLING_MSVC_HPP_
#define BOOST_DLL_DETAIL_DEMANGLING_MSVC_HPP_

#include <boost/dll/detail/demangling/mangled_storage_base.hpp>

#include <boost/core/detail/string_view.hpp>

#include <iterator>
#include <algorithm>
#include <type_traits>

#include <boost/spirit/home/x3.hpp>

namespace boost { namespace dll { namespace detail {

class mangled_storage_impl  : public mangled_storage_base
{
    template<typename T>
    struct dummy {};

    template<typename Return, typename ...Args>
    std::vector<std::string> get_func_params(dummy<Return(Args...)>) const
    {
        return {get_name<Args>()...};
    }
    template<typename Return, typename ...Args>
    std::string get_return_type(dummy<Return(Args...)>) const
    {
        return get_name<Return>();
    }
    //function to remove preceding 'class ' or 'struct ' if the are given in this format.

    inline static void trim_typename(std::string & val);
public:
    using ctor_sym = std::string;
    using dtor_sym = std::string;

    using mangled_storage_base::mangled_storage_base;

    template<typename T>
    std::string get_variable(const std::string &name) const;

    template<typename Func>
    std::string get_function(const std::string &name) const;

    template<typename Class, typename Func>
    std::string get_mem_fn(const std::string &name) const;

    template<typename Signature>
    ctor_sym get_constructor() const;

    template<typename Class>
    dtor_sym get_destructor() const;

    template<typename T> //overload, does not need to virtual.
    std::string get_name() const
    {
        auto nm = mangled_storage_base::get_name<T>();
        trim_typename(nm);
        return nm;
    }

    template<typename T>
    std::string get_vtable() const;

    template<typename T>
    std::vector<std::string> get_related() const;

};

void mangled_storage_impl::trim_typename(std::string & val)
{
    //remove preceding class or struct, because you might want to use a struct as class, et vice versa
    if (val.size() >= 6)
    {
        using namespace std;
        static constexpr char class_ [7] = "class ";
        static constexpr char struct_[8] = "struct ";

        if (equal(begin(class_), end(class_)-1, val.begin())) //aklright, starts with 'class '
            val.erase(0, 6);
        else if (val.size() >= 7)
            if (equal(begin(struct_), end(struct_)-1, val.begin()))
                val.erase(0, 7);
    }
}


namespace parser
{
    namespace x3 = spirit::x3;

    inline auto ptr_rule_impl(std::integral_constant<std::size_t, 32>)
    {
        return -((-x3::space) >> "__ptr32");
    }
    inline auto ptr_rule_impl(std::integral_constant<std::size_t, 64>)
    {
        return -((-x3::space) >> "__ptr64");
    }

    inline auto ptr_rule() {
        return ptr_rule_impl(std::integral_constant<std::size_t, sizeof(std::size_t)*8>());
    }

    inline boost::core::string_view trim_ptrs(boost::core::string_view s) {
        bool retry = false;
        do {
            retry = false;
            while (s.starts_with(" ")) {
                s.remove_prefix(1);
            }

            if (s.starts_with("__ptr32") || s.starts_with("__ptr64")) {
                s.remove_prefix(sizeof("__ptr64") - 1);
                retry = true;
            }
        } while (retry);

        return s;
    }

    inline boost::core::string_view trim_prefix(boost::core::string_view s, boost::core::string_view prefix) {
        if (s.starts_with(prefix)) {
            s.remove_prefix(prefix.size());
        }
        return s;
    }

    auto const visibility = ("public:" | x3::lit("protected:") | "private:");
    inline std::string::size_type find_visibility(boost::core::string_view s) {
        if (s.starts_with("public:")) {
            return sizeof("public:") - 1;
        } else if (s.starts_with("protected:")) {
            return sizeof("protected:") - 1;
        } else if (s.starts_with("private:")) {
            return sizeof("private:") - 1;
        }
        return std::string::npos;
    }

    auto const virtual_ = x3::space >> "virtual";
    inline std::string::size_type find_virtual(boost::core::string_view s) {
        if (s.starts_with(" virtual")) {
            return sizeof(" virtual") - 1;
        }
        return std::string::npos;
    }

    auto const static_     = x3::space >> x3::lit("static") ;
    inline std::string::size_type find_static(boost::core::string_view s) {
        if (s.starts_with(" static")) {
            return sizeof(" static") - 1;
        }
        return std::string::npos;
    }

    inline auto const_rule_impl(std::true_type )  {return x3::space >> "const";};
    inline auto const_rule_impl(std::false_type)  {return x3::eps;};
    template<typename T>
    auto const_rule() {using t = std::is_const<typename std::remove_reference<T>::type>; return const_rule_impl(t());}

    inline auto volatile_rule_impl(std::true_type )  {return x3::space >> "volatile";};
    inline auto volatile_rule_impl(std::false_type)  {return x3::eps;};
    template<typename T>
    auto volatile_rule() {using t = std::is_volatile<typename std::remove_reference<T>::type>; return volatile_rule_impl(t());}


    inline auto inv_const_rule_impl(std::true_type )  {return "const" >>  x3::space ;};
    inline auto inv_const_rule_impl(std::false_type)  {return x3::eps;};
    template<typename T>
    auto inv_const_rule() {using t = std::is_const<typename std::remove_reference<T>::type>; return inv_const_rule_impl(t());}

    inline auto inv_volatile_rule_impl(std::true_type )  {return "volatile" >> x3::space;};
    inline auto inv_volatile_rule_impl(std::false_type)  {return x3::eps;};
    template<typename T>
    auto inv_volatile_rule() {using t = std::is_volatile<typename std::remove_reference<T>::type>; return inv_volatile_rule_impl(t());}


    inline auto reference_rule_impl(std::false_type, std::false_type) {return x3::eps;}
    inline auto reference_rule_impl(std::true_type,  std::false_type) {return x3::space >>"&"  ;}
    inline auto reference_rule_impl(std::false_type, std::true_type ) {return x3::space >>"&&" ;}


    template<typename T>
    auto reference_rule() {using t_l = std::is_lvalue_reference<T>; using t_r = std::is_rvalue_reference<T>; return reference_rule_impl(t_l(), t_r());}

    auto const class_ = ("class" | x3::lit("struct"));

    //it takes a string, because it may be overloaded.
    template<typename T>
    auto type_rule(const std::string & type_name)
    {
        using namespace std;

        return -(class_ >> x3::space)>> x3::string(type_name) >>
                const_rule<T>() >>
                volatile_rule<T>() >>
                reference_rule<T>() >>
                ptr_rule();
    }
    template<>
    inline auto type_rule<void>(const std::string &) { return x3::string("void"); };

    template<typename T>
    std::string::size_type find_type(const mangled_storage_impl& ms, boost::core::string_view s_orig) {
        if (std::is_void<T>::value) {
            if (s_orig.starts_with("void")) {
                return sizeof("void") - 1;
            }
            return std::string::npos;
        }

        auto s = s_orig;
        s = trim_prefix(s, "class ");
        s = trim_prefix(s, "struct ");

        const auto& mangled_name = ms.get_name<T>();
        if (!s.starts_with(mangled_name)) {
            return std::string::npos;
        }
        s.remove_prefix(mangled_name.size());

        if (std::is_const<T>::value) {
            if (!s.starts_with(" const")) {
                return std::string::npos;
            } else {
                s.remove_prefix(sizeof(" const") - 1);
            }
        }

        if (std::is_volatile<T>::value) {
            if (!s.starts_with(" volatile")) {
                return std::string::npos;
            } else {
                s.remove_prefix(sizeof(" volatile") - 1);
            }
        }

        if (std::is_rvalue_reference<T>::value) {
            if (!s.starts_with(" &&")) {
                return std::string::npos;
            } else {
                s.remove_prefix(sizeof(" &&") - 1);
            }
        }

        if (std::is_lvalue_reference<T>::value) {
            if (!s.starts_with(" &")) {
                return std::string::npos;
            } else {
                s.remove_prefix(sizeof(" &") - 1);
            }
        }

        s = trim_ptrs(s);
        return s_orig.size() - s.size();
    }

    auto const cdecl_   = "__cdecl"     >> x3::space;
    auto const stdcall  = "__stdcall"     >> x3::space;
#if defined(_WIN64)//seems to be necessary by msvc 14-x64
    auto const thiscall = "__cdecl"     >> x3::space;
#else
    auto const thiscall = "__thiscall"     >> x3::space;
#endif
    inline std::string::size_type find_thiscall(boost::core::string_view s) {
        if (s.starts_with(" __cdecl ")) {               // Win 64bit
            return sizeof(" __cdecl ") - 1;
        } else if (s.starts_with(" __thiscall ")) {     // Win 32bit
            return sizeof(" __thiscall ") - 1;
        }
        return std::string::npos;
    }

    template<typename Return, typename Arg>
    auto arg_list(const mangled_storage_impl & ms, Return (*)(Arg))
    {
        return type_rule<Arg>(ms.get_name<Arg>());
    }

    template<typename Return, typename First, typename Second, typename ...Args>
    auto arg_list(const mangled_storage_impl & ms, Return (*)(First, Second, Args...))
    {
        using next_type = Return (*)(Second, Args...);
        return type_rule<First>(ms.get_name<First>()) >> x3::char_(',') >> arg_list(ms, next_type());
    }

    template<typename Return>
    auto arg_list(const mangled_storage_impl& /*ms*/, Return (*)())
    {
        return x3::string("void");
    }


    template<typename Return, typename Arg>
    std::string::size_type find_arg_list(const mangled_storage_impl& ms, boost::core::string_view s, Return (*)(Arg))
    {
        return parser::find_type<Arg>(ms, s);
    }

    template<typename Return, typename First, typename Second, typename ...Args>
    std::string::size_type find_arg_list(const mangled_storage_impl & ms, boost::core::string_view s, Return (*)(First, Second, Args...))
    {
        using next_type = Return (*)(Second, Args...);

        const auto res = parser::find_type<First>(ms, s);
        if (res == std::string::npos) {
            return std::string::npos;
        }
        s.remove_prefix(res);
        if (!s.starts_with(",")) {
            return std::string::npos;
        }
        s.remove_prefix(1);

        const auto new_res = parser::find_arg_list(ms, s, next_type());
        if (new_res == std::string::npos) {
            return std::string::npos;
        }

        return res + 1 + new_res;
    }

    template<typename Return>
    std::string::size_type find_arg_list(const mangled_storage_impl& ms, boost::core::string_view s, Return (*)()) {
        return parser::find_type<void>(ms, s);
    }

    class is_destructor_with_name {
        const std::string dtor_name_;

    public:
        explicit is_destructor_with_name(std::string dtor_name) : dtor_name_(std::move(dtor_name)) {}

        inline bool operator()(boost::core::string_view s) const {
            {
                const auto visibility_pos = parser::find_visibility(s);
                if (visibility_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(visibility_pos);
            }
            s = trim_prefix(s, " virtual");
            {
                // cdecl declaration for methods. stdcall cannot be
                const auto thiscall_pos = parser::find_thiscall(s);
                if (thiscall_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(thiscall_pos);
            }

            if (!s.starts_with(dtor_name_)) {
                return false;
            }
            s.remove_prefix(dtor_name_.size());
            s = parser::trim_ptrs(s);
            return s.empty();
        }

        inline bool operator()(const mangled_storage_base::entry& e) const {
            return (*this)(boost::core::string_view(e.demangled.data(), e.demangled.size()));
        }
    };

    template<typename T>
    class is_variable_with_name {
        const std::string variable_name_;
        const mangled_storage_impl& ms_;

    public:
        is_variable_with_name(std::string variable_name, const mangled_storage_impl& ms)
            : variable_name_(std::move(variable_name)), ms_(ms) {}

        inline bool operator()(boost::core::string_view s) const {
            {
                const auto visibility_pos = parser::find_visibility(s);
                if (visibility_pos != std::string::npos) {
                    s.remove_prefix(visibility_pos);
                    s = trim_prefix(s, " static ");
                }
            }
            {
                const auto type_pos = parser::find_type<T>(ms_, s);
                if (type_pos == std::string::npos) {
                    return std::string::npos;
                }
                s.remove_prefix(type_pos);
            }

            if (!s.starts_with(variable_name_)) {
                return false;
            }
            s.remove_prefix(variable_name_.size());
            return s.empty();
        }

        inline bool operator()(const mangled_storage_base::entry& e) const {
            return (*this)(boost::core::string_view(e.demangled.data(), e.demangled.size()));
        }
    };

    template <class Signature>
    class is_constructor_with_name {
        const std::string ctor_name_;
        const mangled_storage_impl& ms_;

    public:
        is_constructor_with_name(std::string ctor_name, const mangled_storage_impl& ms)
            : ctor_name_(std::move(ctor_name)), ms_(ms) {}

        inline bool operator()(boost::core::string_view s) const {
            {
                const auto visibility_pos = parser::find_visibility(s);
                if (visibility_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(visibility_pos);
            }
            {
                // cdecl declaration for methods. stdcall cannot be
                const auto thiscall_pos = parser::find_thiscall(s);
                if (thiscall_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(thiscall_pos);
            }

            if (!s.starts_with(ctor_name_)) {
                return false;
            }
            s.remove_prefix(ctor_name_.size());

            if (!s.starts_with("(")) {
                return false;
            }
            s.remove_prefix(1);

            {
                const auto arg_list_pos = parser::find_arg_list(ms_, s, Signature());
                if (arg_list_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(arg_list_pos);
            }

            if (!s.starts_with(")")) {
                return false;
            }
            s.remove_prefix(1);

            s = parser::trim_ptrs(s);
            return s.empty();
        }

        inline bool operator()(const mangled_storage_base::entry& e) const {
            return (*this)(boost::core::string_view(e.demangled.data(), e.demangled.size()));
        }
    };

    template <class Signature>
    class is_function_with_name;

    template <class Result, class... Args>
    class is_function_with_name<Result(*)(Args...)> {
        const std::string function_name_;
        const mangled_storage_impl& ms_;

    public:
        is_function_with_name(std::string function_name, const mangled_storage_impl& ms)
            : function_name_(std::move(function_name)), ms_(ms) {}

        inline bool operator()(boost::core::string_view s) const {
            {
                const auto visibility_pos = parser::find_visibility(s);
                if (visibility_pos != std::string::npos) {
                    s.remove_prefix(visibility_pos);
                    s = trim_prefix(s, " static ");
                }
            }
            {
                const auto type_pos = parser::find_type<Result>(ms_, s);
                if (type_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(type_pos);
            }
            if (!s.starts_with(" __cdecl ")) {
                return false;
            }
            s.remove_prefix(sizeof(" __cdecl ") - 1);

            if (!s.starts_with(function_name_)) {
                return false;
            }
            s.remove_prefix(function_name_.size());

            if (!s.starts_with("(")) {
                return false;
            }
            s.remove_prefix(1);

            {
                using Signature = Result(*)(Args...);
                const auto arg_list_pos = parser::find_arg_list(ms_, s, Signature());
                if (arg_list_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(arg_list_pos);
            }

            if (!s.starts_with(")")) {
                return false;
            }
            s.remove_prefix(1);

            s = parser::trim_ptrs(s);
            return s.empty();
        }

        inline bool operator()(const mangled_storage_base::entry& e) const {
            return (*this)(boost::core::string_view(e.demangled.data(), e.demangled.size()));
        }
    };
}


template<typename T>
std::string mangled_storage_impl::get_variable(const std::string &name) const {
    const auto found = std::find_if(storage_.begin(), storage_.end(), parser::is_variable_with_name<T>(std::move(name)));

    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}

template<typename Func> std::string mangled_storage_impl::get_function(const std::string &name) const
{
    namespace x3 = spirit::x3;
    using namespace parser;
    using func_type = Func*;
    using return_type = typename function_traits<Func>::result_type;
    std::string return_type_name = get_name<return_type>();


    auto matcher =
                -(visibility >> static_ >> x3::space) >> //it may be a static class-member, which does however not have the static attribute.
                parser::type_rule<return_type>(return_type_name) >>  x3::space >>
                cdecl_ >> //cdecl declaration for methods. stdcall cannot be
                name >> x3::lit('(') >> parser::arg_list(*this, func_type()) >> x3::lit(')') >>  parser::ptr_rule();


    auto predicate = [&](const mangled_storage_base::entry & e)
            {
                if (e.demangled == name)//maybe not mangled,
                    return true;

                auto itr = e.demangled.begin();
                auto end = e.demangled.end();
                auto res = x3::parse(itr, end, matcher);

                return res && (itr == end);
            };

    auto found = std::find_if(storage_.begin(), storage_.end(), predicate);

    if (found != storage_.end())
        return found->mangled;
    else
        return "";

}

template<typename Class, typename Func>
std::string mangled_storage_impl::get_mem_fn(const std::string &name) const
{
    namespace x3 = spirit::x3;
    using namespace parser;
    using func_type = Func*;
    using return_type = typename function_traits<Func>::result_type;
    auto return_type_name = get_name<return_type>();


    auto cname = get_name<Class>();

    auto matcher =
                visibility >> -virtual_ >> x3::space >>
                parser::type_rule<return_type>(return_type_name) >>  x3::space >>
                thiscall >> //cdecl declaration for methods. stdcall cannot be
                cname >> "::" >> name >>
                x3::lit('(') >> parser::arg_list(*this, func_type()) >> x3::lit(')') >>
                inv_const_rule<Class>() >> inv_volatile_rule<Class>() >> parser::ptr_rule();

    auto predicate = [&](const mangled_storage_base::entry & e)
            {
                auto itr = e.demangled.begin();
                auto end = e.demangled.end();
                auto res = x3::parse(itr, end, matcher);

                return res && (itr == end);
            };

    auto found = std::find_if(storage_.begin(), storage_.end(), predicate);

    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}


template<typename Signature>
auto mangled_storage_impl::get_constructor() const -> ctor_sym {
    using func_type = Signature*;


    std::string ctor_name; // = class_name + "::" + name;
    std::string unscoped_cname; //the unscoped class-name
    {
        auto class_name = get_return_type(dummy<Signature>());
        auto pos = class_name.rfind("::");
        if (pos == std::string::npos)
        {
            ctor_name = class_name+ "::" + class_name ;
            unscoped_cname = class_name;
        }
        else
        {
            unscoped_cname = class_name.substr(pos+2) ;
            ctor_name = class_name+ "::" + unscoped_cname;
        }
    }

    const auto f = std::find_if(storage_.begin(), storage_.end(), parser::is_constructor_with_name<func_type>(std::move(ctor_name), *this));

    if (f != storage_.end())
        return f->mangled;
    else
        return "";
}

template<typename Class>
auto mangled_storage_impl::get_destructor() const -> dtor_sym {
    std::string dtor_name; // = class_name + "::" + name;
    std::string unscoped_cname; //the unscoped class-name
    {
        auto class_name = get_name<Class>();
        auto pos = class_name.rfind("::");
        if (pos == std::string::npos)
        {
            dtor_name = class_name+ "::~" + class_name  + "(void)";
            unscoped_cname = class_name;
        }
        else
        {
            unscoped_cname = class_name.substr(pos+2) ;
            dtor_name = class_name+ "::~" + unscoped_cname + "(void)";
        }
    }

    const auto found = std::find_if(storage_.begin(), storage_.end(), parser::is_destructor_with_name(std::move(dtor_name)));

    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}

template<typename T>
std::string mangled_storage_impl::get_vtable() const
{
    std::string id = "const " + get_name<T>() + "::`vftable'";

    auto predicate = [&](const mangled_storage_base::entry & e)
                {
                    return e.demangled == id;
                };

    auto found = std::find_if(storage_.begin(), storage_.end(), predicate);


    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}

template<typename T>
std::vector<std::string> mangled_storage_impl::get_related() const
{
    std::vector<std::string> ret;
    auto name = get_name<T>();

    for (auto & c : storage_)
    {
        if (c.demangled.find(name) != std::string::npos)
            ret.push_back(c.demangled);
    }

    return ret;
}


}}}



#endif /* BOOST_DLL_DETAIL_DEMANGLING_MSVC_HPP_ */
