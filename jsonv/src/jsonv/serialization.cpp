/** \file
 *  Conversion between C++ types and JSON values.
 *
 *  Copyright (c) 2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#include <jsonv/serialization.hpp>
#include <jsonv/coerce.hpp>
#include <jsonv/demangle.hpp>
#include <jsonv/serialization_util.hpp>
#include <jsonv/value.hpp>

#include <cstdint>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace jsonv
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// extractor                                                                                                          //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extractor::~extractor() noexcept = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// serializer                                                                                                         //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

serializer::~serializer() noexcept = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// adapter                                                                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

adapter::~adapter() noexcept = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// extraction_error                                                                                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string make_extraction_error_errmsg(const extraction_context& context, const std::string& message)
{
    std::ostringstream os;
    os << "Extraction error";

    if (!context.path().empty())
        os << " at " << context.path();

    if (!message.empty())
        os << ": " << message;

    return os.str();
}

extraction_error::extraction_error(const extraction_context& context, const std::string& message) :
        std::runtime_error(make_extraction_error_errmsg(context, message)),
        nested_exception(),
        _path(context.path())
{ }

extraction_error::~extraction_error() noexcept = default;

const path& extraction_error::path() const
{
    return _path;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// no_extractor                                                                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static std::string make_no_serializer_extractor_errmsg(const char* kind, const std::type_index& type)
{
    std::ostringstream ss;
    ss << "Could not find " << kind << " for type: " << demangle(type.name());
    return ss.str();
}

no_extractor::no_extractor(const std::type_index& type) :
        runtime_error(make_no_serializer_extractor_errmsg("extractor", type)),
        _type_index(type),
        _type_name(demangle(type.name()))
{ }

no_extractor::no_extractor(const std::type_info& type) :
        no_extractor(std::type_index(type))
{ }

no_extractor::~no_extractor() noexcept
{ }

std::type_index no_extractor::type_index() const
{
    return _type_index;
}

string_view no_extractor::type_name() const
{
    return _type_name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// no_serializer                                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

no_serializer::no_serializer(const std::type_index& type) :
        runtime_error(make_no_serializer_extractor_errmsg("serializer", type)),
        _type_index(type),
        _type_name(demangle(type.name()))
{ }

no_serializer::no_serializer(const std::type_info& type) :
        no_serializer(std::type_index(type))
{ }

no_serializer::~no_serializer() noexcept
{ }

std::type_index no_serializer::type_index() const
{
    return _type_index;
}

string_view no_serializer::type_name() const
{
    return _type_name;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// formats::data                                                                                                      //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct JSONV_LOCAL formats::data
{
public:
    using roots_list      = std::vector<std::shared_ptr<const data>>;
    using extractor_map   = std::unordered_map<std::type_index, const extractor*>;
    using serializer_map  = std::unordered_map<std::type_index, const serializer*>;
    using owned_items_set = std::unordered_set<std::shared_ptr<const void>>;

public:
    /// The previous data this comes from...this allows us to make a huge tree of formats with custom extension points.
    roots_list roots;

    extractor_map extractors;

    owned_items_set owned_items;

    serializer_map serializers;

    explicit data(roots_list roots) :
            roots(std::move(roots))
    { }

public:
    const extractor* find_extractor(const std::type_index& typeidx) const
    {
        return find_impl<extractor>(this,
                                    typeidx,
                                    [] (const data* self) -> const extractor_map& { return self->extractors; }
                                   );
    }

    const serializer* find_serializer(const std::type_index& typeidx) const
    {
        return find_impl<serializer>(this,
                                     typeidx,
                                     [] (const data* self) -> const serializer_map& { return self->serializers; }
                                    );
    }

    template <typename T, typename FSelectMap>
    static const T* find_impl(const data* self,
                              const std::type_index& typeidx,
                              const FSelectMap&      select_map
                             )
    {
        const auto& map = select_map(self);
        auto iter = map.find(typeidx);
        if (iter != std::end(map))
        {
            return iter->second;
        }
        else
        {
            for (const auto& sub : self->roots)
            {
                auto ptr = find_impl<T>(sub.get(), typeidx, select_map);
                if (ptr)
                    return ptr;
            }
            return nullptr;
        }
    }

public:
    extractor_map::iterator insert_extractor(const extractor* ex)
    {
        std::type_index typeidx(ex->get_type());
        auto iter = extractors.find(typeidx);
        if (iter != end(extractors))
        {
            // if we already have a value, search the shared maps to see if it is present
            std::ostringstream os;
            os << "Already have an extractor for type " << demangle(typeidx.name());
            throw std::invalid_argument(os.str());
        }
        else
        {
            return extractors.emplace(typeidx, ex).first;
        }
    }

    void insert_extractor(std::shared_ptr<const extractor> ex)
    {
        auto iter = insert_extractor(ex.get());
        auto rollback = detail::on_scope_exit([this, &iter] { extractors.erase(iter); });
        owned_items.insert(std::move(ex));
        rollback.release();
    }

    serializer_map::iterator insert_serializer(const serializer* ser)
    {
        std::type_index typeidx(ser->get_type());
        auto iter = serializers.find(typeidx);
        if (iter != end(serializers))
        {
            // if we already have a value, search the shared maps to see if it is present
            std::ostringstream os;
            os << "Already have a serializer for type " << demangle(typeidx.name());
            throw std::invalid_argument(os.str());
        }
        else
        {
            return serializers.emplace(typeidx, ser).first;
        }
    }

    void insert_serializer(std::shared_ptr<const serializer> ser)
    {
        auto iter = insert_serializer(ser.get());
        auto rollback = detail::on_scope_exit([this, &iter] { serializers.erase(iter); });
        owned_items.insert(std::move(ser));
        rollback.release();
    }

    void insert_adapter(const adapter* adp)
    {
        auto iter = insert_extractor(adp);
        auto rollback = detail::on_scope_exit([this, &iter] { extractors.erase(iter); });
        insert_serializer(adp);
        rollback.release();
    }

    void insert_adapter(std::shared_ptr<const adapter> adp)
    {
        auto iter_ex = insert_extractor(adp.get());
        auto rollback_ex = detail::on_scope_exit([this, &iter_ex] { extractors.erase(iter_ex); });
        auto iter_ser = insert_serializer(adp.get());
        auto rollback_ser = detail::on_scope_exit([this, &iter_ser] { serializers.erase(iter_ser); });
        owned_items.insert(std::move(adp));
        rollback_ex.release();
        rollback_ser.release();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// formats                                                                                                            //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

formats::formats(data::roots_list roots) :
        _data(std::make_shared<data>(std::move(roots)))
{ }

formats::formats() :
        formats(data::roots_list())
{ }

formats formats::compose(const list& bases)
{
    data::roots_list roots;
    roots.reserve(bases.size());
    std::transform(begin(bases), end(bases),
                   std::back_inserter(roots),
                   [] (const formats& fmt) { return fmt._data; }
                  );
    return formats(std::move(roots));
}

formats::~formats() noexcept
{ }

const extractor& formats::get_extractor(std::type_index type) const
{
    const extractor* ex = _data->find_extractor(type);
    if (ex)
        return *ex;
    else
        throw no_extractor(type);
}

const extractor& formats::get_extractor(const std::type_info& type) const
{
    return get_extractor(std::type_index(type));
}

void formats::extract(const std::type_info&     type,
                      const value&              from,
                      void*                     into,
                      const extraction_context& context
                     ) const
{
    get_extractor(type).extract(context, from, into);
}

const serializer& formats::get_serializer(std::type_index type) const
{
    const serializer* ser = _data->find_serializer(type);
    if (ser)
        return *ser;
    else
        throw no_serializer(type);
}

const serializer& formats::get_serializer(const std::type_info& type) const
{
    return get_serializer(std::type_index(type));
}

value formats::to_json(const std::type_info& type,
                       const void* from,
                       const serialization_context& context
                      ) const
{
    return get_serializer(type).to_json(context, from);
}

void formats::register_extractor(const extractor* ex)
{
    _data->insert_extractor(ex);
}

void formats::register_extractor(std::shared_ptr<const extractor> ex)
{
    _data->insert_extractor(std::move(ex));
}

void formats::register_serializer(const serializer* ser)
{
    _data->insert_serializer(ser);
}

void formats::register_serializer(std::shared_ptr<const serializer> ser)
{
    _data->insert_serializer(std::move(ser));
}

void formats::register_adapter(const adapter* adp)
{
    _data->insert_adapter(adp);
}

void formats::register_adapter(std::shared_ptr<const adapter> adp)
{
    _data->insert_adapter(std::move(adp));
}

bool formats::operator==(const formats& other) const
{
    return _data.get() == other._data.get();
}

bool formats::operator!=(const formats& other) const
{
    return !operator==(other);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// formats::defaults                                                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
static void register_integer_adapter(formats& fmt)
{
    static auto instance = make_adapter([] (const value& from) { return T(from.as_integer()); },
                                        [] (const T& from) { return value(static_cast<std::int64_t>(from)); }
                                       );
    fmt.register_adapter(&instance);
}

static formats create_default_formats()
{
    formats fmt;

    static auto json_extractor = make_adapter([] (const value& from) { return from; },
                                              [] (const value& from) { return from; }
                                             );
    fmt.register_adapter(&json_extractor);

    static auto string_extractor = make_adapter([] (const value& from) { return from.as_string(); },
                                                [] (const std::string& from) { return value(from); }
                                               );
    fmt.register_adapter(&string_extractor);

    static auto cchar_ptr_serializer = make_serializer<const char*>([] (const char* from) { return value(from); });
    fmt.register_serializer(&cchar_ptr_serializer);
    static auto char_ptr_serializer = make_serializer<char*>([] (char* from) { return value(from); });
    fmt.register_serializer(&char_ptr_serializer);

    static auto bool_extractor = make_adapter([] (const value& from) { return from.as_boolean(); },
                                              [] (const bool& from) { return value(from); }
                                             );
    fmt.register_adapter(&bool_extractor);

    register_integer_adapter<std::int8_t>(fmt);
    register_integer_adapter<std::uint8_t>(fmt);
    register_integer_adapter<std::int16_t>(fmt);
    register_integer_adapter<std::uint16_t>(fmt);
    register_integer_adapter<std::int32_t>(fmt);
    register_integer_adapter<std::uint32_t>(fmt);
    register_integer_adapter<std::int64_t>(fmt);
    register_integer_adapter<std::uint64_t>(fmt);
    #if defined(__APPLE__)
    register_integer_adapter<std::size_t>(fmt);
    register_integer_adapter<long>(fmt);
    #elif defined(_MSC_VER)
    // In MSVC's 64-bit compiler, `std::int64_t` is a distinct type from `long`. Since these are really common types,
    // we will add them explicitly.
    register_integer_adapter<long>(fmt);
    register_integer_adapter<unsigned long>(fmt);
    #endif

    static auto double_extractor = make_adapter([] (const value& from) { return from.as_decimal(); },
                                                [] (const double& from) { return value(from); }
                                               );
    fmt.register_adapter(&double_extractor);
    static auto float_extractor = make_adapter([] (const value& from) { return float(from.as_decimal()); },
                                               [] (const float& from) { return value(from); }
                                              );
    fmt.register_adapter(&float_extractor);

    return fmt;
}

static const formats& default_formats_ref()
{
    static formats instance = create_default_formats();
    return instance;
}

formats formats::defaults()
{
    return formats::compose({ default_formats_ref() });
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// formats::global                                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static formats& global_formats_ref()
{
    static formats instance = default_formats_ref();
    return instance;
}

formats formats::global()
{
    return formats::compose({ global_formats_ref() });
}

void formats::set_global(formats fmt)
{
    global_formats_ref() = std::move(fmt);
}

void formats::reset_global()
{
    global_formats_ref() = default_formats_ref();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// formats::coerce                                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
static void register_integer_coerce_extractor(formats& fmt)
{
    static auto instance = make_extractor([] (const value& from) { return T(coerce_integer(from)); });
    fmt.register_extractor(&instance);
}

static formats create_coerce_formats()
{
    formats fmt;

    static auto string_extractor = make_extractor([] (const value& from) { return coerce_string(from); });
    fmt.register_extractor(&string_extractor);

    static auto bool_extractor = make_extractor([] (const value& from) { return coerce_boolean(from); });
    fmt.register_extractor(&bool_extractor);

    register_integer_coerce_extractor<std::int8_t>(fmt);
    register_integer_coerce_extractor<std::uint8_t>(fmt);
    register_integer_coerce_extractor<std::int16_t>(fmt);
    register_integer_coerce_extractor<std::uint16_t>(fmt);
    register_integer_coerce_extractor<std::int32_t>(fmt);
    register_integer_coerce_extractor<std::uint32_t>(fmt);
    register_integer_coerce_extractor<std::int64_t>(fmt);
    register_integer_coerce_extractor<std::uint64_t>(fmt);

    static auto double_extractor = make_extractor([] (const value& from) { return coerce_decimal(from); });
    fmt.register_extractor(&double_extractor);
    static auto float_extractor = make_extractor([] (const value& from) { return float(coerce_decimal(from)); });
    fmt.register_extractor(&float_extractor);

    return fmt;
}

static const formats& coerce_formats_ref()
{
    static formats instance = formats::compose({ create_coerce_formats(), default_formats_ref() });
    return instance;
}

formats formats::coerce()
{
    return formats::compose({ coerce_formats_ref() });
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// context_base                                                                                                       //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

context_base::context_base(jsonv::formats        fmt,
                           const jsonv::version& ver,
                           const void*           userdata
                          ) :
        _formats(std::move(fmt)),
        _version(ver),
        _user_data(userdata)
{ }

context_base::context_base() :
        context_base(formats::global())
{ }

context_base::~context_base() noexcept = default;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// extraction_context                                                                                                 //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extraction_context::extraction_context(jsonv::formats        fmt,
                                       const jsonv::version& ver,
                                       jsonv::path           p,
                                       const void*           userdata
                                      ) :
        context_base(std::move(fmt), ver, userdata),
        _path(std::move(p))
{ }

extraction_context::extraction_context() :
        context_base()
{ }

extraction_context::~extraction_context() noexcept = default;

void extraction_context::extract(const std::type_info& type, const value& from, void* into) const
{
    try
    {
        formats().extract(type, from, into, *this);
    }
    catch (const extraction_error&)
    {
        throw;
    }
    catch (const std::exception& ex)
    {
        throw extraction_error(*this, ex.what());
    }
    catch (...)
    {
        throw extraction_error(*this, "");
    }
}

void extraction_context::extract_sub(const std::type_info& type,
                                     const value&          from,
                                     jsonv::path           subpath,
                                     void*                 into
                                    ) const
{
    extraction_context sub(*this);
    sub._path += subpath;
    try
    {
        return sub.extract(type, from.at_path(subpath), into);
    }
    catch (const extraction_error&)
    {
        throw;
    }
    catch (const std::exception& ex)
    {
        throw extraction_error(sub, ex.what());
    }
    catch (...)
    {
        throw extraction_error(sub, "");
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// serialization_context                                                                                              //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

serialization_context::serialization_context(jsonv::formats        fmt,
                                             const jsonv::version& ver,
                                             const void*           userdata
                                            ) :
        context_base(std::move(fmt), ver, userdata)
{ }

serialization_context::serialization_context() :
        context_base()
{ }

serialization_context::~serialization_context() noexcept = default;

value serialization_context::to_json(const std::type_info& type, const void* from) const
{
    return formats().to_json(type, from, *this);
}

}
