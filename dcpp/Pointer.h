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

#include <boost/intrusive_ptr.hpp>

#include <memory>

#include <atomic>

namespace dcpp {

using std::unique_ptr;
using std::forward;

template<typename T>
class intrusive_ptr_base
{
public:
    void inc() noexcept {
        intrusive_ptr_add_ref(this);
    }

    void dec() noexcept {
        intrusive_ptr_release(this);
    }

    bool unique(int val = 1) const noexcept {
        return (ref <= val);
    }

protected:
    intrusive_ptr_base() noexcept : ref(0) { }

private:
    friend void intrusive_ptr_add_ref(intrusive_ptr_base* p) { ++p->ref; }
    friend void intrusive_ptr_release(intrusive_ptr_base* p) { if(--p->ref == 0) { delete static_cast<T*>(p); } }

    std::atomic<long> ref;
};

struct DeleteFunction {
    template<typename T>
    void operator()(const T& p) const { delete p; }
};

} // namespace dcpp

