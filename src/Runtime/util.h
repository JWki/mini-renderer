#pragma once

#include <stdint.h>
#include <string.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace mini {
    /*
        *   Win32 High Precision Timer
    */
    class Timer
    {
        uint64_t m_frequency;
        uint64_t m_timestamp;
    public:
        Timer()
        {
            LARGE_INTEGER frequency, timestamp;
            QueryPerformanceFrequency(&frequency);
            QueryPerformanceCounter(&timestamp);

            m_frequency = frequency.QuadPart;
            m_timestamp = timestamp.QuadPart;
        }

        void Reset() {
            LARGE_INTEGER timestamp;
            QueryPerformanceCounter(&timestamp);
            m_timestamp = timestamp.QuadPart;
        }

        double GetElapsedTime() {
            LARGE_INTEGER currentTime = {};
            QueryPerformanceCounter(&currentTime);
            return static_cast<double>(currentTime.QuadPart - m_timestamp) / static_cast<double>(m_frequency);
        }
    };

    class ByteStream
    {
        char* m_buffer = nullptr;
        uint32_t    m_offset = 0;
        uint32_t    m_bufferSize = 0;
    public:
        ByteStream() = default;
        ByteStream(void* buffer, uint32_t bufferSize)
            : m_buffer(reinterpret_cast<char*>(buffer)), m_offset(0), m_bufferSize(bufferSize) {}


        uint32_t    GetOffset() const { return m_offset; }
        char* GetBuffer() const { return m_buffer; }

        template <class T>
        void Read(T* target)
        {
            ReadBytes(target, sizeof(T));
        }

        void ReadBytes(void* target, uint32_t numBytes)
        {
            numBytes = numBytes <= m_bufferSize - m_offset ? numBytes : m_bufferSize - m_offset;
            if (target != nullptr && numBytes > 0)
            {
                memcpy(target, m_buffer + m_offset, numBytes);
            }
            m_offset += numBytes;
        }

        template <class T>
        void Write(T const& source)
        {
            WriteBytes(&source, sizeof(T));
        }

        void WriteBytes(void const* source, uint32_t numBytes)
        {
            numBytes = numBytes <= m_bufferSize - m_offset ? numBytes : m_bufferSize - m_offset;
            memcpy(m_buffer + m_offset, source, numBytes);
            m_offset += numBytes;
        }
    };


    inline void* Win32LoadFileContents(char const* path, uint64_t* outFileSize = nullptr)
    {
        HANDLE handle = CreateFileA(path, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (handle == INVALID_HANDLE_VALUE) {
            return nullptr;
        }
        DWORD size = GetFileSize(handle, NULL);
        void* buffer = malloc(size + 1);    // @note allocate 1 extra byte...
        memset(buffer, 0x0, size + 1);  // ...for text files to null terminate them if they aren't
        DWORD bytesRead = 0;
        auto res = ReadFile(handle, buffer, size, &bytesRead, NULL);
        if (outFileSize != nullptr) {
            *outFileSize = (uint64_t)size;
        }
        if (res == FALSE || bytesRead != size) {
            free(buffer);
            CloseHandle(handle);
            return nullptr;
        }
        CloseHandle(handle);
        return buffer;
    }
}