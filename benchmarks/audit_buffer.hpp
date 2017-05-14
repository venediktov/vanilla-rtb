#pragma once

/*
   Copyright 2017 Vladimir Lysyy (mrbald@github)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
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

namespace audit {

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
    std::atomic_flag flag_ { ATOMIC_FLAG_INIT };
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

private:
    ipx::file_mapping mapping_;
    ipx::mapped_region region_;

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
            segment_deleter_ { io_service_.wrap([] (log_segment* segment) noexcept {
                delete segment;
            }) },
            roll_segments_ { io_service_.wrap([this] (size_t segment_idx) {
                segments_[(segment_idx + 1) % segments_.size()] = make_next_segment();
            }) }
    {
        segments_[0] = make_next_segment();
        segments_[1] = make_next_segment();
    }

    /**
     * Reserves a requred space in the buffer, scrolling to a next segment if required
     */
    std::shared_ptr<char> checkout(size_t required_size) {
        if (unlikely(required_size > filesize_ - 1))
            throw std::runtime_error("too much memory requested");

        // TODO: VL: refine usage of atomics and the mutex
        guard_t guard(mutex_);

        auto segment_idx = segment_idx_.load(std::memory_order_acquire);
        auto segment = std::atomic_load(&segments_[segment_idx % segments_.size()]);

        size_t pos = segment_offset_;
        if (unlikely(pos + required_size >= segment->size() - 1)) { // one byte for End-of-Media
            *(segment->memory() + pos) = 0x19; // End-of-Media
            // GC the completed one (will be closed when the last reference to it is released)
            std::atomic_store(&segments_[segment_idx % segments_.size()], {});
            segment_idx = segment_idx_.fetch_add(1, std::memory_order_release) + 1;
            roll_segments_(segment_idx);
            segment = segments_[segment_idx % segments_.size()];
            segment_offset_ = pos = 0u;
        }

        segment_offset_ += required_size;

        return {segment, segment->memory() + pos}; // aliased shared_ptr to hold the segment alive
    }

private:
    std::shared_ptr<log_segment> make_next_segment() {
        return {new log_segment{filename_base_ + std::to_string(name_idx_++), filesize_}, segment_deleter_};
    }

    using mutex_t = spin_mutex;
    using guard_t = std::lock_guard<mutex_t>;

    mutable mutex_t mutex_;

    asio::io_service& io_service_;
    std::string const filename_base_ = "memory.bin.";
    size_t const filesize_;
    size_t name_idx_ = 0u;

    std::function<void(size_t)> roll_segments_;

    std::function<void(log_segment*)> segment_deleter_;

    std::array<std::shared_ptr<log_segment>, 3> segments_;
    std::atomic<size_t> segment_idx_ {0};
    size_t segment_offset_ {0};
}; // struct segmented_log_buffer

} // namespace audit
