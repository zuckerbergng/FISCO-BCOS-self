/*
    @CopyRight:
    This file is part of FISCO-BCOS.

    FISCO-BCOS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FISCO-BCOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FISCO-BCOS.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @brief: current handle of easylog
 *
 * @file easylog.cpp
 * @author yujiechen
 * @date 2018-08-27
 */

#include "easylog.h"
#include <iostream>
#include <string>
#include <thread>
#ifdef __APPLE__
#include <pthread.h>
#endif
#if !defined(ETH_EMSCRIPTEN)
#include <boost/asio/ip/tcp.hpp>
#endif
#include "Guards.h"
using namespace std;
using namespace dev;

struct ThreadLocalLogName
{
    ThreadLocalLogName(std::string const& _name) { m_name.reset(new string(_name)); }
    boost::thread_specific_ptr<std::string> m_name;
};

/// Associate a name with each thread for nice logging.
struct ThreadLocalLogContext
{
    ThreadLocalLogContext() = default;

    void push(std::string const& _name)
    {
        if (!m_contexts.get())
            m_contexts.reset(new vector<string>);
        m_contexts->push_back(_name);
    }

    void pop() { m_contexts->pop_back(); }

    string join(string const& _prior)
    {
        string ret;
        if (m_contexts.get())
            for (auto const& i : *m_contexts)
                ret += _prior + i;
        return ret;
    }

    boost::thread_specific_ptr<std::vector<std::string>> m_contexts;
};

ThreadLocalLogContext g_logThreadContext;

ThreadLocalLogName g_logThreadName("main");

void dev::ThreadContext::push(string const& _n)
{
    g_logThreadContext.push(_n);
}

void dev::ThreadContext::pop()
{
    g_logThreadContext.pop();
}

string dev::ThreadContext::join(string const& _prior)
{
    return g_logThreadContext.join(_prior);
}

void dev::pthread_setThreadName(std::string const& _n)
{
#if defined(__GLIBC__)
    pthread_setname_np(pthread_self(), _n.c_str());
#elif defined(__APPLE__)
    pthread_setname_np(_n.c_str());
#else
    g_logThreadName.m_name.reset(new std::string(_n));
#endif
}

/// Set the current thread's log name.
std::string dev::getThreadName()
{
#if defined(__GLIBC__) || defined(__APPLE__)
    char buffer[128];
    pthread_getname_np(pthread_self(), buffer, 127);
    buffer[127] = 0;
    return buffer;
#else
    return g_logThreadName.m_name.get() ? *g_logThreadName.m_name.get() : "<unknown>";
#endif
}
