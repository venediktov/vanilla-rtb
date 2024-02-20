/* 
 * File:   process.hpp
 * Author: Vladimir Venediktov
 * Copyright (c) 2016-2018 Venediktes Gruppe, LLC
 *
 * Created on February 19, 2016, 7:34 PM
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __OS_UNIX_PROCESS_HPP__
#define	__OS_UNIX_PROCESS_HPP__

#include <type_traits>
#include <stdexcept>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <boost/scoped_array.hpp>
#include <boost/lexical_cast.hpp>
#ifdef DEBUG
#include <iostream> 
#endif

namespace OS { namespace UNIX {

const std::string FAILED_CREATE_PIPE = "failed to create pipe pid=" ;
const std::string FAILED_FORK = "failed to fork pid=" ;
const std::string FAILED_EXEC_CHILD = "failed executing child process #" ;
const std::string FAILED_INSERT_CHILD = "failed to insert child pid=" ;
const std::string FAILED_WAIT = "wait failed , status=" ; 

struct null_proc_handler{};

template <typename>
struct Process;

template<typename Handler = null_proc_handler>
struct Process {

    using  pipe_list_t = std::vector<int> ;
    using  proc_list_t = std::set<pid_t> ;
    
    template<typename T = Handler,
             typename = typename std::enable_if<std::is_same<T,null_proc_handler>::value>::type>
    Process() :  handle_(), sub_pipes_(), sub_procs_() {}
    
    template<typename T = Handler, 
             typename = typename std::enable_if<!std::is_same<T,null_proc_handler>::value>::type> 
    Process(const T &handle) :  handle_(handle), sub_pipes_(), sub_procs_()  {}
    
    virtual ~Process() { exit(0); }
    
    template<typename ...Args,
             typename T = Handler,
             typename = typename std::enable_if<!std::is_same<T,null_proc_handler>::value>::type>
    auto operator() (Args && ...args ) const {
        return handle_(std::forward<Args>(args)...) ;
    }
    
    template<typename Child,
             typename T = Handler,
             typename = typename std::enable_if<std::is_same<T,null_proc_handler>::value>::type>
    std::pair<proc_list_t, pipe_list_t>
    spawn_exec(const std::vector<Child> &procs , const std::vector<std::string> &args) {
        //initialize vector of '\0' see vector ctor uses T() e.g char() is 0
        std::vector<char> params(1024) ;
        boost::scoped_array<char *> argv ( new char *[args.size()+1] ) ;
        std::vector<std::string>::const_iterator itr = args.begin() ;
        std::vector<char>::iterator pitr = params.begin() ;
        for ( int i = 0; itr != args.end() ; ++itr , ++pitr) {
            argv[i++] = &*pitr ;
            pitr = std::copy(itr->begin(), itr->end(), pitr) ;
        }
        argv[args.size()] = nullptr ;
        std::size_t i {0};
        for ( auto const &child : procs ){
            int fd[2] ;
            if ( pipe(fd) < 0) {
                std::runtime_error(FAILED_CREATE_PIPE + boost::lexical_cast<std::string>(getpid()) );
            }
            pid_t pid;
            if ((pid = fork()) < 0) {
                std::runtime_error(FAILED_FORK + boost::lexical_cast<std::string>(getpid()) );
            } else if (pid == 0) {
                close(fd[1]) ; //child close write
                ::dup2(fd[0], STDIN_FILENO) ;
                if (execvp(argv[0], argv.get() ) < 0) {
                    std::runtime_error ( FAILED_EXEC_CHILD + 
                                         boost::lexical_cast<std::string>(i) +
                                         ", pid=" + boost::lexical_cast<std::string>(getpid()) +  
                                         ",ppid=" + boost::lexical_cast<std::string>(getppid()) 
                    );
                    exit(-1);
                }
            }
#ifdef DEBUG
            std::clog << std::string("parent spawned child pid=") + 
                         boost::lexical_cast<std::string>(pid) +  " #:"  +
                         boost::lexical_cast<std::string>(++i) + "\n";
#endif
            if ( !sub_procs_.insert(pid).second  ) {
                close(fd[0]); //close read from child
                close(fd[1]); //close write to child
                std::runtime_error(FAILED_INSERT_CHILD  + 
                   boost::lexical_cast<std::string>(pid) + 
                   ", #" + boost::lexical_cast<std::string>(i) ) ;
            }
            close(fd[0]) ; //parent close read
            sub_pipes_.push_back(fd[1]) ;
        }
        return std::make_pair(sub_procs_, sub_pipes_) ;
    }
    
    template<typename Child,
             typename ...Args,
             typename T = Handler,
             typename = typename std::enable_if<std::is_same<T,null_proc_handler>::value>::type>
    const  proc_list_t & 
    spawn(const std::vector<Child> &procs, Args && ...args) {
        std::size_t i {0};
        for ( auto const &child : procs) {
            pid_t pid;
            if ((pid = fork()) < 0) {
                std::runtime_error(FAILED_FORK +  boost::lexical_cast<std::string>(getpid()) );
            } else if (pid == 0) { //child
                child(std::forward<Args>(args)...) ;
                exit(0);
            }
#ifdef DEBUG
            std::clog << std::string("parent spawned child pid=") + 
                         boost::lexical_cast<std::string>(pid) +  " #:"  +
                         boost::lexical_cast<std::string>(++i) + "\n";
#endif
            if ( !sub_procs_.insert(pid).second  ) {
                std::runtime_error(FAILED_INSERT_CHILD  + 
                                   boost::lexical_cast<std::string>(pid) + 
                                   ", #" + boost::lexical_cast<std::string>(i) ) ; 
            }
        }
        return sub_procs_ ;
    }
    

    template<typename T = Handler,
             typename = typename std::enable_if<std::is_same<T,null_proc_handler>::value>::type>
    void wait(proc_list_t &sub_procs) {
        proc_list_t intersection;
        std::set_intersection(sub_procs_.begin(), sub_procs_.end(),
                          sub_procs.begin(), sub_procs.end(),
                          std::inserter(intersection, intersection.begin()));
        while (!intersection.empty()) {
            int stat;
            int wpid = ::wait(&stat);
            if (wpid < 0) {
                std::runtime_error(FAILED_WAIT  +  boost::lexical_cast<std::string>(stat) );
            } else if ( intersection.erase(wpid) ) {
                sub_procs_.erase(wpid) ;
            }
        }
    }
   
    template <typename T = Handler,
              typename = typename std::enable_if<std::is_same<T,null_proc_handler>::value>::type,
              template <class,class> class C, class A >
    void distribute(const  pipe_list_t &sub_pipes, const C<std::string,A> &values) {
        int max_size = sub_pipes.size();
        typename C<std::string,A>::const_iterator itr = values.begin();
        for (int n = 0; itr != values.end(); ++itr) {
            write(sub_pipes.at(n++), itr->c_str(), itr->size());
            if (n >= max_size)
                n = 0;
        }
        // close output e.g. signalling end of transmission
        for(int fd : sub_pipes) {
            close(fd);
        }
    }
    
    
private:
    Handler handle_ ;
    pipe_list_t sub_pipes_ ;
    proc_list_t sub_procs_ ;
    
};

}} //namespaces

#endif	/* PROCESS_HPP */

