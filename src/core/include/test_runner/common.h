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

#include <flecs.h>

namespace TestRunnerDetail {

// This handles the difference between Windows (MSVC) and Linux/Mac (GCC/Clang)
#if defined(_MSC_VER) // Windows
#include <typeinfo>
template <typename T>
static std::string getTypeName() {
	std::string name = typeid(T).name();
	// MSVC returns "struct MyModule" or "class MyModule". 
	// We remove the prefix to get just "MyModule".
	size_t space_pos = name.find(' ');
	if (space_pos != std::string::npos) {
		return name.substr(space_pos + 1);
	}
	return name;
}
#else // Linux / macOS / GCC / Clang
#include <cxxabi.h>
#include <typeinfo>
#include <memory>
template <typename T>
static std::string getTypeName() {
	int status;
	// specific GCC/Clang API to "demangle" the weird internal names
	std::unique_ptr<char, void(*)(void*)> res{
			abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
			std::free
	};
	return (status == 0) ? res.get() : typeid(T).name();
}
#endif

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
		return getTypeName<Derived>();
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
* Also add to all primitive value components.
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

		//std::cout << "\nTest Name,Duration (ns)\n";
		//std::cout << name + "," + std::to_string(duration) + "\n";

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
