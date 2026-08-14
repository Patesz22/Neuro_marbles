#pragma once
#include <iostream>
#include <fstream>
#include <mutex>
#include <filesystem>
#include <cstdarg>

class Logger
{
public:
    static void Init();
    static void Shutdown();
    static void Printf(const char* format, ...);

private:
    static std::ofstream logFile;
    static std::mutex logMutex;
    static std::streambuf* originalCoutBuffer;

    // Custom stream buffer that writes to both the console AND our file
    class DualStreamBuf : public std::streambuf
    {
    public:
        DualStreamBuf(std::streambuf* original, std::ofstream& file);
    protected:
        virtual int overflow(int c) override;
        virtual std::streamsize xsputn(const char* s, std::streamsize n) override;
        virtual int sync() override;
    private:
        std::streambuf* originalBuffer;
        std::ofstream& outFile;
    };

    static DualStreamBuf* customBuf;
};

// This macro magically replaces all standard 'printf' calls with Logger Printf
#define printf Logger::Printf