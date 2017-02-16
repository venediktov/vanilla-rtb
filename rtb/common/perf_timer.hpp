/* 
 * File:   perf_timer.hpp
 * Author: arseny.bushev@gmail.com
 *
 * Created on 15 февраля 2017 г., 11:20
 */

#ifndef PERF_TIMER_HPP
#define PERF_TIMER_HPP

template<typename Stream>
struct perf_timer {
    perf_timer(std::shared_ptr<Stream> &osp) : begin{std::chrono::steady_clock::now()}, end{begin}, osp{osp} {}
    ~perf_timer() {
      end = std::chrono::steady_clock::now() ;
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
  decltype(std::chrono::steady_clock::now()) begin;
  decltype(std::chrono::steady_clock::now()) end;
  std::shared_ptr<Stream> osp;
};


#endif /* PERF_TIMER_HPP */

