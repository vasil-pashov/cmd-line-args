#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "cmd_line_args.h"

TEST_SUITE("Parse Input") {
	TEST_CASE("Empty") {
		const int argc = 1;
		char* argv[] = {"program"};
		CMD::CommandLineArgs parser;
		REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
	}

	TEST_CASE("Flag param") {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam(CMD::CommandLineArgs::Type::Flag, "flag", "A flag", false);
		SUBCASE("Flag param is set") {
			char* argv[argc] = {"program", "--flag"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			const bool* val = parser.getValue<CMD::CommandLineArgs::Type::Flag>("flag");
			REQUIRE(*val);
		}

		SUBCASE("Flag param cannot have a value") {
			char* argv[argc] = {"program", "--flag=2"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::FlagHasValue);
				CHECK_EQ(strcmp(param, "flag"), 0);
				CHECK_EQ(strcmp(input, "--flag=2"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::FlagHasValue);
			REQUIRE_FALSE(*parser.getValue<CMD::CommandLineArgs::Type::Flag>("flag"));
		}

		SUBCASE("Flag which is defined but not added in the args is false") {
			char* argv[1] = {"program"};
			REQUIRE_EQ(parser.parse(1, argv), CMD::CommandLineArgs::ErrorCode::Success);
			const bool* flagVal = parser.getValue<CMD::CommandLineArgs::Type::Flag>("flag");
			REQUIRE_FALSE(*flagVal);
		}

		SUBCASE("Flag param cannot be accessed via other types") {
			char* argv[argc] = {"program", "--flag"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);

			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::Int>("flag"), nullptr);
			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::String>("flag"), nullptr);
		}
	}

	TEST_CASE("Int param") {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam(CMD::CommandLineArgs::Type::Int, "intParam", "An integer", false);
		SUBCASE("Int param is set") {
			char* argv[argc] = {"program", "--intParam=4"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			const int* val = parser.getValue<CMD::CommandLineArgs::Type::Int>("intParam");
			REQUIRE_EQ(*val, 4);
		}

		SUBCASE("Int param must have a value") {
			char* argv[argc] = {"program", "--intParam"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::MissingValue);
				CHECK_EQ(strcmp(param, "intParam"), 0);
				CHECK_EQ(strcmp(input, "--intParam"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::MissingValue);
			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::Int>("intParam"), nullptr);
		}

		SUBCASE("Int param must have proper type") {
			char* argv[argc] = {"program", "--intParam=43asd"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::WrongValueType);
				CHECK_EQ(strcmp(param, "intParam"), 0);
				CHECK_EQ(strcmp(input, "--intParam=43asd"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::WrongValueType);
			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::Int>("intParam"), nullptr);
		}

		SUBCASE("Int param cannot be accessed via other types") {
			char* argv[argc] = {"program", "--intParam=123"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);

			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::Flag>("intParam"), nullptr);
			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::String>("intParam"), nullptr);
		}
	}

	TEST_CASE("String param") {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam(CMD::CommandLineArgs::Type::String, "stringParam", "A string", false);
		SUBCASE("String param is set") {
			char* argv[argc] = {"program", "--stringParam= random string with \n escaped \t chars "};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			const std::string* str = parser.getValue<CMD::CommandLineArgs::Type::String>("stringParam");
			REQUIRE_EQ(strcmp(str->c_str() , " random string with \n escaped \t chars "), 0);
		}

		SUBCASE("String param cannot be empty") {
			char* argv[argc] = {"program", "--stringParam="};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::MissingValue);
			const std::string* val = parser.getValue<CMD::CommandLineArgs::Type::String>("stringParam");
			REQUIRE_EQ(val, nullptr);
		}

		SUBCASE("String param cannot be accessed via other types") {
			char* argv[argc] = {"program", "--stringParam=123"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);

			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::Flag>("stringParam"), nullptr);
			REQUIRE_EQ(parser.getValue<CMD::CommandLineArgs::Type::Int>("stringParam"), nullptr);
		}
	}
}