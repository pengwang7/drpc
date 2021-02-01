/*
 * MIT License
 *
 * Copyright (c) 2020 pengwang7
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __TIMING_WHEEL_HPP__
#define __TIMING_WHEEL_HPP__

#include <list>
#include <unordered_set>

#include "async_cbs.hpp"

// The module implement a Timing-Wheel, 
// It only used to timeout mechanism for TCP connections,
// used it replace one connection with a timer.

namespace drpc {

template <class T>
class RingBuffer {
public:
    explicit RingBuffer(size_t size = 0) {
        size_ = size;
    }

    ~RingBuffer() {
        list_.clear();
        size_ = 0;
    }

public:
    void push_back(T elem) {
        if (list_.size() == size_) {
            list_.pop_front();
        }

        list_.push_back(elem);
    }

    void resize(size_t size) {
        size_ = size;
    }

    bool empty() { return list_.empty(); }

    bool full() { return list_.size() == size_; }

    size_t size() { return list_.size(); }

    T& back() { return list_.back(); }

private:
    size_t size_;
    std::list<T> list_;
};

struct Entry;

using WeakEntryPtr = std::weak_ptr<Entry>;

using EntryPtr = std::shared_ptr<Entry>;

using Bucket = std::unordered_set<EntryPtr>;

using CycleBucket = RingBuffer<Bucket>;

struct Entry {
    Entry(const weak_channel_ptr chan)
        : chan_(chan) {

    }

    ~Entry() {
        channel_ptr chan = chan_.lock();
        if (chan) {
            chan->Close();
        }
    }

    weak_channel_ptr chan_;
};

class TimingWheel {
public:
    explicit TimingWheel(size_t slot_size, size_t step)
        : slot_size_(slot_size), step_(step) {
        cycle_buckets_.resize(slot_size_);
    }

    ~TimingWheel() {

    }

public:
    void push_back(EntryPtr elem) {
        cycle_buckets_.back().insert(elem);
    }

    void push_bucket(Bucket elem) {
        cycle_buckets_.push_back(elem);
    }

private:
    size_t slot_size_;

    size_t step_;

    CycleBucket cycle_buckets_;
};

} /* end namespace drpc */

#endif /* __TIMING_WHEEL_HPP__ */
