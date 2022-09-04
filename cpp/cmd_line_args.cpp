#include "cmd_line_args.h"
#include <cstring>
#include <cassert>
#include <cstdio>
#include <format>

namespace CMD {

CommandLineArgs::ErrorCode CommandLineArgs::parse(const int argc, char** argv, ErrorCallbackT errorCallback) {
    // argv[0] is the name of the program (exe), that is why the for starts from 1
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (arg[0] == '-') {
            int j = 1;
            bool hasParams = false;
            while (arg[j]) {
                if (arg[j] == '=') {
                    hasParams = true;
                    break;
                }
                j++;
            }

            // -someArg=someVal -> arg + 1 to start past the '-' from 's'
            // j-1 is the length, subtract 1 to account for the parameter
            const int nameLen = j - 1;
            std::string name(arg + 1, nameLen);
            auto infoIterator = paramInfo.find(name);
            if (infoIterator == paramInfo.end()) {
                const ErrorCode code = ErrorCode::UnknownParameter;
                if (errorCallback) {
                    const std::string& error = std::format("Unknown parameter {}", name);
                    errorCallback(code, name.c_str(), arg, error.c_str());
                }
                return code;
            }

            // Check values passed to parameters
            // Flags should not have values
            // Other types must have values
            if (hasParams && infoIterator->second.type == Type::Flag) {
                const ErrorCode code = ErrorCode::FlagHasValue;
                if (errorCallback) {
                    const std::string& error = std::format(
                        "Parameter {} is a flag parameter. It does not have value!"
                        "Received input is: {}",
                        name.c_str(),
                        arg
                    );
                    errorCallback(code, name.c_str(), arg, error.c_str());
                }
                return code;
            } else if (!hasParams && infoIterator->second.type != Type::Flag) {
                const ErrorCode code = ErrorCode::MissingValue;
                if (errorCallback) {
                    const std::string& error = std::format(
                        "Parameter must receive value. The syntax is -parameter=value (no spaces surrounding =)"
                        "Received input is: {}",
                        arg
                    );
                    errorCallback(code, name.c_str(), arg, error.c_str());
                }
                return code;
            }
            
            // Parse the value for the parameter
            ParamVal val;
            const int paramStartIndex = j + 1;
            switch (infoIterator->second.type) {
                case Type::Int: {
                    char* next;
                    const int res = strtol(arg + paramStartIndex, &next, 0);
                    // strtol returns 0 if it cannot parse, so there are two possibilities to get 0
                    // a) It could not parse
                    // b) It parsed a zero
                    // So check if the number is 0
                    // second parameter of strtol returns which is the next char after the last parsed char
                    // we want it to be the end of the string
                    if ((res == 0 && arg[j + 1] != '0') || *next != '\0') {
                        const ErrorCode code = ErrorCode::WrongValueType;
                        if (errorCallback) {
                            const std::string& error = std::format(
                                "Wrong parameter value. Expected: -{}=[INTEGER_VALUE]"
                                "Received input is: {}",
                                arg,
                                arg
                            );
                            errorCallback(ErrorCode::WrongValueType, name.c_str(), arg, error.c_str());
                        }
                        return code;
                    }
                    val = res;
                } break;
                case Type::String: {
                    val = std::string(arg + paramStartIndex);
                } break;
                case Type::Flag: {
                    val = true;
                } break;
                default: {
                    assert(false);
                    const ErrorCode code = ErrorCode::UnknownParsingError;
                    if (errorCallback) {
                        const std::string& error = std::format(
                            "Unknown input %s. All arguments must start with \'-\' and use \'=\' for setting value."
                            "There must be no intervals surrounding \'=\' or after the \'-\'!",
                            arg
                        );
                        errorCallback(code, name.c_str(), arg, error.c_str());
                    }
                    return code;
                }
            }
            paramValues[name] = val;
        } else {
            const ErrorCode code = ErrorCode::WrongParamFormat;
            if (errorCallback) {
                const std::string& error = std::format(
                    "Unknown input %s. All arguments must start with \'-\' and use \'=\' for setting value."
                    "There must be no intervals surrounding \'=\' or after the \'-\'!",
                    arg
                );
                errorCallback(code, arg, arg, error.c_str());
            }
            return code;
        }
    }

    // Check if all required parameters are passed
    for (auto& it : paramInfo) {
        if (it.second.required && paramValues.find(it.first) == paramValues.end()) {
            const ErrorCode code = ErrorCode::MissingParameter;
            if (errorCallback) {
                const std::string& error = std::format("Required parameter {} cannot be found!", it.first.c_str());
                errorCallback(ErrorCode::MissingParameter, it.first.c_str(), "", error.c_str());
            }
            return code;
        }
    }
    return ErrorCode::Success;
}

CommandLineArgs::~CommandLineArgs() {
	freeMem();
}

bool CommandLineArgs::isSet(const std::string& param) const {
	return paramValues.find(param) != paramValues.end();
}

void CommandLineArgs::freeValues() {
	paramValues.clear();
}

void CommandLineArgs::freeMem() {
	freeValues();
	paramInfo.clear();
}

void CommandLineArgs::print(FILE* f) {
	for (auto& it : paramInfo) {
		fprintf(f, "%s - %s\n", it.first.c_str(), it.second.description.c_str());
	}
}

} // namespace CMD