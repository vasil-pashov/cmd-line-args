# Command Line Arguments Parser

Parses command line arguments passed to programs.
It supports parameters with types and does type checking for each parameter. For supported types
see `CMD::CommandLineArgs::Type`.
 
Currently supports only long parameter names starting with two dashes -- followed by the name of the parameter.
Non flag parameters must have a value. It is passed by an equals sign with now whitespace between the equals
the name of the parameter and the value of the parameter.
 
Positional arguments are not supported at the moment, however some arguments can be marked as required. Check
`CMD::CommandLineArgs::addParam` to see how to add required parameter.
 
# Usage
  - Create `CMD::CommandLineArgs` class
  - Add parameters via `CMD::CommandLineArgs::addParam`
  - Parse the parameters passed to main via `CMD::CommandLineArgs::parse`
  - Request parameters via `CMD::CommandLineArgs::getValue`
 
```c++
int main(int argc, char** argv) {
  using namespace CMD;
  CommandLineArgs parser;
  parser.addParam(CommandLineArgs::Type::Int, "intParam", false);
  parser.addParam(CommandLineArgs::Type::Flag, "flag", false);
  parser.addParam(CommandLineArgs::Type::String, "str", true);
  if(parser.parse(argc, argv) != CommandLineArgs::ErrorCode::Success) {
    return 1;
  }
  const int* intParam = parser.getValue<CommandLineArgs::Type::Int>("intParam"); // If --intParam=value was not passed, getValue returns nullptr
  if(intParam) {
    std::cout<<*intParam;
  }
  const bool* flag = parser.getValue<CommandLineArgs::Type::Flag>("flag") // Flag always return non null. If passed returns ptr to true, otherwise to false
  std::cout<<*flag;
  const std::string* str = parser.getValue<CommandLineArgs::Type::String>("str") // It was added with required=true it will always have non null ptr
  std::cout<<*str;
}
```
# CMake options
* `CMD_LINE_ARGS_WITH_TESTS` Builds tests for the project. Default `OFF`.
* `CMD_LINE_ARGS_WITH_INSTALL` Generate install target. Default `OFF`.
* `CMD_LINE_ARGS_WITH_DOCS` Generate doxygen documentation, generated in build/html folder. Default `OFF`.

# Dependecies
## Doctest

In case `CMD_LINE_ARGS_WITH_TESTS` is `ON` tests will be generated. The testing framework is [doxygen](https://github.com/doxygen/doxygen). Everything needed will be automatically downloaded and setup using CMake FetchContent utility. Building the tests works out-of-the-box.

## Documentation
In case `CMD_LINE_ARGS_WITH_DOCS` is `ON` a target which builds doxygen documentation will be generated. Sadly building doxygen on windows is a pain in the ass. Thus if this option is `ON` doxygen must be installed on the system. CMake uses `find_package(Doxygen REQUIRED)` to search for Doxygen executables.
