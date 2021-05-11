/*
 *  Copyright 2004 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <sys/uio.h>

#include "logger.hpp"
#include "utility.hpp"

namespace drpc {

// NOTE: Modify by pengwang7.

static const char k2CRLF[] = "\r\n\r\n";

namespace internal {

// (Internal; please don't use outside this file.) Determines if elements of
// type U are compatible with a BufferT<T>. For most types, we just ignore
// top-level const and forbid top-level volatile and require T and U to be
// otherwise equal, but all byte-sized integers (notably char, int8_t, and
// uint8_t) are compatible with each other. (Note: We aim to get rid of this
// behavior, and treat all types the same.)
template <typename T, typename U>
struct BufferCompat {
    static constexpr bool value =
        !std::is_volatile<U>::value &&
        ((std::is_integral<T>::value && sizeof(T) == 1)
         ? (std::is_integral<U>::value && sizeof(U) == 1)
         : (std::is_same<T, typename std::remove_const<U>::type>::value));
};

// Determines if the given class has zero-argument .data() and .size() methods
// whose return values are convertible to T* and size_t, respectively.
template <typename DS, typename T>
class HasDataAndSize {
private:
    template <
        typename C,
        typename std::enable_if<
            std::is_convertible<decltype(std::declval<C>().data()), T*>::value &&
            std::is_convertible<decltype(std::declval<C>().size()),
                                std::size_t>::value>::type* = nullptr>

    static int Test(int);

    template <typename>
    static char Test(...);

public:
    static constexpr bool value = std::is_same<decltype(Test<DS>(0)), int>::value;
};

}  // namespace internal

class ByteBufferWriter;
class ByteBufferReader;

// Basic buffer class, can be grown and shrunk dynamically.
// Unlike std::string/vector, does not initialize data when increasing size.
// If "ZeroOnFree" is true, any memory is explicitly cleared before releasing.
// The type alias "ZeroOnFreeBuffer" below should be used instead of setting
// "ZeroOnFree" in the template manually to "true".
template <typename T, bool ZeroOnFree = true>
class BufferT {
    static_assert(std::is_trivial<T>::value, "T must be a trivial type.");

    static_assert(std::is_const<T>::value == false, "T may not be const.");

    friend class ByteBufferReader;

    friend class ByteBufferWriter;

public:
    using value_type = T;

    using const_iterator = const T*;

    explicit BufferT(size_t capacity = 1024)
        : capacity_(capacity),
          read_index_(0),
          write_index_(0),
          data_(capacity_ > 0 ?
                new T[capacity_] {
        0
    } : nullptr) {
        DASSERT(IsConsistent(), "BufferT error.");
    }

    BufferT(const BufferT&) = delete;

    BufferT& operator=(const BufferT&) = delete;

    ~BufferT() {
        MaybeZeroCompleteBuffer();
    }

    BufferT& operator=(BufferT&& buf) {
        DASSERT(IsConsistent(), "BufferT error.");
        MaybeZeroCompleteBuffer();
        capacity_ = buf.capacity_;
        read_index_ = buf.read_index_;
        write_index_ = buf.write_index_;
        std::swap(data_, buf.data_);
        buf.data_.reset();
        buf.OnMovedFrom();

        return *this;
    }

    bool operator==(const BufferT& buf) const {
        DASSERT(IsConsistent(), "BufferT error.");
        if (size() != buf.size()) {
            return false;
        }

        for (size_t i = read_index_; i < write_index_; ++ i) {
            if (data_[i] != buf.data_[i]) {
                return false;
            }
        }

        return true;
    }

    bool operator!=(const BufferT& buf) const {
        return !(*this == buf);
    }

    T& operator[](size_t index) {
        DASSERT(index < write_index_, "BufferT error.");
        return data()[index];
    }

    T operator[](size_t index) const {
        DASSERT(index < write_index_, "BufferT error.");
        return data()[index];
    }

    // The AppendData functions add data to the end of the buffer. They accept
    // the same input types as the constructors.
    template <typename U,
              typename std::enable_if<
                  internal::BufferCompat<T, U>::value>::type* = nullptr>
    void AppendData(const U* data, size_t size) {
        DASSERT(IsConsistent(), "BufferT error.");

        if (WritableByteSize() < size * sizeof(U)) {
            EnsureCapacityWithHeadroom(size * sizeof(U), true);
        }

        static_assert(sizeof(T) == sizeof(U), "");
        memmove(end(), data, size * sizeof(U));
        write_index_ += size * sizeof(U);

        DASSERT(IsConsistent(), "BufferT error.");
    }

    template <typename U,
              size_t N,
              typename std::enable_if<
                  internal::BufferCompat<T, U>::value>::type* = nullptr>
    void AppendData(const U (&array)[N]) {
        AppendData(array, N);
    }

    template <typename W,
              typename std::enable_if<
                  internal::HasDataAndSize<const W, const T>::value>::type* = nullptr>
    void AppendData(const W& w) {
        AppendData(w.data(), w.size());
    }

    template <typename U,
              typename std::enable_if<
                  internal::BufferCompat<T, U>::value>::type* = nullptr>
    void AppendData(const U& item) {
        AppendData(&item, 1);
    }

    // The RecvData function read data from socket and add it to the end for
    // the buffer. An extra space is used to solve the problem of excessive
    // buffer initialization.
    template <typename U = T,
              typename std::enable_if<
                  internal::BufferCompat<T, U>::value>::type* = nullptr>
    ssize_t RecvData(int socket) {
        static_assert(sizeof(T) == sizeof(U), "");
        DASSERT(IsConsistent(), "BufferT error.");

        U extra_space[65535] = {0};
        struct iovec iove[2];
        int iovs = (WritableByteSize() < sizeof(extra_space)) ? 2 : 1;

        iove[0].iov_base = end();
        iove[0].iov_len = WritableByteSize();
        iove[1].iov_base = extra_space;
        iove[1].iov_len = sizeof(extra_space);

        ssize_t bytes_transferred = readv(socket, iove, iovs);
        if (bytes_transferred <= 0) {
            if (EVUTIL_ERR_RW_RETRIABLE(bytes_transferred)) {
                return RETRIABLE_ERROR;
            }
        } else if (static_cast<size_t>(bytes_transferred) <= WritableByteSize()) {
            write_index_ += bytes_transferred;
        } else {
            write_index_ = capacity_;
            AppendData(extra_space, bytes_transferred - WritableByteSize());
        }

        DASSERT(IsConsistent(), "BufferT error.");

        return bytes_transferred;
    }

    // Ensure that the buffer size can be increased to at least capacity without
    // further reallocation. (Of course, this operation might need to reallocate
    // the buffer.)
    void EnsureCapacity(size_t capacity) {
        // Don't allocate extra headroom, since the user is asking for a specific
        // capacity.
        EnsureCapacityWithHeadroom(capacity, false);
    }

    // Resets the buffer to zero size without altering capacity. Works even if the
    // buffer has been moved from.
    void clear() {
        MaybeZeroCompleteBuffer();
        read_index_ = 0;
        write_index_ = 0;
        DASSERT(IsConsistent(), "BufferT error.");
    }

    // Swaps two buffers. Also works for buffers that have been moved from.
    friend void swap(BufferT& a, BufferT& b) {
        using std::swap;
        swap(a.capacity_, b.capacity_);
        swap(a.read_index_, b.read_index_);
        swap(a.write_index_, b.write_index_);
        swap(a.data_, b.data_);
    }

    // Get the buffer unread byte size.
    size_t UnreadByteSize() const {
        DASSERT(IsConsistent(), "BufferT error.");
        return (write_index_ - read_index_) * sizeof(T);
    }

    // Get the buffer prepend byte size.
    size_t PrepensiveByteSize() const {
        DASSERT(IsConsistent(), "BufferT error.");
        return read_index_ * sizeof(T);
    }

    // Get the buffer writable byte size.
    size_t WritableByteSize() const {
        DASSERT(IsConsistent(), "BufferT error.");
        return (capacity_ - write_index_) * sizeof(T);
    }

    // Get a pointer to the data. Just .data() will give you a (const) T*, but if
    // T is a byte-sized integer, you may also use .data<U>() for any other
    // byte-sized integer U.
    template <typename U = T,
              typename std::enable_if<
                  internal::BufferCompat<T, U>::value>::type* = nullptr>
    const U* data() const {
        DASSERT(IsConsistent(), "BufferT error.");
        return reinterpret_cast<U*>(data_.get());
    }

    template <typename U = T,
              typename std::enable_if<
                  internal::BufferCompat<T, U>::value>::type* = nullptr>
    U* data() {
        DASSERT(IsConsistent(), "BufferT error.");
        return reinterpret_cast<U*>(data_.get());
    }

    bool empty() const {
        DASSERT(IsConsistent(), "BufferT error.");
        return UnreadByteSize() == 0;
    }

    size_t size() const {
        DASSERT(IsConsistent(), "BufferT error.");
        return UnreadByteSize();
    }

    size_t capacity() const {
        DASSERT(IsConsistent(), "BufferT error.");
        return capacity_;
    }

    T* begin() {
        return data() + read_index_;
    }

    T* end() {
        return data() + write_index_;
    }

    const T* begin() const {
        return data() + read_index_;
    }

    const T* end() const {
        return data() + write_index_;
    }

    const char* Find2CRLF() const {
        const char* crlf = std::search(begin(), end(), k2CRLF, k2CRLF + 4);
        return crlf == end() ? nullptr : crlf;
    }

private:
    void EnsureCapacityWithHeadroom(size_t capacity, bool extra_headroom) {
        DASSERT(IsConsistent(), "BufferT error.");

        size_t unread_byte_size = UnreadByteSize();
        size_t avaiable_byte_size = PrepensiveByteSize() + WritableByteSize();

        if (avaiable_byte_size < capacity) {
            // If the caller asks for extra headroom, ensure that the new capacity is
            // >= 1.5 times the old capacity. Any constant > 1 is sufficient to prevent
            // quadratic behavior; as to why we pick 1.5 in particular, see
            // https://github.com/facebook/folly/blob/master/folly/docs/FBVector.md and
            // http://www.gahcep.com/cpp-internals-stl-vector-part-1/.
            const size_t new_capacity =
                extra_headroom ? std::max(capacity, capacity_ + capacity_ / 2)
                : capacity;

            std::unique_ptr<T[]> new_data(new T[new_capacity] {0});
            if (data_ != nullptr) {
                std::memcpy(new_data.get(), begin(), unread_byte_size);
            }

            data_ = std::move(new_data);
            capacity_ = new_capacity;
            read_index_ = 0;
            write_index_ = unread_byte_size;
        } else {
            memmove(data_.get(), begin(), unread_byte_size);
            write_index_ = unread_byte_size;
        }

        DASSERT(UnreadByteSize() == unread_byte_size, "BufferT error.");
        DASSERT(WritableByteSize() >= capacity, "BufferT error.");
        DASSERT(IsConsistent(), "BufferT error.");
    }

    // Zero the complete buffer if template argument "ZeroOnFree" is true.
    void MaybeZeroCompleteBuffer() {
        if (ZeroOnFree && capacity_ > 0) {
            // It would be sufficient to only zero "size_" elements, as all other
            // methods already ensure that the unused capacity contains no sensitive
            // data---but better safe than sorry.
            memset(data_.get(), 0, capacity_ * sizeof(T));
        }
    }

    // Precondition for all methods except Clear, operator= and the destructor.
    // Postcondition for all methods except move construction and move
    // assignment, which leave the moved-from object in a possibly inconsistent
    // state.
    bool IsConsistent() const {
        return (data_ || capacity_ == 0) &&
               write_index_ >= read_index_ && write_index_ <= capacity_;
    }

    void OnMovedFrom() {
        // Our heap block should have been stolen.
        DASSERT(!data_, "BufferT error.");

        // Ensure that *this is always inconsistent, to provoke bugs.
        capacity_ = 0;
        read_index_ = 1;
        write_index_ = 0;
    }

private:
    size_t capacity_;

    size_t read_index_;

    size_t write_index_;

    std::unique_ptr<T[]> data_;
};

// By far the most common sort of buffer.
using Buffer = BufferT<char>;

} // namespace drpc

#endif // __BUFFER_HPP__
