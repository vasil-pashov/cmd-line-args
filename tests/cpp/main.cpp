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
		parser.addParam("flag", "A flag", CMD::CommandLineArgs::Type::Flag, false);
		SUBCASE("Flag param is set") {
			char* argv[argc] = {"program", "-flag"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			REQUIRE(parser.isSet("flag"));
		}

		SUBCASE("Flag param cannot have a value") {
			char* argv[argc] = {"program", "-flag=2"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::FlagHasValue);
				CHECK_EQ(strcmp(param, "flag"), 0);
				CHECK_EQ(strcmp(input, "-flag=2"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::FlagHasValue);
			REQUIRE_FALSE(parser.isSet("flag"));
		}
	}

	TEST_CASE("Int param") {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam("intParam", "An integer", CMD::CommandLineArgs::Type::Int, false);
		SUBCASE("Int param is set") {
			char* argv[argc] = {"program", "-intParam=4"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			REQUIRE(parser.isSet("intParam"));
			const std::optional val = parser.getValue<int>("intParam");
			REQUIRE_EQ(val.value(), 4);
		}

		SUBCASE("Int param must have a value") {
			char* argv[argc] = {"program", "-intParam"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::MissingValue);
				CHECK_EQ(strcmp(param, "intParam"), 0);
				CHECK_EQ(strcmp(input, "-intParam"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::MissingValue);
			REQUIRE_FALSE(parser.isSet("intParam"));
		}

		SUBCASE("Int param must have proper type") {
			char* argv[argc] = {"program", "-intParam=43 asd"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::WrongValueType);
				CHECK_EQ(strcmp(param, "intParam"), 0);
				CHECK_EQ(strcmp(input, "-intParam=43 asd"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::WrongValueType);
			REQUIRE_FALSE(parser.isSet("intParam"));
		}
	}

	TEST_CASE("String param") {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam("stringParam", "A string", CMD::CommandLineArgs::Type::String, false);
		SUBCASE("String param is set") {
			char* argv[argc] = {"program", "-stringParam= random string with \n escaped \t chars "};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			REQUIRE(parser.isSet("stringParam"));
			const std::optional<std::string>& val = parser.getValue<std::string>("stringParam");
			REQUIRE_EQ(strcmp(val.value().c_str() , " random string with \n escaped \t chars "), 0);
		}

		SUBCASE("String param can be empty") {
			char* argv[argc] = {"program", "-stringParam="};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			REQUIRE(parser.isSet("stringParam"));
			const std::optional<std::string>& val = parser.getValue<std::string>("stringParam");
			REQUIRE_EQ(strcmp(val.value().c_str(), ""), 0);
		}
	}
}