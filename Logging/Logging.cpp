#include "Logging.h"
#include "Config/NeuroConfig.h"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <vector>

// Define static members
std::ofstream Logger::logFile;
std::mutex Logger::logMutex;
std::streambuf* Logger::originalCoutBuffer = nullptr;
Logger::DualStreamBuf* Logger::customBuf = nullptr;

// --- DualStreamBuf Implementation ---
Logger::DualStreamBuf::DualStreamBuf(std::streambuf* original, std::ofstream& file)
	: originalBuffer(original), outFile(file)
{}

int Logger::DualStreamBuf::overflow(int c)
{
	if (c != EOF)
	{
		std::lock_guard<std::mutex> lock(Logger::logMutex);
		if (originalBuffer) originalBuffer->sputc(c);
		if (outFile.is_open()) outFile.put(c);
	}
	return c;
}

std::streamsize Logger::DualStreamBuf::xsputn(const char* s, std::streamsize n)
{
	std::lock_guard<std::mutex> lock(Logger::logMutex);
	if (originalBuffer) originalBuffer->sputn(s, n);
	if (outFile.is_open())
	{
		outFile.write(s, n);
		outFile.flush(); // Ensure real-time logging for crash debugging
	}
	return n;
}

int Logger::DualStreamBuf::sync()
{
	std::lock_guard<std::mutex> lock(Logger::logMutex);
	if (originalBuffer) originalBuffer->pubsync();
	if (outFile.is_open()) outFile.flush();
	return 0;
}

// --- Logger Core Implementation ---
void Logger::Init()
{
	// Create logs directory if it doesn't exist
	std::filesystem::create_directory("logs");

	// Generate Timestamp (e.g., 2026-07-21_18-30-23)
	auto now = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	std::tm timeInfo;
	localtime_s(&timeInfo, &now_c); // Use localtime_s for thread safety on Windows

	std::string logDir = GetConfigString("LogDirectory", "logs/");

	if (!logDir.empty() && logDir.back() != '/' && logDir.back() != '\\')
	{
		logDir += "/";
	}

	std::stringstream filename;
	filename << logDir << "Log_" << std::put_time(&timeInfo, "%Y-%m-%d_%H-%M-%S") << ".txt";

	logFile.open(filename.str(), std::ios::out | std::ios::app);

	// 4. Hijack std::cout
	if (logFile.is_open())
	{
		originalCoutBuffer = std::cout.rdbuf();
		customBuf = new DualStreamBuf(originalCoutBuffer, logFile);
		std::cout.rdbuf(customBuf);

		std::cout << "[Logger] Initialized successfully. File: " << filename.str() << "\n";
	}
}

void Logger::Shutdown()
{
	// Restore original cout buffer so the game doesn't crash on exit
	if (originalCoutBuffer)
	{
		std::cout.rdbuf(originalCoutBuffer);
		originalCoutBuffer = nullptr;
	}

	if (customBuf)
	{
		delete customBuf;
		customBuf = nullptr;
	}

	if (logFile.is_open())
	{
		logFile.close();
	}
}

// custom printf wrapper
void Logger::Printf(const char* format, ...)
{
	// Process the varargs
	va_list args;
	va_start(args, format);

	// Determine required length
	int len = std::vsnprintf(nullptr, 0, format, args);
	va_end(args);

	if (len > 0)
	{
		std::vector<char> buffer(len + 1);
		va_start(args, format);
		std::vsnprintf(&buffer[0], buffer.size(), format, args);
		va_end(args);

		std::cout << &buffer[0];
	}
}