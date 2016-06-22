/*
 @ 0xCCCCCCCC
*/

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef KBASE_PICKLE_H_
#define KBASE_PICKLE_H_

#include <cstdint>
#include <string>

namespace kbase {

class Pickle;

class PickleReader {
public:
    explicit PickleReader(const Pickle& pickle);

    bool Read(bool* result);

    bool Read(int* result);

    bool Read(uint32_t* result);

    bool Read(int64_t* result);

    bool Read(uint64_t* result);

    bool Read(float* result);

    bool Read(double* result);

    bool Read(std::string* result);

    bool Read(std::wstring* result);

    bool ReadBytes(const char** data, int length);

    bool ReadData(const char** data, int* read_length);

    bool SkipBytes(int num_bytes);

private:
    template<typename T>
    inline bool ReadBuiltIninType(T* result);

    template<typename T>
    inline const char* GetReadPointerAndAdvance();

    const char* GetReadPointerAndAdvance(int num_bytes);

    // When the size of element doesn't equal to sizeof(char), use this function
    // for safety consieration. this function runs overflow check on int32 num_bytes.
    const char* GetReadPointerAndAdvance(int num_elements, size_t element_size);

private:
    const char* read_ptr_;
    const char* read_end_ptr_;
};


// Underlying memory layout:
// <---------------- capacity -------------->
// +------+-----+-----+-+-----+---+-----+---+
// |header|seg_1|seg_2|#|seg_3|...|seg_n|   |
// +------+-----+-----+-+-----+---+-----+---+
//        <---------- payload ---------->
// Note that, every segment starts on the address that is 4-byte aligned, thus
// there might be a padding between two logically consecutive segments.

class Pickle {
private:
    struct Header {
        uint32_t payload_size;
    };

public:
    Pickle();

    // Creates from a given serialized buffer.
    Pickle(const void* data, size_t size_in_bytes);

    Pickle(const Pickle& other);

    Pickle(Pickle&& other) noexcept;

    Pickle& operator=(const Pickle& rhs);

    Pickle& operator=(Pickle&& rhs) noexcept;

    ~Pickle();

    // Returns the size of internal data, including header.
    inline size_t size() const;

    // Returns true, if no payload there, i.e. payload_size == 0.
    // Returns false, otherwise.
    inline bool empty() const;

    inline const void* data() const;

    inline bool Write(bool value);

    inline bool Write(int value);

    inline bool Write(uint32_t value);

    inline bool Write(int64_t value);

    inline bool Write(uint64_t value);

    inline bool Write(float value);

    inline bool Write(double value);

    bool Write(const std::string& value);

    bool Write(const std::wstring& value);

    // Serializes data in byte with specified length. PoD types only.
    // These functions guarantee that the internal data remains unchanged if the
    // funtion fails.
    bool WriteByte(const void* data, int data_len);

    bool WriteData(const char* data, int length);

    inline const char* payload() const;

    inline size_t payload_size() const;

private:
    // Resizes the capacity of the internal buffer. This function internally rounds the
    // `new_capacity` up to the nearest multiple of predefined storage unit.
    void Resize(size_t new_capacity);

    // Locates to the next uint32-aligned offset, and resize internal buffer if
    // necessary.
    // Returns the location that the data should be written at, or nullptr if
    // an error occured.
    char* BeginWrite(size_t length);

    // Zeros pading memory; otherwise some memory detectors may complain about
    // uninitialized memory.
    void EndWrite(char* dest, size_t length);

    inline char* mutable_payload() const;

    inline const char* end_of_payload() const;

private:
    static constexpr const size_t kCapacityUnit = 64U;
    Header* header_;
    size_t capacity_;

    friend class PickleReader;
};

inline size_t Pickle::size() const
{
    return sizeof(Header) + header_->payload_size;
}

inline bool Pickle::empty() const
{
    return payload_size() == 0;
}

inline const void* Pickle::data() const
{
    return header_;
}

inline size_t Pickle::payload_size() const
{
    return header_->payload_size;
}

inline char* Pickle::mutable_payload() const
{
    return const_cast<char*>(payload());
}

inline const char* Pickle::payload() const
{
    return reinterpret_cast<const char*>(header_) + sizeof(Header);
}

inline const char* Pickle::end_of_payload() const
{
    return payload() + payload_size();
}

inline bool Pickle::Write(bool value)
{
    return Write(value ? 1 : 0);
}

inline bool Pickle::Write(int value)
{
    return WriteByte(&value, sizeof(value));
}

inline bool Pickle::Write(uint32_t value)
{
    return WriteByte(&value, sizeof(value));
}
inline bool Pickle::Write(int64_t value)
{
    return WriteByte(&value, sizeof(value));
}
inline bool Pickle::Write(uint64_t value)
{
    return WriteByte(&value, sizeof(value));
}
inline bool Pickle::Write(float value)
{
    return WriteByte(&value, sizeof(value));
}
inline bool Pickle::Write(double value)
{
    return WriteByte(&value, sizeof(value));
}

}   // namespace kbase

#endif  // KBASE_PICKLE_H_