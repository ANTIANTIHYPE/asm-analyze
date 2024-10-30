#pragma once

#include <color.hpp>
#include <iostream>

/**
  A namespace for debugging-related functionality.
 */
namespace dbg {
    /**
      A class for logging debug messages with different levels of severity.
     */
    class Debugger {
    public:
        /**
          Enumerates the available log levels.
         */
        enum Level : uint8_t {
            /**
              Informational log level.
             */
            INFO,
            /**
              Warning log level.
             */
            WARN,
            /**
              Error log level.
             */
            ERR,
            /**
              Fatal error log level.
             */
            FATAL
        };

        /**
          Logs a message with the specified level.
         *
          @param message The message to log.
          @param level The log level to use (from the Level enum).
         */
        inline const static void log(const std::string& message, Level level) {
            std::string l{};
            Color::Code cc = Color::RED;

            switch (level) {
            case INFO:
                l = "INFO";
                cc = Color::GREEN;
                break;
            case WARN:
                l = "WARN";
                cc = Color::YELLOW;
                break;
            case ERR:
                l = "ERROR";
                break;
            case FATAL:
                l = "FATAL";
                break;
            }

            std::string colored = Color::colorize(l, cc);

            std::cout << "[" << colored << "] ";
            std::cout << message << '\n';
        }

        /**
          Logs an informational message.
         *
          @param message The message to log.
         */
        inline const static void info(const std::string& message) {
            log(message, INFO);
        }

        /**
          Logs a warning message.
         *
          @param message The message to log.
         */
        inline const static void warn(const std::string& message) {
            log(message, WARN);
        }

        /**
          Logs an error message.
         *
          @param message The message to log.
         */
        inline const static void error(const std::string& message) {
            log(message, ERR);
        }

        /**
          Logs a fatal error message.
         *
          @param message The message to log.
         */
        inline const static void fatal(const std::string& message) {
            log(message, FATAL);
        }
    };

    namespace Misc {
        /**
        Logs a fatal error message and exits the program.
        *
        @param message The message to log.
        @param code Exit code (default is 1).
        */
        inline const static void prefexit() {
            std::cout << "Press Enter to exit...";
            std::cin.sync();
        }

        [[noreturn]] inline const static void fexit(const std::string& message, const int& code = 1) {
            Debugger::fatal(message);
            prefexit(); // Now this will work since prefexit is defined before fexit
            exit(code); // This will terminate the program
        }
    }

    /**
      A namespace for debugging macros.
     */
    namespace Macros {
        /**
          Logs an informational message using the Debugger class.
         *
          @param message The message to log.
         */
        template <typename T>
        inline constexpr void info(const T& message) {
            dbg::Debugger::info(message);
        }

        /**
          Logs a warning message using the Debugger class.
         *
          @param message The message to log.
         */
        template <typename T>
        inline constexpr void warn(const T& message) {
            dbg::Debugger::warn(message);
        }

        /**
          Logs an error message using the Debugger class.
         *
          @param message The message to log.
         */
        template <typename T>
        inline constexpr void error(const T& message) {
            dbg::Debugger::error(message);
        }

        /**
          Logs a fatal error message using the Debugger class.
         *
          @param message The message to log.
         */
        template <typename T>
        inline constexpr void fatal(const T& message) {
            dbg::Debugger::fatal(message);
        }
    }
}