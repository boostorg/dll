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

namespace boost { namespace dll { namespace detail {

class mangled_storage_impl  : public mangled_storage_base
{
    template<typename T>
    struct dummy {};

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


namespace parser {

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

    inline bool consume_visibility(boost::core::string_view& s) {
        if (s.starts_with("public:")) {
            s.remove_prefix(sizeof("public:") - 1);
        } else if (s.starts_with("protected:")) {
            s.remove_prefix(sizeof("protected:") - 1);
        } else if (s.starts_with("private:")) {
            s.remove_prefix(sizeof("private:") - 1);
        } else {
            return false;
        }
        return true;
    }

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

        if (std::is_const<typename std::remove_reference<T>::type>::value) {
            if (!s.starts_with(" const")) {
                return std::string::npos;
            } else {
                s.remove_prefix(sizeof(" const") - 1);
            }
        }

        if (std::is_volatile<typename std::remove_reference<T>::type>::value) {
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

        s = parser::trim_ptrs(s);
        return s_orig.size() - s.size();
    }

    inline bool consume_thiscall(boost::core::string_view& s) {
        if (s.starts_with(" ")) s.remove_prefix(1);

        if (s.starts_with("__cdecl ")) {               // Win 64bit
            s.remove_prefix(sizeof("__cdecl ") - 1);
            return true;
        } else if (s.starts_with("__thiscall ")) {     // Win 32bit
            s.remove_prefix(sizeof("__thiscall ") - 1);
            return true;
        }
        return false;
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
        const std::string& dtor_name_;

    public:
        explicit is_destructor_with_name(const std::string& dtor_name)
            : dtor_name_(dtor_name) {}

        inline bool operator()(boost::core::string_view s) const {
            if (!parser::consume_visibility(s)) {
                return false;
            }
            s = trim_prefix(s, " virtual");
        
            if (!parser::consume_thiscall(s)) {
                return false;
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
        const std::string& variable_name_;
        const mangled_storage_impl& ms_;

    public:
        is_variable_with_name(const std::string& variable_name, const mangled_storage_impl& ms)
            : variable_name_(variable_name), ms_(ms) {}

        inline bool operator()(boost::core::string_view s) const {
            if (parser::consume_visibility(s)) {
                s = parser::trim_prefix(s, " static ");
            }
            {
                const auto type_pos = parser::find_type<T>(ms_, s);
                if (type_pos == std::string::npos) {
                    return false;
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
        const std::string& ctor_name_;
        const mangled_storage_impl& ms_;

    public:
        is_constructor_with_name(const std::string& ctor_name, const mangled_storage_impl& ms)
            : ctor_name_(ctor_name), ms_(ms) {}

        inline bool operator()(boost::core::string_view s) const {
            if (!parser::consume_visibility(s)) {
                return false;
            }
            if (!parser::consume_thiscall(s)) {
                return false;
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
        const std::string& function_name_;
        const mangled_storage_impl& ms_;

    public:
        is_function_with_name(const std::string& function_name, const mangled_storage_impl& ms)
            : function_name_(function_name), ms_(ms) {}

        inline bool operator()(boost::core::string_view s) const {
            if (parser::consume_visibility(s)) {
                s = trim_prefix(s, " static ");
            }
            {
                const auto type_pos = parser::find_type<Result>(ms_, s);
                if (type_pos == std::string::npos) {
                    BOOST_ASSERT(false);
                    return false;
                }
                s.remove_prefix(type_pos);
            }

            if (s.starts_with(" ")) s.remove_prefix(1);
            if (!s.starts_with("__cdecl ")) {
                throw std::runtime_error(s);
                    BOOST_ASSERT(false);
                return false;
            }
            s.remove_prefix(sizeof("__cdecl ") - 1);

            if (!s.starts_with(function_name_)) {
                    BOOST_ASSERT(false);
                return false;
            }
            s.remove_prefix(function_name_.size());

            if (!s.starts_with("(")) {
                    BOOST_ASSERT(false);
                return false;
            }
            s.remove_prefix(1);

            {
                using Signature = Result(*)(Args...);
                const auto arg_list_pos = parser::find_arg_list(ms_, s, Signature());
                if (arg_list_pos == std::string::npos) {
                    BOOST_ASSERT(false);
                    return false;
                }
                s.remove_prefix(arg_list_pos);
            }

            if (!s.starts_with(")")) {
                    BOOST_ASSERT(false);
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

    template <typename Class, typename Func>
    class is_mem_fn_with_name;

    template <typename Class, class Result, class... Args>
    class is_mem_fn_with_name<Class, Result(*)(Args...)> {
        const std::string& function_name_;
        const mangled_storage_impl& ms_;

    public:
        is_mem_fn_with_name(const std::string& function_name, const mangled_storage_impl& ms)
            : function_name_(function_name), ms_(ms) {}

        inline bool operator()(boost::core::string_view s) const {
            if (!parser::consume_visibility(s)) {
                return false;
            }
            s = trim_prefix(s, " virtual");

            if (!s.starts_with(" ")) {
                return false;
            }
            s.remove_prefix(1);

            {
                const auto type_pos = parser::find_type<Result>(ms_, s);
                if (type_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(type_pos);
            }
            if (!parser::consume_thiscall(s)) {
                return false;
            }
            {
                const auto type_pos = parser::find_type<typename std::remove_cv<Class>::type>(ms_, s);
                if (type_pos == std::string::npos) {
                    return false;
                }
                s.remove_prefix(type_pos);
            }

            if (!s.starts_with("::")) {
                return false;
            }
            s.remove_prefix(sizeof("::") - 1);

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

            if (std::is_const<Class>::value) {
                if (!s.starts_with("const ")) {
                    return false;
                } else {
                    s.remove_prefix(sizeof("const ") - 1);
                }
            }

            if (std::is_volatile<Class>::value) {
                if (!s.starts_with("volatile ")) {
                    return false;
                } else {
                    s.remove_prefix(sizeof("volatile ") - 1);
                }
            }

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
    const auto found = std::find_if(storage_.begin(), storage_.end(), parser::is_variable_with_name<T>(name, *this));

    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}

template<typename Func>
std::string mangled_storage_impl::get_function(const std::string &name) const {
    const auto found = std::find_if(storage_.begin(), storage_.end(), parser::is_function_with_name<Func*>(name, *this));

    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}

template<typename Class, typename Func>
std::string mangled_storage_impl::get_mem_fn(const std::string &name) const {
    const auto found = std::find_if(storage_.begin(), storage_.end(), parser::is_mem_fn_with_name<Class, Func*>(name, *this));

    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}


template<typename Signature>
auto mangled_storage_impl::get_constructor() const -> ctor_sym {
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

    const auto f = std::find_if(storage_.begin(), storage_.end(), parser::is_constructor_with_name<Signature*>(ctor_name, *this));

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

    const auto found = std::find_if(storage_.begin(), storage_.end(), parser::is_destructor_with_name(dtor_name));

    if (found != storage_.end())
        return found->mangled;
    else
        return "";
}

template<typename T>
std::string mangled_storage_impl::get_vtable() const {
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
std::vector<std::string> mangled_storage_impl::get_related() const {
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
