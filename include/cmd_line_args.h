#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <variant>

namespace CMD {

/// Structure to parse and hold command line arguments
struct CommandLineArgs {
	~CommandLineArgs();

	enum class ErrorCode {
		Success = 0,
		MissingParameter,
		WrongValueType,
		UnknownParameter,
		FlagHasValue,
		MissingValue,
		ParameterExists,
		Unknown
	};

	/// @brief Type of the callback function which can be passed to parse
	/// @code The code of the error
	/// @param The name of the parameter (without the dashes)
	/// @input The whole erroneous input passed for the parameter
	/// @descrioption A description of the error
	using ErrorCallbackT = void(*)(ErrorCode code, const char* param, const char* input, const char* description);

	/// @brief Get the value of a parameter
	/// @tparam T The type of the parameter.
	/// @param param The name of the parameter
	/// @return If the passed type T is the same as the declared type of the parameter
	/// and the parameter was passed then the optional will contain the value of the parameter.
	/// If the type is bool and matches with the declared type:
	/// a) If the parameter was passed, then the optional will contain true
	/// b) If the parameter was not passed, then the optional will contain false.
	/// If the types don't match or the parameter was not passed (except for the case of bool parameters)
	/// then the optional will be empty.
	template<typename T>
	std::optional<T> getValue(const std::string& param) {
		const auto valueIt = paramValues.find(param);

		if (valueIt == paramValues.end()) {
			if constexpr (std::is_same_v<T, bool>) {
				const auto infoIt = paramInfo.find(param);
				if (infoIt != paramInfo.end()) {
					return false;
				}
			}
			return {};
		}

		if (!std::holds_alternative<T>(valueIt->second)) {
			return {};
		}

		return std::get<T>(valueIt->second);
	}
	/// Add a info for possible parameter that can come from the command line. All possible parameters must
	/// be added before calling parse. Calling this does not add a parameter only information.
	/// @tparam T The type of the parameter. It will be used when parsing the argv to ensure that the passed
	/// parameters are of type T. Allowed types: bool for flags, int, std::string.
	/// @param[in] name name of the param, must not include - or =.
	/// @param[in] description description for the parameter can include everything. This is shown when calling print
	/// @param[in] required if true the parameter is required. If a parameter is required but not found parse will return error
	template<typename T>
	ErrorCode addParam(const char* name, const char* description, bool required) {
		constexpr Type t = getType<T>();
		return addParam(name, description, t, required);
	}
	/// parse command line arguments that come from main
	/// @param[in] numArgs The number of the arguments
	/// @param[in] args An array of arguments that comes from main function where
	/// @param[in] errorCallback A function which will be called if an error in the parsing accures
	/// args[0] is the name of the program, args[1] is the first argument and so on
	/// @returns The code of the first error encounterd during the parsing or ErrorCode::Success in case of no errors
	ErrorCode parse(const int numArgs, char** args, ErrorCallbackT errorCallback = nullptr);
	/// Check if the parameter has been parsed. This is the way to check if flag parameter (param with no type) has been set.
	/// @param[in] name the name of the parameter
	/// @retval true if the parameter was parsed, false otherwise
	bool isSet(const char* name) const;
	/// Show the parameters with their description
	/// @param[in,out] f File stream where parameters will be printed
	void print(FILE* f);
	/// Delete all parameters as well as all parameter info
	void freeMem();
	/// Delete only the parameters, without deleting the info
	void freeValues();
private:
	enum class Type {
		Int,
		String,
		Flag
	};

	/// @brief Match C++ type to internal type
	/// @tparam T C++ type
	/// @return Internal type used in ParamInfo
	template<typename T>
	static constexpr Type getType() {
		if constexpr (std::is_same_v<T, int>) {
			return Type::Int;
		} else if constexpr (std::is_same_v<T, std::string>) {
			return Type::String;
		} else if constexpr (std::is_same_v<T, bool>) {
			return Type::Flag;
		} else {
			static_assert(sizeof(T) < 0, "Unknown type");
		}
	}
	/// Add a info for possible parameter that can come from the command line. All possible parameters must
	/// be added before calling parse. Calling this does not add a parameter only information.
	/// @param[in] name The name of the parameter, must not include - or =.
	/// @param[in] description The description for the parameter can include everything. This is shown when calling print
	/// @param[in] type The internal type of the parameter.
	/// @param[in] required if true the parameter is required. If a parameter is required but not found parse will return error
	ErrorCode addParam(const char* name, const char* description, const Type type, const bool required);

	struct ParamInfo {
		ParamInfo() = default;
		ParamInfo(const char* description, const Type type, const bool required);
		std::string description;
		Type type;
		bool required;
	};

	using ParamVal = std::variant<std::string, int, bool>;

	std::unordered_map<std::string, ParamInfo> paramInfo;
	std::unordered_map<std::string, ParamVal> paramValues;
	using ValuesConstIt = std::unordered_map<std::string, ParamVal>::const_iterator;
	using InfoConstIt = std::unordered_map<std::string, ParamInfo>::const_iterator;
};

}
