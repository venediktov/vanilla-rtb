1._ Series
==========

[1.0](https://github.com/tgockel/json-voorhees/issues?q=milestone%3Av1.0)
-------------------------------------------------------------------------

Stabilizing the API and finalizing things for a release.

 - [1.0.1](https://github.com/tgockel/json-voorhees/releases/tag/v1.0.1): *FUTURE*
   - More reasonable dealing with invalid UTF-8 input
   - Improved support for JSON comments
   - Improved support for Mac OSX

 - [1.0.0](https://github.com/tgockel/json-voorhees/releases/tag/v1.0.0): 2015 March 13
   - Moves to CMake as the build system
   - Greatly improves the speed of the parser
   - Removes `nullptr` as a type for null in preference of `jsonv::null`
   - Better support for MSVC


0._ Series
==========

[0.5](https://github.com/tgockel/json-voorhees/issues?q=milestone%3Av0.5)
-------------------------------------------------------------------------

The focus of this release is extensible serialization between JSON values and C++ types.

 - [0.5.1](https://github.com/tgockel/json-voorhees/releases/tag/v0.5.1): 2015 February 17
   - Creates `null` to be used in place of `nullptr`
   - Adds the ability to set the default size of `tokenizer`'s buffer
   - Fixes issue in parsing where string divisions on buffer boundaries would yield incorrect results (and potentially
     crash)

 - [0.5.0](https://github.com/tgockel/json-voorhees/releases/tag/v0.5.0): 2015 February 13
   - Creates `formats`, `extractor`, `serializer` and `adapter` classes
   - Creates the `extract` and `to_json` free functions for conversion
   - Creates the Serialization Builder DSL for easily making type adapters
   - Adds support for compiling with GCC and Clang on Windows with [Cygwin](https://www.cygwin.com/)
   - Adds experimental support for Microsoft Visual Studio 14 (CTP 5)
   - Adds the `value::is_X` convenience functions for checking `kind` values.

[0.4](https://github.com/tgockel/json-voorhees/issues?q=milestone%3Av0.4)
-------------------------------------------------------------------------

The focus of this release was the creation of tools to traverse and manipulate the JSON AST.

 - [0.4.1](https://github.com/tgockel/json-voorhees/releases/tag/v0.4.1): 2015 January 19
   - Adds reverse iteration to `array_view` and `object_view`
   - Adds the `map` algorithm
   - Adds the `diff` algorithm
   - Adds `value::count_path`
   - Fixes issue in parsing where large inputs would buffer incorrectly

 - [0.4.0](https://github.com/tgockel/json-voorhees/releases/tag/v0.4.0): 2015 January 13
   - Creates a generic visitor system for `jsonv::value`
   - Creates the `path` system, which is a very simplified version of JSONPath
   - Creates the `merge` and `traverse` families of algorithms
   - Creates the *coerce* library for non-strict conversion
   - Extends `array_view` and `object_view` to have *owning* versions, so calling `value::as_array` and
     `value::as_object` on rvalues works as you would expect (safely)
   - Make `kind::decimal` and `kind::integer` equivalent in almost all cases
   - Various bugfixes

[0.3](https://github.com/tgockel/json-voorhees/issues?q=milestone%3Av0.3)
-------------------------------------------------------------------------

The main focus of this release is access and modification of the low-level parsing and encoding system.
 
 - [0.3.1](https://github.com/tgockel/json-voorhees/releases/tag/v0.3.1): 2014 September 27
   - Greatly expands the flexibility of `parse_options`
   - Adds all the tests from [JSON_Checker](http://json.org/JSON_checker/)
   - Adds a `parse` that takes a `string_ref`, which unifies the `const char*` and `std::string` overloads
   - Changes the installer to put header files inside of a `jsonv` folder
   - Adds Arch Linux `PKGBUILD` to the `installer` folder
   - Fixes issue with incorrectly escaping `\\\"`
   - Fixes issue with `value::compare(const value&) const` returning non-zero when two decimal kinds were within the
     epsilon value (and `operator==` would have returned `true`)

 - [0.3.0](https://github.com/tgockel/json-voorhees/releases/tag/v0.3.0): 2014 September 21
    - Creates `tokenizer` for easier access to JSON parsing
    - Creates `encode` for customization of JSON output
    - Creates `value::array_view` and `value::object_view` for use in range-based for loops
    - Creates the `make install` recipe (with customization of versioned SOs)
    - Re-write of the parsing system to be stream-based so not everything has to reside in memory at the same time
    - Expose `string_ref` as part of the `encode` and `tokenizer` systems
    - Improved exception-handling in `value`
    - Improved documentation, including automatic Doxygen generation with Travis CI

0.2
---

The main focus of this release was the unification of the JSON type into `jsonv::value`.
Another major addition is full support for parsing and emitting strings into a proper encoding.

 - [0.2.2](https://github.com/tgockel/json-voorhees/releases/tag/v0.2.2): 2014 May 18
    - Adds `std::hash<jsonv::value>`

 - [0.2.1](https://github.com/tgockel/json-voorhees/releases/tag/v0.2.1): 2014 May 9
    - Adds support for building with Clang (version 3.3 and beyond)

 - [0.2.0](https://github.com/tgockel/json-voorhees/releases/tag/v0.2.0): 2014 May 7
    - Elimination of `array` and `object` types in preference of just `value`
    - Added the ability to specify parsing options
    - Use `JSONV_` as the macro prefix everywhere
    - Full support for decoding JSON numeric encodings (`\uNNNN`) as UTF-8 or CESU-8
    - Various fixes for comparison and assignment

0.1
---

The original prototype, which allows for parsing input to the JSON AST, manipulation of said AST and eventually encoding
 it as a string.

 - [0.1.1](https://github.com/tgockel/json-voorhees/releases/tag/v0.1.1): 2014 April 30
    - Minor parsing performance improvements by batching the string read for `parse_number`
    - Move to GNU Make as the build system

 - [0.1.0](https://github.com/tgockel/json-voorhees/releases/tag/v0.1.0): 2014 April 24
