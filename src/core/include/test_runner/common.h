#pragma once

#include <string>
#include <functional>
#include <optional>
#include <map>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <type_traits>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <memory>

#include <flecs.h>

namespace TestRunnerDetail {

// Source - https://stackoverflow.com/a/64490578
// Posted by einpoklum, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-17, License - CC BY-SA 4.0

// This next line assumes C++17; otherwise, replace it with
// your own string view implementation
#include <string_view>
#if __cplusplus >= 202002L
#  include <source_location>
#endif

template <typename T> constexpr std::string_view type_name();

template <>
constexpr std::string_view type_name<void>()
{
	return "void";
}

namespace detail {

	using type_name_prober = void;

	template <typename T>
	constexpr std::string_view wrapped_type_name()
	{
#if __cplusplus >= 202002L
		return std::source_location::current().function_name();
#else
#  if defined(__clang__) || defined(__GNUC__)
		return __PRETTY_FUNCTION__;
#  elif defined(_MSC_VER)
		return __FUNCSIG__;
#  else
#    error "Unsupported compiler"
#  endif
#endif // __cplusplus >= 202002L
	}

	constexpr std::size_t wrapped_type_name_prefix_length()
	{
		return wrapped_type_name<type_name_prober>()
			.find(type_name<type_name_prober>());
	}

	constexpr std::size_t wrapped_type_name_suffix_length()
	{
		return wrapped_type_name<type_name_prober>().length()
			- wrapped_type_name_prefix_length()
			- type_name<type_name_prober>().length();
	}

} // namespace detail

template <typename T>
constexpr std::string_view type_name()
{
	constexpr auto wrapped_name = detail::wrapped_type_name<T>();
	constexpr auto prefix_length = detail::wrapped_type_name_prefix_length();
	constexpr auto suffix_length = detail::wrapped_type_name_suffix_length();
	constexpr auto type_name_length =
		wrapped_name.length() - prefix_length - suffix_length;
	return wrapped_name.substr(prefix_length, type_name_length);
}

template <typename T>
constexpr std::string_view getTypeName() { 
	auto full = type_name<T>();

	size_t space_pos = full.find(' ');
	if (space_pos != std::string::npos) {
		return full.substr(space_pos + 1);
	}
	return full;
}

// ================================================================================================
template <typename Derived>
class AutoPrefixedError : public std::runtime_error {
public:
	explicit AutoPrefixedError(const char* message)
		: std::runtime_error(generatePrefix() + ": " + message)
	{}
	explicit AutoPrefixedError(const std::string& message)
		: std::runtime_error(generatePrefix() + ": " + message) 
	{}

private:
	// Helper to get the nice "Namespace::Class" string
	static std::string generatePrefix() {
		return std::string{ getTypeName<Derived>() };
	}
};

using WorldCallback = std::function<void(flecs::world&)>;
using ModuleImporterMap = std::map<std::string, WorldCallback>;

// ================================================================================================
enum class OperatorType {
	EQ,
	NEQ,
	LT,
	LTE,
	GT,
	GTE,
};

// ================================================================================================
/**
* Add as a component to all registered components
* to capture the presence of comparison hooks.
*/
struct SupportedOperators {
	bool equals = false;
	bool cmp = false;
};

// ================================================================================================
enum class LogLevel {
	FATAL = 0,
	ERROR,
	WARN,
	INFO,
	TRACE,
};

// ================================================================================================
class Log {
public:
	using Level = LogLevel;

	// ==============================================================================================
	// Proxy class to handle newline on destruction
	class LineLogger {
	public:
		LineLogger(std::ostream& os, bool active) : _os(os), _active(active) {}

		// Prevent copying to ensure it stays a temporary
		LineLogger(const LineLogger&) = delete;
		LineLogger& operator=(const LineLogger&) = delete;

		LineLogger(LineLogger&&) = default;
		LineLogger& operator=(LineLogger&&) = default;

		// Finalize the line when the temporary object is destroyed
		~LineLogger() {
			if (_active) {
				_os << "\n";
			}
		}

		// Ref-qualified operator<<: only works on temporaries (rvalues)
		template <typename T>
		LineLogger&& operator<<(const T& value)&& {
			if (_active) {
				_os << value;
			}
			return std::move(*this);
		}

		operator std::ostream&()&& {
			return _os;
		}

	private:
		std::ostream& _os;
		bool _active;
	};

	static LineLogger fatal() { return { std::cerr, _logLevel >= Level::FATAL }; }
	static LineLogger error() { return { std::cerr, _logLevel >= Level::ERROR }; }
	static LineLogger warn()  { return { std::cout, _logLevel >= Level::WARN  }; }
	static LineLogger info()  { return { std::cout, _logLevel >= Level::INFO  }; }
	static LineLogger trace() { return { std::cout, _logLevel >= Level::TRACE }; }

	static void setLevel(Level level) { _logLevel = level; }

private:
	inline static Level _logLevel = Level::WARN;
};

// ================================================================================================
/**
* Simple profiler to measure execution of a unit test.
*/
class RunTestProfiler {
public:
	using Clock = std::chrono::steady_clock;

	struct Probe {
		std::string name;
		Clock::time_point start;

		Probe(std::string testName)
			: name(std::move(testName)), start(Clock::now()) {}

		~Probe() {
			auto end = Clock::now();
			std::chrono::duration<double, std::micro> elapsed = end - start;
			RunTestProfiler::record(name, elapsed.count());
		}
	};

	static void init(size_t n) {
		_targetCount = n;
		_results.clear();
		_results.reserve(n);
		_isEnabled = true;
	}

	static bool isEnabled() {
		return _isEnabled;
	}

private:
	using TimePerTestMap = std::unordered_map<std::string, double>;
	static inline TimePerTestMap _results;
	static inline size_t _targetCount = 0;
	static inline bool _isEnabled = false;

	static void record(const std::string& name, double duration) {
		_results[name] = duration;

		if (_results.size() == _targetCount) {
			generateReport();
		}
	}

	static void generateReport() {
		auto now = std::time(nullptr);
		auto* tm = std::localtime(&now);
		char ts[32];
		std::strftime(ts, sizeof(ts), "%H-%M-%S_%d_%m", tm);

		std::string filename = "perf_" + std::to_string(_targetCount) + "_tests_" + std::string(ts) + ".csv";
		std::ofstream file(filename);

		std::string header = "Test Name,Duration (us)\n";
		std::string body;
		for (const auto& [name, duration] : _results) {
			body += name + "," + std::to_string(duration) + "\n";
		}

		if (file.is_open()) {
			file << header << body;
			file.close();
		}

		_results.clear();

		std::cout << "\n--- PROFILER REACHED " << _targetCount << " TEST INVOCATIONS ---\n";
		std::cout << "File saved: " << filename << "\n";
		std::cout << header << body;
		std::cout << "-----------------------------------------------\n" << std::endl;
	}
};

}
