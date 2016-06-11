/** \file
 *
 *  Copyright (c) 2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/


#include "test.hpp"

#include <jsonv/parse.hpp>
#include <jsonv/serialization_builder.hpp>
#include <jsonv/serialization_optional.hpp>

#include <set>
#include <sstream>
#include <tuple>
#include <vector>

namespace jsonv_test
{

using namespace jsonv;

namespace
{

struct person
{
    person() = default;

    person(std::string       f,
           std::string       l,
           int               a,
           std::set<long>    favorite_numbers = std::set<long>{},
           std::vector<long> winning_numbers  = std::vector<long>{},
           optional<std::string>  m = nullopt
          ) :
            firstname(std::move(f)),
            middle_name(m),
            lastname(std::move(l)),
            age(a),
            favorite_numbers(std::move(favorite_numbers)),
            winning_numbers(std::move(winning_numbers))
    { }

    std::string       firstname;
    optional<std::string>  middle_name;
    std::string       lastname;
    int               age;
    std::set<long>    favorite_numbers;
    std::vector<long> winning_numbers;

    bool operator==(const person& other) const
    {
        return std::tie(firstname,       middle_name,       lastname,       age,       favorite_numbers,       winning_numbers)
            == std::tie(other.firstname, other.middle_name, other.lastname, other.age, other.favorite_numbers, other.winning_numbers);
    }

    friend std::ostream& operator<<(std::ostream& os, const person& p)
    {
        return os << p.firstname << " " << (p.middle_name ? *p.middle_name+" " : "") << p.lastname << " (" << p.age << ")";
    }

    friend std::string to_string(const person& p)
    {
        std::ostringstream os;
        os << p;
        return os.str();
    }
};

}

TEST(serialization_builder_members)
{
    formats base = formats_builder()
                    .type<person>()
                        .member("firstname", &person::firstname)
                        .member("middle_name", &person::middle_name)
                        .member("lastname",  &person::lastname)
                        .member("age",       &person::age)
                        .register_container<optional<std::string>>()
                    .check_references(formats::defaults())
                ;
    formats fmt = formats::compose({ base, formats::defaults() });

    person p("Bob", "Builder", 29);
    to_string(p);
    value expected = object({ { "firstname",    p.firstname },
                              { "middle_name",  jsonv::null },
                              { "lastname",     p.lastname  },
                              { "age",          p.age       }
                            }
                           );
    value encoded = to_json(p, fmt);
    ensure_eq(expected, encoded);

    person q = extract<person>(encoded, fmt);
    ensure_eq(expected, encoded);
}

TEST(serialization_builder_members_since)
{
    using my_pair = std::pair<int, int>;

    formats base =
        formats_builder()
            .type<my_pair>()
                .member("a", &my_pair::first)
                .member("b", &my_pair::second)
                    .since({ 2, 0 })
        ;
    formats fmt = formats::compose({ base, formats::defaults() });

    auto to_json_ver = [&fmt] (const version& v)
                       {
                        serialization_context context(fmt, v);
                        return context.to_json(my_pair(5, 10));
                       };

    ensure_eq(0U, to_json_ver({ 1, 0 }).count("b"));
    ensure_eq(1U, to_json_ver({ 2, 0 }).count("b"));
    ensure_eq(1U, to_json_ver({ 3, 0 }).count("b"));
}

TEST(serialization_builder_container_members)
{
    formats base = formats_builder()
                    .type<person>()
                        .member("firstname",        &person::firstname)
                        .member("lastname",         &person::lastname)
                        .member("age",              &person::age)
                        .member("favorite_numbers", &person::favorite_numbers)
                        .member("winning_numbers",  &person::winning_numbers)
                    #if JSONV_COMPILER_SUPPORTS_TEMPLATE_TEMPLATES
                    .register_containers<long, std::set, std::vector>()
                    #else
                    .register_container<std::set<long>>()
                    .register_container<std::vector<long>>()
                    #endif
                    .check_references(formats::defaults())
                ;
    formats fmt = formats::compose({ base, formats::defaults() });

    person p("Bob", "Builder", 29, { 1, 2, 3, 4 }, { 5, 6, 7, 8 });
    value expected = object({ { "firstname",        p.firstname          },
                              { "lastname",         p.lastname           },
                              { "age",              p.age                },
                              { "favorite_numbers", array({ 1, 2, 3, 4 })},
                              { "winning_numbers",  array({ 5, 6, 7, 8 })},
                            }
                           );
    auto encoded = to_json(p, fmt);
    ensure_eq(expected, encoded);
    person q = extract<person>(encoded, fmt);
    ensure_eq(p, q);
}

TEST(serialization_builder_extract_extra_keys)
{
    std::set<std::string> extra_keys;
    auto extra_keys_handler = [&extra_keys] (const extraction_context&, const value&, std::set<std::string> x)
                              {
                                  extra_keys = std::move(x);
                              };

    formats base = formats_builder()
                    .type<person>()
                        .member("firstname", &person::firstname)
                        .member("lastname",  &person::lastname)
                        .member("age",       &person::age)
                        .on_extract_extra_keys(extra_keys_handler)
                    .check_references(formats::defaults())
                ;
    formats fmt = formats::compose({ base, formats::defaults() });

    person p("Bob", "Builder", 29);
    value encoded = object({ { "firstname", p.firstname },
                             { "lastname",  p.lastname  },
                             { "age",       p.age       },
                             { "extra1",    10          },
                             { "extra2",    "pie"       }
                           }
                          );

    person q = extract<person>(encoded, fmt);
    ensure_eq(p, q);
    ensure(extra_keys == std::set<std::string>({ "extra1", "extra2" }));
}

TEST(serialization_builder_defaults)
{
    formats base = formats_builder()
                    .type<person>()
                        .member("firstname",        &person::firstname)
                        .member("lastname",         &person::lastname)
                        .member("age",              &person::age)
                            .default_value(20)
                        .member("favorite_numbers", &person::favorite_numbers)
                        .member("winning_numbers",  &person::winning_numbers)
                            .default_value([] (const extraction_context& cxt, const value& val)
                                           {
                                               return cxt.extract_sub<std::vector<long>>(val, "favorite_numbers");
                                           }
                                          )
                            .default_on_null()
                    #if JSONV_COMPILER_SUPPORTS_TEMPLATE_TEMPLATES
                    .register_containers<long, std::set, std::vector>()
                    #else
                    .register_container<std::set<long>>()
                    .register_container<std::vector<long>>()
                    #endif
                    .check_references(formats::defaults())
                ;
    formats fmt = formats::compose({ base, formats::defaults() });

    person p("Bob", "Builder", 20, { 1, 2, 3, 4 }, { 1, 2, 3, 4 });
    value input = object({ { "firstname",        p.firstname          },
                           { "lastname",         p.lastname           },
                           { "favorite_numbers", array({ 1, 2, 3, 4 })},
                           { "winning_numbers",  null                 },
                         }
                        );
    auto encoded = to_json(p, fmt);
    person q = extract<person>(encoded, fmt);
    ensure_eq(p, q);
}

TEST(serialization_builder_encode_checks)
{
    formats base = formats_builder()
                    .type<person>()
                        .member("firstname",        &person::firstname)
                        .member("lastname",         &person::lastname)
                        .member("age",              &person::age)
                            .encode_if([] (const serialization_context&, int age) { return age > 20; })
                        .member("favorite_numbers", &person::favorite_numbers)
                            .encode_if([] (const serialization_context&, const std::set<long>& nums) { return nums.size(); })
                        .member("winning_numbers",  &person::winning_numbers)
                            .encode_if([] (const serialization_context&, const std::vector<long>& nums) { return nums.size(); })
                    #if JSONV_COMPILER_SUPPORTS_TEMPLATE_TEMPLATES
                    .register_containers<long, std::set, std::vector>()
                    #else
                    .register_container<std::set<long>>()
                    .register_container<std::vector<long>>()
                    #endif
                    .check_references(formats::defaults())
                ;
    formats fmt = formats::compose({ base, formats::defaults() });

#ifdef __APPLE__
    person p("Bob", "Builder", 20, std::set<long>{}, { 1 });
#else
    person p("Bob", "Builder", 20, {}, { 1 });
#endif
    value expected = object({ { "firstname", p.firstname },
                              { "lastname",  p.lastname  },
                              { "winning_numbers", array({ 1 }) },
                            }
                           );
    value encoded = to_json(p, fmt);
    ensure_eq(expected, encoded);
}

TEST(serialization_builder_check_references_fails)
{
    formats_builder builder;
    builder.reference_type(std::type_index(typeid(int)));
    builder.reference_type(std::type_index(typeid(long)), std::type_index(typeid(person)));
    ensure_throws(std::logic_error, builder.check_references(formats(), "test"));
}

namespace
{

struct foo
{
  int         a;
  int         b;
  std::string c;
};

struct bar
{
  foo         x;
  foo         y;
  std::string z;
  std::string w;
};

TEST(serialization_builder_extra_unchecked_key)
{
    jsonv::formats local_formats =
        jsonv::formats_builder()
            .type<foo>()
               .member("a", &foo::a)
               .member("b", &foo::b)
                   .default_value(10)
                   .default_on_null()
               .member("c", &foo::c)
            .type<bar>()
               .member("x", &bar::x)
               .member("y", &bar::y)
               .member("z", &bar::z)
                   .since(jsonv::version(2, 0))
               .member("w", &bar::w)
                   .until(jsonv::version(5, 0))
    ;
    jsonv::formats format = jsonv::formats::compose({ jsonv::formats::defaults(), local_formats });
    
    jsonv::value val = object({ { "x", object({ { "aaaaa", 50 }, { "b", 20 }, { "c", "Blah"  } }) },
                                { "y", object({ { "a",     10 },              { "c", "No B?" } }) },
                                { "z", "Only serialized in 2.0+" },
                                { "w", "Only serialized before 5.0" }
                              }
                             );
    bar x = jsonv::extract<bar>(val, format);  
}

TEST(serialization_builder_extra_unchecked_key_throws)
{
    jsonv::formats local_formats =
        jsonv::formats_builder()
            .type<foo>()
               .on_extract_extra_keys(jsonv::throw_extra_keys_extraction_error)
               .member("a", &foo::a)
               .member("b", &foo::b)
                   .default_value(10)
                   .default_on_null()
               .member("c", &foo::c)
            .type<bar>()
               .member("x", &bar::x)
               .member("y", &bar::y)
               .member("z", &bar::z)
                   .since(jsonv::version(2, 0))
               .member("w", &bar::w)
                   .until(jsonv::version(5, 0))
    ;
    jsonv::formats format = jsonv::formats::compose({ jsonv::formats::defaults(), local_formats });
    
    jsonv::value val = object({ { "x", object({ { "aaaaa", 50 }, { "b", 20 }, { "c", "Blah"  } }) },
                                { "y", object({ { "a",     10 },              { "c", "No B?" } }) },
                                { "z", "Only serialized in 2.0+" },
                                { "w", "Only serialized before 5.0" }
                              }
                             );
    try
    {
        jsonv::extract<bar>(val, format);
        throw std::runtime_error("Should have thrown an extraction_error");
    }
    catch (const extraction_error& err)
    {
        ensure_eq(path({"x"}), err.path());
    }
}

}

namespace x
{

enum class ring
{
    fire,
    wind,
    water,
    earth,
    heart,
};

}

TEST(serialization_builder_enum_strings)
{
    using namespace x;
    
    jsonv::formats formats =
        jsonv::formats_builder()
            .enum_type<ring>("ring",
                             {
                               { ring::fire,  "fire"  },
                               { ring::wind,  "wind"  },
                               { ring::water, "water" },
                               { ring::earth, "earth" },
                               { ring::heart, "heart" },
                             }
                            )
            #if JSONV_COMPILER_SUPPORTS_TEMPLATE_TEMPLATES
            .register_containers<ring, std::vector>()
            #else
            .register_container<std::vector<ring>>()
            #endif
            .check_references(jsonv::formats::defaults());
    
    ensure(ring::fire  == jsonv::extract<ring>("fire",  formats));
    ensure(ring::wind  == jsonv::extract<ring>("wind",  formats));
    ensure(ring::water == jsonv::extract<ring>("water", formats));
    ensure(ring::earth == jsonv::extract<ring>("earth", formats));
    ensure(ring::heart == jsonv::extract<ring>("heart", formats));
    
    jsonv::value jsons = jsonv::array({ "fire", "wind", "water", "earth", "heart" });
    std::vector<ring> exp = { ring::fire, ring::wind, ring::water, ring::earth, ring::heart };
    std::vector<ring> val = jsonv::extract<std::vector<ring>>(jsons, formats);
    ensure(val == exp);
    
    value enc = jsonv::to_json(exp, formats);
    ensure(enc == jsons);
    
    ensure_throws(jsonv::extraction_error, jsonv::extract<ring>("FIRE",    formats));
    ensure_throws(jsonv::extraction_error, jsonv::extract<ring>("useless", formats));
}

TEST(serialization_builder_enum_strings_icase)
{
    using namespace x;
    
    jsonv::formats formats =
        jsonv::formats_builder()
            .enum_type_icase<ring>("ring",
                             {
                               { ring::fire,  "fire"  },
                               { ring::wind,  "wind"  },
                               { ring::water, "water" },
                               { ring::earth, "earth" },
                               { ring::heart, "heart" },
                             }
                            )
            #if JSONV_COMPILER_SUPPORTS_TEMPLATE_TEMPLATES
            .register_containers<ring, std::vector>()
            #else
            .register_container<std::vector<ring>>()
            #endif
            .check_references(jsonv::formats::defaults());
    
    ensure(ring::fire  == jsonv::extract<ring>("fiRe",  formats));
    ensure(ring::wind  == jsonv::extract<ring>("wIND",  formats));
    ensure(ring::water == jsonv::extract<ring>("Water", formats));
    ensure(ring::earth == jsonv::extract<ring>("EARTH", formats));
    ensure(ring::heart == jsonv::extract<ring>("HEART", formats));
    
    jsonv::value jsons = jsonv::array({ "fire", "wind", "water", "earth", "heart" });
    std::vector<ring> exp = { ring::fire, ring::wind, ring::water, ring::earth, ring::heart };
    std::vector<ring> val = jsonv::extract<std::vector<ring>>(jsons, formats);
    ensure(val == exp);
    
    value enc = jsonv::to_json(exp, formats);
    ensure(enc == jsons);
    
    ensure_throws(jsonv::extraction_error, jsonv::extract<ring>("useless", formats));
}

TEST(serialization_builder_enum_strings_icase_multimapping)
{
    using namespace x;
    
    jsonv::formats formats =
        jsonv::formats_builder()
            .enum_type_icase<ring>("ring",
                             {
                               { ring::fire,  "fire"  },
                               { ring::fire,  666     },
                               { ring::wind,  "wind"  },
                               { ring::water, "water" },
                               { ring::earth, "earth" },
                               { ring::earth, true    },
                               { ring::heart, "heart" },
                               { ring::heart, "useless" },
                             }
                            )
             #if JSONV_COMPILER_SUPPORTS_TEMPLATE_TEMPLATES
             .register_containers<ring, std::vector>()
             #else
             .register_container<std::vector<ring>>()
             #endif
            .check_references(jsonv::formats::defaults());
    
    ensure(ring::fire  == jsonv::extract<ring>("fiRe",  formats));
    ensure(ring::fire  == jsonv::extract<ring>(666,     formats));
    ensure(ring::wind  == jsonv::extract<ring>("wIND",  formats));
    ensure(ring::water == jsonv::extract<ring>("Water", formats));
    ensure(ring::earth == jsonv::extract<ring>("EARTH", formats));
    ensure(ring::earth == jsonv::extract<ring>(true, formats));
    ensure(ring::heart == jsonv::extract<ring>("HEART", formats));
    ensure(ring::heart == jsonv::extract<ring>("useless", formats));
    
    jsonv::value jsons = jsonv::array({ "fire", "wind", "water", "earth", "heart" });
    std::vector<ring> exp = { ring::fire, ring::wind, ring::water, ring::earth, ring::heart };
    std::vector<ring> val = jsonv::extract<std::vector<ring>>(jsons, formats);
    ensure(val == exp);
    
    value enc = jsonv::to_json(exp, formats);
    ensure(enc == jsons);
    
    ensure_throws(jsonv::extraction_error, jsonv::extract<ring>(false, formats));
    ensure_throws(jsonv::extraction_error, jsonv::extract<ring>(5,     formats));
}

namespace
{

struct base
{
    virtual std::string get() const = 0;
};

struct a_derived :
        base
{
    virtual std::string get() const override { return "a"; }
    
    static void json_adapt(adapter_builder<a_derived>& builder)
    {
        builder.member("type", &a_derived::x);
    }
    
    std::string x = "a";
};

struct b_derived :
        base
{
    virtual std::string get() const override { return "b"; }
    
    static void json_adapt(adapter_builder<b_derived>& builder)
    {
        builder.member("type", &b_derived::x);
    }
    
    std::string x = "b";
};

}

TEST(serialization_builder_polymorphic_direct)
{
    formats fmts =
        formats::compose
        ({
            formats_builder()
                .polymorphic_type<std::unique_ptr<base>>("type")
                    .subtype<a_derived>("a")
                    .subtype<b_derived>("b")
                .type<a_derived>(a_derived::json_adapt)
                .type<b_derived>(b_derived::json_adapt)
                .register_container<std::vector<std::unique_ptr<base>>>()
                .check_references(formats::defaults()),
            formats::defaults()
        });
    
    value input = array({ object({{ "type", "a" }}), object({{ "type", "b" }}) });
    auto output = extract<std::vector<std::unique_ptr<base>>>(input, fmts);
    
    ensure(output.at(0)->get() == "a");
    ensure(output.at(1)->get() == "b");
    
    value encoded = to_json(output, fmts);
    ensure_eq(input, encoded);
}

}
