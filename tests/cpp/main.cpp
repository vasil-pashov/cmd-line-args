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
		parser.addParam<bool>("flag", "A flag", false);
		SUBCASE("Flag param is set") {
			char* argv[argc] = {"program", "-flag"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			REQUIRE(parser.getValue<bool>("flag").value());
		}

		SUBCASE("Flag param cannot have a value") {
			char* argv[argc] = {"program", "-flag=2"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::FlagHasValue);
				CHECK_EQ(strcmp(param, "flag"), 0);
				CHECK_EQ(strcmp(input, "-flag=2"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::FlagHasValue);
			REQUIRE_FALSE(parser.getValue<bool>("flag").value());
		}

		SUBCASE("Flag which is defined but not added in the args is false") {
			char* argv[1] = {"program"};
			REQUIRE_EQ(parser.parse(1, argv), CMD::CommandLineArgs::ErrorCode::Success);
			const std::optional flagVal = parser.getValue<bool>("flag");
			REQUIRE_FALSE(*flagVal);
		}
	}

	TEST_CASE_TEMPLATE("Cannot access flag via other types", T, int, std::string) {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam<bool>("flag", "A flag", false);
		char* argv[argc] = {"program", "-flag"};
		REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);

		const std::optional opt = parser.getValue<T>("flag");
		REQUIRE_FALSE(opt.has_value());
	}

	TEST_CASE("Int param") {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam<int>("intParam", "An integer", false);
		SUBCASE("Int param is set") {
			char* argv[argc] = {"program", "-intParam=4"};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
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
		}

		SUBCASE("Int param must have proper type") {
			char* argv[argc] = {"program", "-intParam=43asd"};
			auto callback = [](CMD::CommandLineArgs::ErrorCode code, const char* param, const char* input, const char* description) -> void {
				CHECK_EQ(code, CMD::CommandLineArgs::ErrorCode::WrongValueType);
				CHECK_EQ(strcmp(param, "intParam"), 0);
				CHECK_EQ(strcmp(input, "-intParam=43asd"), 0);
			};
			REQUIRE_EQ(parser.parse(argc, argv, callback), CMD::CommandLineArgs::ErrorCode::WrongValueType);
		}
	}

	TEST_CASE_TEMPLATE("Cannot access int via other types", T, bool , std::string) {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam<int>("val", "A val", false);
		char* argv[argc] = {"program", "-val=132"};
		REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);

		const std::optional opt = parser.getValue<T>("val");
		REQUIRE_FALSE(opt.has_value());
	}

	TEST_CASE("String param") {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam<std::string>("stringParam", "A string", false);
		SUBCASE("String param is set") {
			char* argv[argc] = {"program", "-stringParam= random string with \n escaped \t chars "};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			const std::optional<std::string>& val = parser.getValue<std::string>("stringParam");
			REQUIRE_EQ(strcmp(val.value().c_str() , " random string with \n escaped \t chars "), 0);
		}

		SUBCASE("String param can be empty") {
			char* argv[argc] = {"program", "-stringParam="};
			REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
			const std::optional<std::string>& val = parser.getValue<std::string>("stringParam");
			REQUIRE_EQ(strcmp(val.value().c_str(), ""), 0);
		}
	}

	TEST_CASE_TEMPLATE("Cannot access string via other types", T, bool, int) {
		const int argc = 2;
		CMD::CommandLineArgs parser;
		parser.addParam<std::string>("str", "A str", false);
		char* argv[argc] = {"program", "-str=132"};
		REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);

		const std::optional opt = parser.getValue<T>("str");
		REQUIRE_FALSE(opt.has_value());
	}

	TEST_CASE_TEMPLATE("Undefined values have empty optionals", T, bool, int, std::string) {
		CMD::CommandLineArgs parser;
		const int argc = 1;
		char* argv[argc] = {"program"};
		REQUIRE_EQ(parser.parse(argc, argv), CMD::CommandLineArgs::ErrorCode::Success);
		const std::optional opt = parser.getValue<T>("undefined");
		REQUIRE_FALSE(opt.has_value());
	}
}