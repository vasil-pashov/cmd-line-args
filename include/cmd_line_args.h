#pragma once
#include <unordered_map>
#include <string>
#include <optional>
#include <variant>

namespace CMD {

/// @brief Class to parse and hold command line arguments
///
/// How to use:
///		- Create CommandLineArgs class
/// 	- Add parameters via CommandLineArgs::addParam
/// 	- Parse the parameters passed to main via CommandLineArgs::parse
/// 	- Request parameters via CommandLineArgs::getValue
class CommandLineArgs {
public:
	CommandLineArgs() = default;
	~CommandLineArgs();

	CommandLineArgs(const CommandLineArgs&) = delete;
	CommandLineArgs& operator=(const CommandLineArgs&) = delete;

	CommandLineArgs(CommandLineArgs&&) = default;
	CommandLineArgs& operator=(CommandLineArgs&&) = default;

	/// @brief Holds all errors codes return from the class.
	enum class ErrorCode {
		Success = 0, ///< No errors occurred
		MissingParameter, ///< Parameter which was required is not passed
		WrongValueType, ///< The type of the value of a parameter is not the same as in the definition
		UnknownParameter, ///< Parsed parameter name which was not defined
		FlagHasValue, ///< Parameter with type bool was passed a value with -flag=value.
		MissingValue, ///< Parameter which requires a value was not passed a value with -param=value
		ParameterExists, ///< Trying to define a parameter more than once
		WrongParamFormat, ///< Parameter does not start with --
		UnknownParsingError ///< Unknown error during parsing
	};

	enum class Type {
		Int,
		String,
		Flag
	};

	template<Type T>
	struct TypeMatch {
	};

	template<>
	struct TypeMatch<Type::Int> {
		using type = int;
	};

	template<>
	struct TypeMatch<Type::String> {
		using type = std::string;
	};

	template<>
	struct TypeMatch<Type::Flag> {
		using type = bool;
	};

	/// @brief Type of the callback function which can be passed to parse
	/// @param[in] code The code of the error
	/// @param[in] The name of the parameter (without the dashes)
	/// @param[in] input The whole erroneous input passed for the parameter
	/// @param[in] description A description of the error
	typedef void(*ErrorCallbackT)(ErrorCode code, const char* param, const char* input, const char* description);

	/// Add a info for possible parameter that can come from the command line. All possible parameters must
	/// be added before calling parse. Calling this does not add a parameter only information.
	/// @tparam T The type of the parameter. It will be used when parsing the argv to ensure that the passed
	/// parameters are of type T. Allowed types: bool for flags, int, std::string.
	/// @param[in] name The name of the param, must not include - or =.
	/// @param[in] description The description for the parameter can include everything. This is shown when calling print
	/// @param[in] required if true the parameter is required. If a parameter is required but not found parse will return error
	template<typename NameT, typename DescT> [[nodiscard]]
	ErrorCode addParam(Type paramT, NameT&& name, DescT&& description, bool required);

	/// @brief Get the value of a parameter
	/// @tparam ParamT The type of the parameter. One of CMD::CommandLineArgs::Type.
	/// @param[in] param The name of the parameter
	/// @return Pointer to the value or nullptr.
	/// - If the passed type T is the same as the declared type of the parameter
	///   and the parameter was passed then return pointer to the value of the parameter.
	/// - If the type is bool and matches with the declared type:
	///		- If the parameter was passed, then the pointer is valid and points to true.
	///		- If the parameter was not passed, then the pointer is valid and points to false.
	/// - If the types don't match or the parameter was not passed (except for the case of bool parameters)
	///   then return nullptr.
	template<Type ParamT, typename OutT = TypeMatch<ParamT>::type>
	const OutT* getValue(const std::string& param);

	/// @brief Parse command line arguments that are parameters passed to main function.
	/// @param[in] numArgs The number of the arguments
	/// @param[in] args An array of arguments that comes from main function where
	/// args[0] is the name of the program, args[1] is the first argument and so on.
	/// @param[in] errorCallback A function which will be called if an error in the parsing happens
	/// @returns The code of the first error encountered during the parsing or ErrorCode::Success in case of no errors
	[[nodiscard]]
	ErrorCode parse(const int numArgs, char** args, ErrorCallbackT errorCallback = nullptr);
	/// @brief Show the parameters with their description
	/// @param[in,out] f File stream where parameters will be printed
	void print(FILE* f);
	/// @brief Delete all parameters as well as all parameter info
	void freeMem();
	/// @brief Delete only the parameters, without deleting the info
	void freeValues();
private:
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

	/// @brief Internal structure to hold information about each parameter
	struct ParamInfo {
		ParamInfo() = default;
		template<typename DescT>
		ParamInfo(DescT&& description, const Type type, const bool required) :
			description(std::forward<DescT>(description)),
			type(type),
			required(required)
		{ }
		/// Description of the parameter. Printed when CommandLineArgs::print is called
		std::string description;
		/// Internal type of parameter. Used when the value is parsed. Matches to at least
		/// one C++ type.
		Type type;
		/// If the parameter is required and not passed the parse will fail.
		bool required;
	};

	using ParamVal = std::variant<std::string, int, bool>;

	std::unordered_map<std::string, ParamInfo> paramInfo;
	std::unordered_map<std::string, ParamVal> paramValues;
};

template<CommandLineArgs::Type ParamT, typename OutT>
const OutT* CommandLineArgs::getValue(const std::string& param) {
	auto it = paramValues.find(param);

	if (it == paramValues.end()) {
		if constexpr (ParamT == Type::Flag) {
			const auto infoIt = paramInfo.find(param);
			if (infoIt != paramInfo.end() && infoIt->second.type == Type::Flag) {
				return &std::get<bool>((paramValues[param] = false));
			}
		}
		return nullptr;
	}
	
	if (!std::holds_alternative<OutT>(it->second)) {
		return nullptr;
	}
	
	return &std::get<OutT>(it->second);
}

template<typename NameT, typename DescT>
CommandLineArgs::ErrorCode CommandLineArgs::addParam(CommandLineArgs::Type paramT, NameT&& name, DescT&& description, bool required) {
	std::string paramName = std::forward<NameT>(name);
	auto it = paramInfo.find(paramName);
	if (it != paramInfo.end()) {
		return ErrorCode::ParameterExists;
	}
	paramInfo[std::move(paramName)] = ParamInfo(std::forward<DescT>(description), paramT, required);
	return ErrorCode::Success;
}

}
