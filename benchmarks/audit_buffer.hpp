#pragma once

/*
   Copyright 2017 Vladimir Lysyy (mrbald@github)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
namespace ipx = boost::interprocess;

#include <boost/asio/io_service.hpp>
namespace asio = boost::asio;
#include <fstream>

#include <mutex>
#include <atomic>
#include <array>

namespace auditor {

#if __x86_64__
inline void zzz() noexcept { asm volatile("pause\n": : :"memory"); }
#else
inline void zzz() noexcept {}
#endif

#define likely(x)       __builtin_expect(!!(x), true)
#define unlikely(x)     __builtin_expect(!!(x), false)

struct null_mutex
{
    void lock() noexcept {}
    void unlock() noexcept {}
    bool try_lock() noexcept { return true; }
}; // struct null_mutex

struct spin_mutex
{
    void lock() noexcept
    {
        while (flag_.test_and_set(std::memory_order_acquire)) zzz();
    }

    void unlock() noexcept
    {
        flag_.clear(std::memory_order_release);
    }

    bool try_lock() noexcept
    {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

private:
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
}; // struct spin_mutex

/**
 * A single memory mapped file (whole file is mapped)
 */
struct log_segment {
    log_segment(std::string const& filename, size_t filesize):
            mapping_{create_file(filename, filesize).c_str(), ipx::read_write},
            region_{mapping_, ipx::read_write} {}

    char* memory() const noexcept {
        return reinterpret_cast<char*>(region_.get_address());
    }

    size_t size() const noexcept {
        return region_.get_size();
    }

    char const* filename() const noexcept {
        return mapping_.get_name();
    }

    /**
     * Advance by `size` bytes and returns the "would write" position.
     * NB: Does not care if the position + size overflow the memory,
     *     so special care should be taken by the caller to not.
     */
    size_t reserve(size_t size) {
        return offset_.fetch_add(size, std::memory_order_acq_rel);
    }
private:
    ipx::file_mapping const mapping_;
    ipx::mapped_region const region_;
    std::atomic<size_t> offset_ {0};

    static std::string create_file(std::string const& filename, size_t filesize) {
        ipx::file_mapping::remove(filename.c_str());

        std::filebuf fbuf;
        fbuf.open(filename.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);

        //Set the size
        fbuf.pubseekoff(filesize - 1, std::ios_base::beg);
        fbuf.sputc(0);
        return filename;
    }
}; // struct log_segment


/**
 * Thread safe chunked log buffer, automatically switching to the next file
 * when no space is left in the current one.
 */
struct segmented_log_buffer {
    segmented_log_buffer(asio::io_service& io_service, std::string const& dirname, size_t filesize):
            io_service_ {io_service},
            filename_base_ {dirname + "/memory.bin."},
            filesize_ {filesize},
            request_next_segment_ { io_service_.wrap([this] {
                prep_next_segment();
            }) },
            segment_deleter_ { io_service_.wrap([] (log_segment* segment) noexcept {
                delete segment;
            }) }
    {
        prep_next_segment();
        prep_next_segment();
    }

    /**
     * Reserves the  requred space in the buffer, jumping to the next segment if required
     */
    std::shared_ptr<char> reserve(size_t required_size) {
        if (unlikely(required_size > filesize_ - 1))
            throw std::runtime_error("too much memory requested");

        std::shared_ptr<log_segment> segment;
        size_t pos = 0u;
        do {
            auto const segment_idx = segment_idx_.load(std::memory_order_acquire);
            auto const psegment = &segments_[segment_idx % segments_.size()];

            { // effectively std::atomic_load with custom spinlock
                guard_t guard {mutex_};
                segment = *psegment;
            }

            if (likely(segment)) {
                pos = segment->reserve(required_size);
                auto const last_pos = segment->size() - 1;
                if (unlikely(pos < last_pos && (pos + required_size >= last_pos))) {
                    // it's this request that crossed the segment capacity

                    { // effectively std::atomic_store with custom spinlock
                        guard_t guard {mutex_};
                        psegment->reset();
                    }

                    request_next_segment_();

                    *(segment->memory() + pos) = 0x19; // End-of-Media
                    segment.reset();

                    // publish the next segment
                    segment_idx_.store(segment_idx + 1, std::memory_order_release);
                }
                else if (unlikely(pos >= last_pos)) {
                    segment.reset();
                }
            }
        } while (unlikely(!segment));

        return {segment, segment->memory() + pos}; // aliased shared_ptr to hold the segment alive
    }

private:
    void prep_next_segment() {
        std::string const filename = filename_base_ + std::to_string(make_segment_idx_);
        std::shared_ptr<log_segment> segment {new log_segment{filename, filesize_}, segment_deleter_};

        auto* psegment = &segments_[(make_segment_idx_++) % segments_.size()];
        {
            guard_t guard {mutex_};
            assert(!*psegment);
            psegment->swap(segment);
        }
    }

    using mutex_t = spin_mutex;
    using guard_t = std::lock_guard<mutex_t>;

    mutable mutex_t mutex_; // protects segments_[*]

    asio::io_service& io_service_;
    std::string const filename_base_ = "memory.bin.";
    size_t const filesize_;

    std::function<void()> const request_next_segment_;

    std::function<void(log_segment*)> const segment_deleter_;

    std::array<std::shared_ptr<log_segment>, 3> segments_;
    std::atomic<size_t> segment_idx_ {0};
    size_t make_segment_idx_ = 0u;

}; // struct segmented_log_buffer

} // namespace auditor
