/*
 * Copyright (C) 2001-2012 Jacek Sieka, arnetheduck on gmail point com
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

#include "CriticalSection.h"
#include "SettingsManager.h"
#include "Singleton.h"
#include "Speaker.h"
#include "Util.h"

#include <string>
#include <unordered_map>

namespace dcpp {

using std::string;
using std::unordered_map;

class ConnectivityManagerListener {
public:
    virtual ~ConnectivityManagerListener() { }
    template<int I> struct X { enum { TYPE = I }; };

    typedef X<0> Message;
    typedef X<1> Finished;

    virtual void on(Message, const string&) noexcept { }
    virtual void on(Finished) noexcept { }
};

class ConnectivityManager : public Singleton<ConnectivityManager>, public Speaker<ConnectivityManagerListener>
{
public:
    void detectConnection();
    void setup(bool settingsChanged);
    bool isRunning() const { return running; }
    void updateLast();

private:
    friend class Singleton<ConnectivityManager>;
    friend class MappingManager;

    ConnectivityManager();
    virtual ~ConnectivityManager() { }

    void mappingFinished(bool success);
    void log(const string& msg);

    void startSocket();
    void listen();
    void disconnect();

    bool autoDetected;
    bool running;

    string lastBind;
};

} // namespace dcpp
