/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
 * Copyright (C) 2009-2019 EiskaltDC++ developers
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#if !defined(__APPLE__) && !defined(__MACH__)

#include <mutex>

namespace dcpp {

typedef std::recursive_mutex CriticalSection;
typedef std::mutex FastCriticalSection;
typedef std::unique_lock<std::recursive_mutex> Lock;
typedef std::lock_guard<std::mutex> FastLock;

} // namespace dcpp

#else // !defined(__APPLE__) && !defined(__MACH__)

#include <boost/signals2/mutex.hpp>

namespace dcpp {

class CriticalSection
{
public:
    CriticalSection() noexcept {
        pthread_mutexattr_init(&ma);
        pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&mtx, &ma);
    }
    ~CriticalSection() noexcept {
        pthread_mutex_destroy(&mtx);
        pthread_mutexattr_destroy(&ma);
    }
    void lock() noexcept { pthread_mutex_lock(&mtx); }
    void unlock() noexcept { pthread_mutex_unlock(&mtx); }
    pthread_mutex_t& getMutex() { return mtx; }
private:
    CriticalSection(const CriticalSection&);
    CriticalSection& operator=(const CriticalSection&);

    pthread_mutex_t mtx;
    pthread_mutexattr_t ma;
};

// A fast, non-recursive and unfair implementation of the Critical Section.
// It is meant to be used in situations where the risk for lock conflict is very low,
// i.e. locks that are held for a very short time. The lock is _not_ recursive, i e if
// the same thread will try to grab the lock it'll hang in a never-ending loop. The lock
// is not fair, i e the first to try to lock a locked lock is not guaranteed to be the
// first to get it when it's freed...

class FastCriticalSection {
public:
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
private:
    typedef boost::signals2::mutex mutex_t;
    mutex_t mtx;
};

template<class T>
class LockBase {
public:
    LockBase(T& aCs) noexcept : cs(aCs) { cs.lock(); }
    ~LockBase() noexcept { cs.unlock(); }
private:
    LockBase& operator=(const LockBase&);
    T& cs;
};

typedef LockBase<CriticalSection> Lock;
typedef LockBase<FastCriticalSection> FastLock;
}

#endif // !defined(__APPLE__) && !defined(__MACH__)

