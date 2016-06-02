/** \file jsonv/all.hpp
 *  A head which includes all other JSON Voorhees headers.
 *  
 *  Copyright (c) 2012-2015 by Travis Gockel. All rights reserved.
 *
 *  This program is free software: you can redistribute it and/or modify it under the terms of the Apache License
 *  as published by the Apache Software Foundation, either version 2 of the License, or (at your option) any later
 *  version.
 *
 *  \author Travis Gockel (travis@gockelhut.com)
**/
#ifndef __JSONV_ALL_HPP_INCLUDED__
#define __JSONV_ALL_HPP_INCLUDED__

namespace jsonv
{

/** \mainpage Overview
 *  JSON Voorhees is a JSON library written for the C++ programmer who wants to be productive in this modern world. What
 *  does that mean? There are a ton of JSON libraries floating around touting how they are "modern" C++ and so on. But
 *  who really cares? JSON Voorhees puts the focus more on the resulting C++ than any "modern" feature set. This means
 *  the library does not skip on string encoding details like having full support for UTF-8. Are there "modern"
 *  features? Sure, but this library is not meant to be a gallery of them -- a good API should get out of your way and
 *  let you work. It is hosted on <a href="https://github.com/tgockel/json-voorhees">GitHub</a> and sports an Apache
 *  License, so use it anywhere you need.
 * 
 *  Features include (but are not necessarily limited to):
 *  
 *   - Simple
 *     - A `value` should not feel terribly different from a C++ Standard Library container
 *     - Write valid JSON with `operator<<`
 *     - Simple JSON parsing with `parse`
 *     - Reasonable error messages when parsing fails
 *     - Full support for Unicode-filled JSON (encoded in UTF-8 in C++)
 *   - Efficient
 *     - Minimal overhead to store values (a `value` is 16 bytes on a 64-bit platform)
 *     - No-throw move semantics wherever possible
 *   - Easy
 *     - Convert a `value` into a C++ type using `extract<T>`
 *     - Encode a C++ type into a value using `to_json`
 *   - Safe
 *     - In the best case, illegal code should fail to compile
 *     - An illegal action should throw an exception
 *     - Almost all utility functions have a <a href="http://www.gotw.ca/gotw/082.htm">strong exception guarantee</a>.
 *   - Stable
 *     - Worry less about upgrading -- the API and ABI will not change out from under you
 *   - Documented
 *     - Consumable by human beings
 *     - Answers questions you might actually ask
 *  
 *  \dotfile doc/conversions.dot
 *  
 *  JSON Voorhees is designed with ease-of-use in mind. So let's look at some code!
 *  
 *  \section demo_value The jsonv::value
 *  
 *  The central class of JSON Voorhees is the \c jsonv::value. This class represents a JSON AST and is somewhat of a
 *  dynamic type. This can make things a little bit awkward for C++ programmers who are used to static typing. Don't
 *  worry about it -- you can learn to love it.
 *  
 *  Putting values of different types is super-easy.
 *  
 *  \code
 *  #include <jsonv/value.hpp>
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::value x = jsonv::null;
 *      std::cout << x << std::endl;
 *      x = 5.9;
 *      std::cout << x << std::endl;
 *      x = -100;
 *      std::cout << x << std::endl;
 *      x = "something else";
 *      std::cout << x << std::endl;
 *      x = jsonv::array({ "arrays", "of", "the", 7, "different", "types"});
 *      std::cout << x << std::endl;
 *      x = jsonv::object({
 *                          { "objects", jsonv::array({
 *                                                     "Are fun, too.",
 *                                                     "Do what you want."
 *                                                   })
 *                          },
 *                          { "compose like", "standard library maps" },
 *                       });
 *      std::cout << x << std::endl;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  null
 *  5.9
 *  -100
 *  "something else"
 *  ["arrays","of","the",7,"different","types"]
 *  {"compose like":"standard library maps","objects":["Are fun, too.","Do what you want."]}
 *  \endcode
 *  
 *  If that isn't convenient enough for you, there is a user-defined literal \c _json in the \c jsonv namespace you can
 *  use
 *  
 *  \code
 *  // You can use this hideous syntax if you do not want to bring in the whole jsonv namespace:
 *  using jsonv::operator"" _json;
 *  
 *  jsonv::value x = R"({
 *                        "objects": [ "Are fun, too.",
 *                                     "Do what you want."
 *                                   ],
 *                        "compose like": "You are just writing JSON",
 *                        "which I guess": ["is", "also", "neat"]
 *                     })"_json;
 *  \endcode
 *  
 *  JSON is dynamic, which makes value access a bit more of a hassle, but JSON Voorhees aims to make it not too
 *  horrifying for you. A \c jsonv::value has a number of accessor methods named things like \c as_integer and
 *  \c as_string which let you access the value as if it was that type. But what if it isn't that type? In that case,
 *  the function will throw a \c jsonv::kind_error with a bit more information as to what rule you violated.
 *  
 *  \code
 *  #include <jsonv/value.hpp>
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::value x = jsonv::null;
 *      try
 *      {
 *          x.as_string();
 *      }
 *      catch (const jsonv::kind_error& err)
 *      {
 *          std::cout << err.what() << std::endl;
 *      }
 *      
 *      x = "now make it a string";
 *      std::cout << x.as_string().size() << std::endl;
 *      std::cout << x.as_string() << "\tis not the same as\t" << x << std::endl;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  Unexpected type: expected string but found null.
 *  20
 *  now make it a string    is not the same as  "now make it a string"
 *  \endcode
 *  
 *  You can also deal with container types in a similar manner that you would deal with the equivalent STL container
 *  type, with some minor caveats. Because the \c value_type of a JSON object and JSON array are different, they have
 *  different iterator types in JSON Voorhees. They are aptly-named \c object_iterator and \c array_iterator. The access
 *  methods for these iterators are \c begin_object / \c end_object and \c begin_array / \c end_array, respectively.
 *  The object interface behaves exactly like you would expect a \c std::map<std::string,jsonv::value> to, while the
 *  array interface behaves just like a \c std::deque<jsonv::value> would.
 *  
 *  \code
 *  #include <jsonv/value.hpp>
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::value x = jsonv::object({ { "one", 1 }});
 *      auto iter = x.find("one");
 *      if (iter != x.end_object())
 *          std::cout << iter->first << ": " << iter->second << std::endl;
 *      else
 *          std::cout << "Nothing..." << std::end;
 *      
 *      iter = x.find("two");
 *      if (iter != x.end_object())
 *          std::cout << iter->first << ": " << iter->second << std::endl;
 *      else
 *          std::cout << "Nothing..." << std::end;
 *      
 *      x["two"] = 2;
 *      iter = x.find("two");
 *      if (iter != x.end_object())
 *          std::cout << iter->first << ": " << iter->second << std::endl;
 *      else
 *          std::cout << "Nothing..." << std::end;
 *      
 *      x["two"] = jsonv::array({ "one", "+", x.at("one") });
 *      iter = x.find("two");
 *      if (iter != x.end_object())
 *          std::cout << iter->first << ": " << iter->second << std::endl;
 *      else
 *          std::cout << "Nothing..." << std::end;
 *      
 *      x.erase("one");
 *      iter = x.find("one");
 *      if (iter != x.end_object())
 *          std::cout << iter->first << ": " << iter->second << std::endl;
 *      else
 *          std::cout << "Nothing..." << std::end;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  one: 1
 *  Nothing...
 *  two: 2
 *  two: ["one","+",1]
 *  Nothing...
 *  \endcode
 *  
 *  The iterator types \e work. This means you are free to use all of the C++ things just like you would a regular
 *  container. To use a ranged-based for, simply call \c as_array or \c as_object. Everything from \c <algorithm> and
 *  \c <iterator> or any other library works great with JSON Voorhees. Bring those templates on!
 *  
 *  \code
 *  #include <jsonv/value.hpp>
 *  #include <algorithm>
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::value arr = jsonv::array({ "taco", "cat", 3, -2, jsonv::null, "beef", 4.8, 5 });
 *      std::cout << "Initial: ";
 *      for (const auto& val : arr.as_array())
 *          std::cout << val << '\t';
 *      std::cout << std::endl;
 *      
 *      std::sort(arr.begin_array(), arr.end_array());
 *      std::cout << "Sorted: ";
 *      for (const auto& val : arr.as_array())
 *          std::cout << val << '\t';
 *      std::cout << std::endl;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  Initial: "taco" "cat"   3   -2  null    "beef"  4.8   5
 *  Sorted:  null   -2  3   4.8 5   "beef"  "cat"   "taco"
 *  \endcode
 *  
 *  \section demo_parsing Encoding and decoding
 *  
 *  Usually, the reason people are using JSON is as a data exchange format, either for communicating with other services
 *  or storing things in a file or a database. To do this, you need to \e encode your \c json::value into an
 *  \c std::string and \e parse it back. JSON Voorhees makes this very easy for you.
 *  
 *  \code
 *  #include <jsonv/value.hpp>
 *  #include <jsonv/encode.hpp>
 *  #include <jsonv/parse.hpp>
 *  
 *  #include <iostream>
 *  #include <fstream>
 *  #include <limits>
 *  
 *  int main()
 *  {
 *      jsonv::value obj = jsonv::object();
 *      obj["taco"]  = "cat";
 *      obj["array"] = jsonv::array({ 1, 2, 3, 4, 5 });
 *      obj["infinity"] = std::numeric_limits<double>::infinity();
 *      
 *      {
 *          std::cout << "Saving \"file.json\"... " << obj << std::endl;
 *          std::ofstream file("file.json");
 *          file << obj;
 *      }
 *      
 *      jsonv::value loaded;
 *      {
 *          std::cout << "Loading \"file.json\"...";
 *          std::ifstream file("file.json");
 *          loaded = jsonv::parse(file);
 *      }
 *      std::cout << loaded << std::endl;
 *      
 *      return obj == loaded ? 0 : 1;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  Saving "file.json"... {"array":[1,2,3,4,5],"infinity":null,"taco":"cat"}
 *  Loading "file.json"...{"array":[1,2,3,4,5],"infinity":null,"taco":"cat"}
 *  \endcode
 *  
 *  If you are paying close attention, you might have noticed that the value for the \c "infinity" looks a little bit
 *  more \c null than \c infinity. This is because, much like mathematicians before Anaximander, JSON has no concept of
 *  infinity, so it is actually \e illegal to serialize a token like \c infinity anywhere. By default, when an encoder
 *  encounters an unrepresentable value in the JSON it is trying to encode, it outputs \c null instead. If you wish to
 *  change this behavior, implement your own \c jsonv::encoder (or derive from \c jsonv::ostream_encoder). If you ran
 *  the example program, you might have noticed that the return code was 1, meaning the value you put into the file and
 *  what you got from it were not equal. This is because all the type and value information is still kept around in the
 *  in-memory \c obj. It is only upon encoding that information is lost.
 *  
 *  Getting tired of all this compact rendering of your JSON strings? Want a little more whitespace in your life? Then
 *  \c jsonv::ostream_pretty_encoder is the class for you! Unlike our standard \e compact encoder, this guy will put
 *  newlines and indentation in your JSON so you can present it in a way more readable format.
 *  
 *  \code
 *  #include <jsonv/encode.hpp>
 *  #include <jsonv/parse.hpp>
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      // Make a pretty encoder and point to std::cout
 *      jsonv::ostream_pretty_encoder prettifier(std::cout);
 *      prettifier.encode(jsonv::parse(std::cin));
 *  }
 *  \endcode
 *  
 *  Compile that code and you now have your own little JSON prettification program!
 *  
 *  \section serialization Serialization
 *  
 *  Most of the time, you do not want to deal with \c jsonv::value instances directly. Instead, most people prefer to
 *  convert \c jsonv::value instances into their own strong C++ \c class or \c struct. JSON Voorhees provides utilities
 *  to make this easy for you to use. At the end of the day, you should be able to create an arbitrary C++ type with
 *  <tt>jsonv::extract&lt;my_type&gt;(value)</tt> and create a \c jsonv::value from your arbitrary C++ type with
 *  <tt>jsonv::to_json(my_instance)</tt>.
 *  
 *  \subsection serialization_encoding Extracting with extract
 *  
 *  Let's start with converting a \c jsonv::value into a custom C++ type with <tt>jsonv::extract&lt;T&gt;</tt>.
 *  
 *  \code
 *  #include <jsonv/parse.hpp>
 *  #include <jsonv/serialization.hpp>
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::value val = jsonv::parse(R"({ "a": 1, "b": 2, "c": "Hello!" })");
 *      std::cout << "a=" << jsonv::extract<int>(val.at("a")) << std::endl;
 *      std::cout << "b=" << jsonv::extract<int>(val.at("b")) << std::endl;
 *      std::cout << "c=" << jsonv::extract<std::string>(val.at("c")) << std::endl;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  a=1
 *  b=2
 *  c=Hello!
 *  \endcode
 *  
 *  Overall, this is not very complicated. We did not do anything that could not have been done through a little use of
 *  \c as_integer and \c as_string. So what is this \c extract giving us?
 *  
 *  The real power comes in when we start talking about \c jsonv::formats. These objects provide a set of rules to
 *  encode and decode arbitrary types. So let's make a C++ \c class for our JSON object and write a special constructor
 *  for it.
 *  
 *  \code
 *  #include <jsonv/parse.hpp>
 *  #include <jsonv/serialization.hpp>
 *  #include <jsonv/serialization_util.hpp>
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  class my_type
 *  {
 *  public:
 *      my_type(const jsonv::value& from, const jsonv::extraction_context& context) :
 *              a(context.extract_sub<int>(from, "a")),
 *              b(context.extract_sub<int>(from, "b")),
 *              c(context.extract_sub<std::string>(from, "c"))
 *      { }
 *      
 *      static const jsonv::extractor* get_extractor()
 *      {
 *          static jsonv::extractor_construction<my_type> instance;
 *          return &instance;
 *      }
 *      
 *      friend std::ostream& operator<<(std::ostream& os, const my_type& self)
 *      {
 *          return os << "{ a=" << self.a << ", b=" << self.b << ", c=" << self.c << " }";
 *      }
 *      
 *  private:
 *      int         a;
 *      int         b;
 *      std::string c;
 *  };
 *  
 *  int main()
 *  {
 *      jsonv::formats local_formats;
 *      local_formats.register_extractor(my_type::get_extractor());
 *      jsonv::formats format = jsonv::formats::compose({ jsonv::formats::defaults(), local_formats });
 *      
 *      jsonv::value val = jsonv::parse(R"({ "a": 1, "b": 2, "c": "Hello!" })");
 *      my_type x = jsonv::extract<my_type>(val, format);
 *      std::ostream << x << std::endl;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  { a=1, b=2, c=Hello! }
 *  \endcode
 *  
 *  There is a lot going on in that example, so let's take it one step at a time. First, we are creating a \c my_type
 *  object to store our values, which is nice. Then, we gave it a funny-looking constructor:
 *  
 *  \code
 *      my_type(const jsonv::value& from, const jsonv::extraction_context& context) :
 *              a(context.extract_sub<int>(from, "a")),
 *              b(context.extract_sub<int>(from, "b")),
 *              c(context.extract_sub<std::string>(from, "c"))
 *      { }
 *  \endcode
 *  
 *  This is an <i>extracting constructor</i>. All that means is that it has those two arguments: a \c jsonv::value and
 *  a \c jsonv::extraction_context. The \c jsonv::extraction_context is an optional, but extremely helpful class. Inside
 *  the constructor, we use the \c jsonv::extraction_context to access the values of the incoming JSON object in order
 *  to build our object.
 *  
 *  \code
 *      static const jsonv::extractor* get_extractor()
 *      {
 *          static jsonv::extractor_construction<my_type> instance;
 *          return &instance;
 *      }
 *  \endcode
 *  
 *  A \c jsonv::extractor is a type that knows how to take a \c jsonv::value and create some C++ type out of it. In this
 *  case, we are creating a \c jsonv::extractor_construction, which is a subtype that knows how to call the constructor
 *  of a type. There are all sorts of \c jsonv::extractor implementations in \c jsonv/serialization.hpp, so you should
 *  be able to find one that fits your needs.
 *  
 *  \code
 *      jsonv::formats local_formats;
 *      local_formats.register_extractor(my_type::get_extractor());
 *      jsonv::formats format = jsonv::formats::compose({ jsonv::formats::defaults(), local_formats });
 *  \endcode
 *  
 *  Now things are starting to get interesting. The \c jsonv::formats object is a collection of
 *  <tt>jsonv::extractor</tt>s, so we create one of our own and add the \c jsonv::extractor* from the static function of
 *  \c my_type. Now, \c local_formats \e only knows how to extract instances of \c my_type -- it does \e not know even
 *  the most basic things like how to extract an \c int. We use \c jsonv::formats::compose to create a new instance of
 *  \c jsonv::formats that combines the qualities of \c local_formats (which knows how to deal with \c my_type) and the
 *  \c jsonv::formats::defaults (which knows how to deal with things like \c int and \c std::string). The \c formats
 *  instance now has the power to do everything we need!
 *  
 *  \code
 *      my_type x = jsonv::extract<my_type>(val, format);
 *  \endcode
 *  
 *  This is not terribly different from the example before, but now we are explicitly passing a \c jsonv::formats object
 *  to the function. If we had not provided \c format as an argument here, the function would have thrown a
 *  \c jsonv::extraction_error complaining about how it did not know how to extract a \c my_type.
 *  
 *  \subsection serialization_to_json Serialization with to_json
 *  
 *  JSON Voorhees also allows you to convert from your C++ structures into JSON values, using \c jsonv::to_json. It
 *  should feel like a mirror \c jsonv::extract, with similar argument types and many shared concepts. Just like
 *  extraction, \c jsonv::to_json uses the \c jsonv::formats class, but it uses a \c jsonv::serializer to convert from
 *  C++ into JSON.
 *  
 *  \code
 *  #include <jsonv/serialization.hpp>
 *  #include <jsonv/serialization_util.hpp>
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  class my_type
 *  {
 *  public:
 *      my_type(int a, int b, std::string c) :
 *              a(a),
 *              b(b),
 *              c(std::move(c))
 *      { }
 *      
 *      static const jsonv::serializer* get_serializer()
 *      {
 *          static auto instance = jsonv::make_serializer<my_type>
 *                                 (
 *                                  [] (const jsonv::serialization_context& context, const my_type& self)
 *                                  {
 *                                      return jsonv::object({ { "a", context.to_json(self.a) },
 *                                                             { "b", context.to_json(self.b) },
 *                                                             { "c", context.to_json(self.c) }
 *                                                           }
 *                                                          );
 *                                  }
 *                                 );
 *          return &instance;
 *      }
 *      
 *  private:
 *      int         a;
 *      int         b;
 *      std::string c;
 *  };
 *  
 *  int main()
 *  {
 *      jsonv::formats local_formats;
 *      local_formats.register_serializer(my_type::get_serializer());
 *      jsonv::formats format = jsonv::formats::compose({ jsonv::formats::defaults(), local_formats });
 *      
 *      my_type x(5, 6, "Hello");
 *      std::ostream << jsonv::to_json(x, format) << std::endl;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  {"a":5,"b":6,"c":"Hello"}
 *  \endcode
 *  
 *  \subsection serialization_composition Composing Type Adapters
 *  
 *  Does all this seem a little bit \e manual to you? Creating an \c extractor and \c serializer for every single type
 *  can get a little bit tedious. Unfortunately, until C++ has a standard way to do reflection, we must specify the
 *  conversions manually. However, there \e is an easier way! That way is the
 *  \ref serialization_builder_dsl "Serialization Builder DSL".
 *  
 *  Let's start with a couple of simple structures:
 *  
 *  \code
 *  struct foo
 *  {
 *      int         a;
 *      int         b;
 *      std::string c;
 *  };
 *  
 *  struct bar
 *  {
 *      foo         x;
 *      foo         y;
 *      std::string z;
 *      std::string w;
 *  };
 *  \endcode
 *  
 *  Let's make a \c formats for them using the DSL:
 *  
 *  \code
 *  jsonv::formats formats =
 *      jsonv::formats_builder()
 *          .type<foo>()
 *              .member("a", &foo::a)
 *              .member("b", &foo::b)
 *                  .default_value(10)
 *              .member("c", &foo::c)
 *          .type<bar>()
 *              .member("x", &bar::x)
 *              .member("y", &bar::y)
 *              .member("z", &bar::z)
 *                  .since(jsonv::version(2, 0))
 *              .member("w", &bar::w)
 *                  .until(jsonv::version(5, 0))
 *      ;
 *  \endcode
 *  
 *  What is going on there? The giant chain of function calls is building up a collection of type adapters into a
 *  \c formats for you. The indentation shows the intent -- the <tt>.member("a", &foo::a)</tt> is attached to the type
 *  \c adapter for \c foo (if you tried to specify \c &bar::y in that same place, it would fail to compile). Each
 *  function call returns a reference back to the builder so you can chain as many of these together as you want to. The
 *  \c jsonv::formats_builder is a proper object, so if you wish to spread out building your type adapters into multiple
 *  functions, you can do that by passing around an instance.
 *  
 *  The two most-used functions are \c type and \c member. \c type defines a \c jsonv::adapter for the C++ class
 *  provided at the template parameter. All of the calls before the second \c type call modify the adapter for \c foo.
 *  There, we attach members with the \c member function. This tells the \c formats how to encode and extract each of
 *  the specified members to and from a JSON object using the provided string as the key. The extra function calls like
 *  \c default_value, \c since and \c until are just a could of the many functions available to modify how the members
 *  of the type get transformed.
 *  
 *  The \c formats we built would be perfectly capable of serializing to and extracting from this JSON document:
 *  
 *  \code
 *  {
 *      "x": { "a": 50, "b": 20, "c": "Blah" },
 *      "y": { "a": 10,          "c": "No B?" },
 *      "z": "Only serialized in 2.0+",
 *      "w": "Only serialized before 5.0"
 *  }
 *  \endcode
 *  
 *  For a more in-depth reference, see the \ref serialization_builder_dsl "Serialization Builder DSL page".
 *  
 *  \section demo_algorithm Algorithms
 *  
 *  JSON Voorhees takes a "batteries included" approach. A few building blocks for powerful operations can be found in
 *  the \c algorithm.hpp header file.
 *  
 *  One of the simplest operations you can perform is the \c map operation. This operation takes in some \c jsonv::value
 *  and returns another. Let's try it.
 *  
 *  \code
 *  #include <jsonv/algorithm.hpp>
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::value x = 5;
 *      std::cout << jsonv::map([] (const jsonv::value& y) { return y.as_integer() * 2; }, x) << std::endl;
 *  }
 *  \endcode
 *  
 *  If everything went right, you should see a number:
 *  
 *  \code
 *  10
 *  \endcode
 *  
 *  Okay, so that was not very interesting. To be fair, that is not the most interesting example of using \c map, but it
 *  is enough to get the general idea of what is going on. This operation is so common that it is a member function of
 *  \c value as \c jsonv::value::map. Let's make things a bit more interesting and \c map an \c array...
 *  
 *  \code
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      std::cout << jsonv::array({ 1, 2, 3, 4, 5 })
 *                          .map([] (const jsonv::value& y) { return y.as_integer() * 2; })
 *                << std::endl;
 *  }
 *  \endcode
 *  
 *  Now we're starting to get somewhere!
 *  
 *  \code
 *  [2,4,6,8,10]
 *  \endcode
 *  
 *  The \c map function maps over whatever the contents of the \c jsonv::value happens to be and returns something for
 *  you based on the \c kind. This simple concept is so ubiquitous that <a href="http://www.disi.unige.it/person/MoggiE/">
 *  Eugenio Moggi</a> named it a <a href="http://stackoverflow.com/questions/44965/what-is-a-monad">monad</a>. If you're
 *  feeling adventurous, try using \c map with an \c object or chaining multiple \c map operations together.
 *  
 *  Another common building block is the function \c jsonv::traverse. This function walks a JSON structure and calls a
 *  some user-provided function.
 *  
 *  \code
 *  #include <jsonv/algorithm.hpp>
 *  #include <jsonv/parse.hpp>
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::traverse(jsonv::parse(std::cin),
 *                      [] (const jsonv::path& path, const jsonv::value& value)
 *                      {
 *                          std::cout << path << "=" << value << std::endl;
 *                      },
 *                      true
 *                     );
 *  }
 *  \endcode
 *  
 *  Now we have a tiny little program! Here's what happens when I pipe <tt>{ "bar": [1, 2, 3], "foo": "hello" }</tt>
 *  into the program:
 *  
 *  \code
 *  .bar[0]=1
 *  .bar[1]=2
 *  .bar[2]=3
 *  .foo="hello"
 *  \endcode
 *  
 *  Imagine the possibilities!
 *  
 *  All of the \e really powerful functions can be found in \c util.hpp. My personal favorite is \c jsonv::merge. The
 *  idea is simple: it merges two (or more) JSON values into one.
 *  
 *  \code
 *  #include <jsonv/util.hpp>
 *  #include <jsonv/value.hpp>
 *  
 *  #include <iostream>
 *  
 *  int main()
 *  {
 *      jsonv::value a = jsonv::object({ { "a", "taco" }, { "b", "cat" } });
 *      jsonv::value b = jsonv::object({ { "c", "burrito" }, { "d", "dog" } });
 *      jsonv::value merged = jsonv::merge(std::move(a), std::move(b));
 *      std::cout << merged << std::endl;
 *  }
 *  \endcode
 *  
 *  Output:
 *  
 *  \code
 *  {"a":"taco","b":"cat","c":"burrito","d":"dog"}
 *  \endcode
 *  
 *  You might have noticed the use of \c std::move into the \c merge function. Like most functions in JSON Voorhees,
 *  \c merge takes advantage of move semantics. In this case, the implementation will move the contents of the values
 *  instead of copying them around. While it may not matter in this simple case, if you have large JSON structures, the
 *  support for movement will save you a ton of memory.
 *  
 *  \see https://github.com/tgockel/json-voorhees
 *  \see http://json.org/
**/

}

#include "algorithm.hpp"
#include "coerce.hpp"
#include "config.hpp"
#include "demangle.hpp"
#include "encode.hpp"
#include "forward.hpp"
#include "functional.hpp"
#include "parse.hpp"
#include "path.hpp"
#include "serialization.hpp"
#include "serialization_builder.hpp"
#include "serialization_util.hpp"
#include "string_view.hpp"
#include "tokenizer.hpp"
#include "util.hpp"
#include "value.hpp"

#endif/*__JSONV_ALL_HPP_INCLUDED__*/
