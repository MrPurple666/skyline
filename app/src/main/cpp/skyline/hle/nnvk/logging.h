#pragma once

#include <fmt/format.h>
#include "types.h"

namespace nnvk {
    struct Logger {
        enum class LogLevel {
            Error,
            Warn,
            Info,
            Debug,
        };

        static void Write(LogLevel level, const std::string &str);

        static bool IsEnabled(LogLevel level);

        /**
         * @brief A wrapper around a string which captures the calling function using Clang source location builtins
         * @note A function needs to be declared for every argument template specialization as CTAD cannot work with implicit casting
         * @url https://clang.llvm.org/docs/LanguageExtensions.html#source-location-builtins
         */
        template<typename S>
        struct FunctionString {
            S string;
            const char *function;

            FunctionString(S string, const char *function = __builtin_FUNCTION()) : string(std::move(string)), function(function) {}

            std::string operator*() {
                return std::string(function) + ": " + std::string(string);
            }
        };

        template<typename... Args>
        static void Error(FunctionString<const char *> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Error))
                Write(LogLevel::Error, fmt::format(fmt::runtime(*formatString), args...));
        }

        template<typename... Args>
        static void Error(FunctionString<std::string> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Error))
                Write(LogLevel::Error, fmt::format(fmt::runtime(*formatString), args...));
        }

        template<typename... Args>
        static void Warn(FunctionString<const char *> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Warn))
                Write(LogLevel::Warn, fmt::format(fmt::runtime(*formatString), args...));
        }

        template<typename... Args>
        static void Warn(FunctionString<std::string> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Warn))
                Write(LogLevel::Warn, fmt::format(fmt::runtime(*formatString), args...));
        }

        template<typename... Args>
        static void Info(FunctionString<const char *> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Info))
                Write(LogLevel::Info, fmt::format(fmt::runtime(*formatString), args...));
        }

        template<typename... Args>
        static void Info(FunctionString<std::string> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Info))
                Write(LogLevel::Info, fmt::format(fmt::runtime(*formatString), args...));
        }

        template<typename... Args>
        static void Debug(FunctionString<const char *> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Debug))
                Write(LogLevel::Debug, fmt::format(fmt::runtime(*formatString), args...));
        }

        template<typename... Args>
        static void Debug(FunctionString<std::string> formatString, Args &&... args) {
            if (IsEnabled(LogLevel::Debug))
                Write(LogLevel::Debug, fmt::format(fmt::runtime(*formatString), args...));
        }
    };
}

namespace nnvk::util {
    i64 GetTimeNs();
}

