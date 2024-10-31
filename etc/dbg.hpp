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
    namespace Debugger {
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

            std::string color = Color::colorize(l, cc);

            std::cout << "[" << color << "] ";
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
    } // namespace Debugger

    namespace Misc {
        inline static void pause() {
          // _WIN32 macro is already defined in x64 Windows
#if defined(_WIN32) || defined(__WIN32__) || defined(__NT__) && !(defined(__GNUC__) || defined(__clang__)) // Windows without GCC or Clang
            system("pause"); // :cry: ultimate death
#elif (defined(__GNUC__) || defined(__clang__)) && !defined(__APPLE__) // GCC and Clang andn MACOS DAMNED
            const char message[] = "Press any key to continue..."; // okay this is crazy

            asm (
                "mov $1, %%rax\n"         // syscall: write
                "mov $1, %%rdi\n"         // file descriptor: stdout
                "mov %0, %%rsi\n"         // message pointer
                "mov $30, %%rdx\n"        // length
                "syscall\n"               // invoke syscall
                :
                : "r"(message)
                : "rax", "rdi", "rsi", "rdx"
            );
#elif defined(__APPLE__)
            std::cout << "Press any key to continue..." << std::endl;
            std::cin.get();
#else
#error "Unsupported compiler" // we'll error for now
#endif
        }

        /**
          Logs a fatal error message and exits the program.
         *
          @param message The message to log.
          @param code Exit code (default is 1).
         */
        [[noreturn]] inline const static void fexit(const std::string& message, const int& code = 1) {
            Debugger::fatal(message);
            pause(); // Now this will work since pause is defined before fexit
            exit(code); // This will terminate the program (wow)
        }
    } // namespace Misc

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
            Debugger::info(message);
        }

        /**
          Logs a warning message using the Debugger class.
         *
          @param message The message to log.
         */
        template <typename T>
        inline constexpr void warn(const T& message) {
            Debugger::warn(message);
        }

        /**
          Logs an error message using the Debugger class.
         *
          @param message The message to log.
         */
        template <typename T>
        inline constexpr void error(const T& message) {
            Debugger::error(message);
        }

        /**
          Logs a fatal error message using the Debugger class.
         *
          @param message The message to log.
         */
        template <typename T>
        inline constexpr void fatal(const T& message) {
            Debugger::fatal(message);
        }
    } // namespace Macros
} // namespace dbg
