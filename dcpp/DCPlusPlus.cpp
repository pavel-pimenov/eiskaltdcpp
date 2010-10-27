/*
 * Copyright (C) 2001-2010 Jacek Sieka, arnetheduck on gmail point com
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "stdinc.h"
#include "DCPlusPlus.h"

#include "ConnectionManager.h"
#include "DownloadManager.h"
#include "UploadManager.h"
#include "CryptoManager.h"
#include "ShareManager.h"
#include "SearchManager.h"
#include "QueueManager.h"
#include "ClientManager.h"
#include "HashManager.h"
#include "LogManager.h"
#include "FavoriteManager.h"
#include "SettingsManager.h"
#include "FinishedManager.h"
#include "ResourceManager.h"
#include "ThrottleManager.h"
#include "ADLSearch.h"
#include "WindowManager.h"
#include "StringTokenizer.h"
#ifdef LUA_SCRIPT
#include "ScriptManager.h"
#endif
#include "UPnPManager.h"
#include "ConnectivityManager.h"
#ifdef STDIPFILTER
#include "extra/ipfilter.h"
#endif
#ifdef DHT
#include "dht/DHT.h"
#endif
#include "DebugManager.h"

#ifdef _STLP_DEBUG
void __stl_debug_terminate() {
    int* x = 0;
    *x = 0;
}
#endif

extern "C" int _nl_msg_cat_cntr;

namespace dcpp {

void startup(void (*f)(void*, const string&), void* p) {
    // "Dedicated to the near-memory of Nev. Let's start remembering people while they're still alive."
    // Nev's great contribution to dc++
    while(1) break;


#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    Util::initialize();

    bindtextdomain(PACKAGE, LOCALEDIR);

    ResourceManager::newInstance();
    SettingsManager::newInstance();

    LogManager::newInstance();
    TimerManager::newInstance();
    HashManager::newInstance();
    CryptoManager::newInstance();
    SearchManager::newInstance();
    ClientManager::newInstance();
    ConnectionManager::newInstance();
    DownloadManager::newInstance();
    UploadManager::newInstance();
    ThrottleManager::newInstance();
    QueueManager::newInstance();
    ShareManager::newInstance();
    FavoriteManager::newInstance();
    FinishedManager::newInstance();
    ADLSearchManager::newInstance();
    ConnectivityManager::newInstance();
    UPnPManager::newInstance();
#ifdef STDIPFILTER
    IPFilter::newInstance();
#endif
    WindowManager::newInstance();
#ifdef LUA_SCRIPT
    ScriptManager::newInstance();
#endif
    DebugManager::newInstance();

    SettingsManager::getInstance()->load();
#ifdef STDIPFILTER
    IPFilter::getInstance()->loadList();
#endif
    if(!SETTING(LANGUAGE).empty()) {
#ifdef _WIN32
        string language = "LANGUAGE=" + SETTING(LANGUAGE);
        putenv(language.c_str());
#else
        setenv("LANGUAGE", SETTING(LANGUAGE).c_str(), true);
#endif
        // Apparently this is supposted to make gettext reload the message catalog...
        _nl_msg_cat_cntr++;
    }

    FavoriteManager::getInstance()->load();
    CryptoManager::getInstance()->loadCertificates();
#ifdef USE_DHT
    DHT::newInstance();
#endif
    if(f != NULL)
        (*f)(p, _("Hash database"));
    HashManager::getInstance()->startup();
    if(f != NULL)
        (*f)(p, _("Shared Files"));
    ShareManager::getInstance()->refresh(true, false, true);
    if(f != NULL)
        (*f)(p, _("Download Queue"));
    QueueManager::getInstance()->loadQueue();
    if(f != NULL)
        (*f)(p, _("Users"));
    ClientManager::getInstance()->loadUsers();
}

void shutdown() {

#ifndef _WIN32 //*nix system
    ThrottleManager::getInstance()->shutdown();
#endif
    DebugManager::deleteInstance();
#ifdef LUA_SCRIPT
    ScriptManager::deleteInstance();
#endif
    TimerManager::getInstance()->shutdown();
    HashManager::getInstance()->shutdown();

#ifdef _WIN32
    ThrottleManager::getInstance()->shutdown();
#endif

    ConnectionManager::getInstance()->shutdown();
    UPnPManager::getInstance()->close();

    BufferedSocket::waitShutdown();
    WindowManager::getInstance()->prepareSave();
    QueueManager::getInstance()->saveQueue(true);
    ClientManager::getInstance()->saveUsers();
#ifdef STDIPFILTER
    IPFilter::getInstance()->saveList();
#endif
    SettingsManager::getInstance()->save();

#ifdef USE_DHT
    DHT::deleteInstance();
#endif
    WindowManager::deleteInstance();
#ifdef STDIPFILTER
    IPFilter::deleteInstance();
#endif
    UPnPManager::deleteInstance();
    ConnectivityManager::deleteInstance();
    ADLSearchManager::deleteInstance();
    FinishedManager::deleteInstance();
    ShareManager::deleteInstance();
    CryptoManager::deleteInstance();
    ThrottleManager::deleteInstance();
    DownloadManager::deleteInstance();
    UploadManager::deleteInstance();
    QueueManager::deleteInstance();
    ConnectionManager::deleteInstance();
    SearchManager::deleteInstance();
    FavoriteManager::deleteInstance();
    ClientManager::deleteInstance();
    HashManager::deleteInstance();
    LogManager::deleteInstance();
    SettingsManager::deleteInstance();
    TimerManager::deleteInstance();
    ResourceManager::deleteInstance();

#ifdef _WIN32
    ::WSACleanup();
#endif
}

} // namespace dcpp
