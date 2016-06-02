Data in this directory is from the [JSON_Checker](http://json.org/JSON_checker/) test data.

There are 3 kinds of files in this directory, all with different patterns:

 - `pass${#}.json`: These are valid JSON files and should be parsed by JSON Voorhees.
 - `fail${#}.json`: These are invalid JSON files that should be rejected with a `jsonv::parse_error`.
 - `pass-but-fail-strict-${#}.json`: These are invalid JSON files that the default JSON Voorhees parser will happily
   parse, but should be rejected in strict parsing mode (which does not exist).
   There is [an issue](https://github.com/tgockel/json-voorhees/issues/16) to think about or potentially do something
   about these files.
