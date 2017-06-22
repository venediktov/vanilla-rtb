#include <benchmark/benchmark.h>

#include "audit_codec.hpp"
#include "audit_buffer.hpp"

#include <thread>

namespace {

struct AuditBufferBenchmarkFixture: benchmark::Fixture
{
    asio::io_service io_service;

    std::thread thread {[&] {
        asio::io_service::work work_{io_service};
        io_service.run();
    }};

    auditor::segmented_log_buffer logbuf {io_service, ".", 1 << 25};

    ~AuditBufferBenchmarkFixture()
    {
        io_service.stop();
        if (thread.joinable()) {
            thread.join();
        }
    }
}; // AuditBufferBenchmarkFixture

static auto rec = std::make_tuple(int8_t('a'), 1, "abcdef", 2.3, std::string_view("hello", sizeof("hello")-1));
static auto memory = std::array<char, 65536>{};

void audit_encoder_benchmark(benchmark::State& state)
{
    auto const encoder = [](auto&&... args) {
        return auditor::encode_record(memory.data(), std::forward<decltype(args)>(args)...);
    };

    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(std::apply(encoder, rec));
    }

    //state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) * input.size());
}

BENCHMARK(audit_encoder_benchmark);


void audit_size_calc_benchmark(benchmark::State& state)
{
    auto const size_calculator = [](auto&&... args) {
        return auditor::full_record_size(std::forward<decltype(args)>(args)...);
    };

    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(std::apply(size_calculator, rec));
    }
}

BENCHMARK(audit_size_calc_benchmark);

BENCHMARK_DEFINE_F(AuditBufferBenchmarkFixture, audit_buffer_benchmark)(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        uint32_t const required_size = std::apply([] (auto&&... args) {
            return auditor::full_record_size(std::forward<decltype(args)>(args)...);
        }, rec);

        std::apply([addr = logbuf.reserve(required_size)] (auto&&... args) {
            auditor::encode_record(addr.get(), std::forward<decltype(args)>(args)...);
        }, rec);
    }
}

BENCHMARK_REGISTER_F(AuditBufferBenchmarkFixture, audit_buffer_benchmark)->DenseThreadRange(1, 4);

} // local namespace
