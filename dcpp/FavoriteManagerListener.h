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

#include <string>

#include "forward.h"

namespace dcpp {

using std::string;

class FavoriteManagerListener {
public:
    virtual ~FavoriteManagerListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> DownloadStarting;
    typedef X<1> DownloadFailed;
    typedef X<2> DownloadFinished;
    typedef X<3> FavoriteAdded;
    typedef X<4> FavoriteRemoved;
    typedef X<5> UserAdded;
    typedef X<6> UserRemoved;
    typedef X<7> StatusChanged;
    typedef X<8> LoadedFromCache;
    typedef X<9> Corrupted;

    virtual void on(DownloadStarting, const string&) noexcept { }
    virtual void on(DownloadFailed, const string&) noexcept { }
    virtual void on(DownloadFinished, const string&) noexcept { }
    virtual void on(FavoriteAdded, const FavoriteHubEntryPtr) noexcept { }
    virtual void on(FavoriteRemoved, const FavoriteHubEntryPtr) noexcept { }
    virtual void on(UserAdded, const FavoriteUser&) noexcept { }
    virtual void on(UserRemoved, const FavoriteUser&) noexcept { }
    virtual void on(StatusChanged, const FavoriteUser&) noexcept { } // NOTE: freedcpp
    virtual void on(LoadedFromCache, const string&, const string&) noexcept { }
    virtual void on(Corrupted, const string&) noexcept { }
};

} // namespace dcpp
