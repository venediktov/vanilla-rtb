/* 
 * File:   perf_timer.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 15 февраля 2017 г., 11:20
 */

#ifndef PERF_TIMER_HPP
#define PERF_TIMER_HPP

#include <chrono>
#include <memory>
#include <string>

template<typename Stream>
struct perf_timer {
    using perf_clock = std::chrono::high_resolution_clock;

    perf_timer(std::shared_ptr<Stream> &osp, std::string name = "") : 
        begin{perf_clock::now()}, end{begin}, osp{osp}, name{std::move(name)} 
    {}
    ~perf_timer() {
      end = perf_clock::now() ;
      if(name.length()) {
        *osp << name << " : ";
      }
      *osp << "elapsed_sec=" 
         << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count() 
         << "|";
      *osp << "elapsed_ms=" 
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() 
         << "|";
      *osp << "elapsed_mu=" 
         << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() 
         << "|";
      *osp << "elapsed_ns=" 
         << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() 
         << "|"; 
    }
private:
  perf_clock::time_point begin;
  perf_clock::time_point end;
  std::shared_ptr<Stream> osp;
  std::string name;
};


#endif /* PERF_TIMER_HPP */

