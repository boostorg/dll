module;

#include <boost/config.hpp>
#include <boost/assert/source_location.hpp>
#include <boost/core/invoke_swap.hpp>
#include <boost/noncopyable.hpp>
#include <boost/predef/os.h>
#include <boost/throw_exception.hpp>
#include <boost/type_index/ctti_type_index.hpp>

#ifdef BOOST_DLL_USE_STD_FS
#   include <filesystem>
#   include <system_error>
#else
#   include <boost/filesystem/path.hpp>
#   include <boost/filesystem/operations.hpp>
#   include <boost/system/system_error.hpp>
#   include <boost/system/error_code.hpp>
#endif

#if !BOOST_OS_WINDOWS
#   include <dlfcn.h>
#   include <link.h>
#endif

#include <map>
#include <memory>
#include <fstream>

#ifndef _MSC_VER
#include <boost/core/demangle.hpp>
#endif

#define BOOST_DLL_INTERFACE_UNIT

export module boost.dll;

#ifdef __clang__
#   pragma clang diagnostic ignored "-Winclude-angled-in-module-purview"
#endif

#include <boost/dll.hpp>
#include <boost/dll/import_class.hpp>
#include <boost/dll/import_mangled.hpp>
