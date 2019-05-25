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

#include "stdinc.h"
#include "DirectoryListing.h"

#include "BZUtils.h"
#include "ClientManager.h"
#include "CryptoManager.h"
#include "File.h"
#include "FilteredFile.h"
#include "QueueManager.h"
#include "ShareManager.h"
#include "SimpleXML.h"
#include "SimpleXMLReader.h"
#include "StringTokenizer.h"
#include "version.h"

namespace dcpp {

DirectoryListing::DirectoryListing(const HintedUser& aUser) :
    user(aUser),
    root(new Directory(NULL, Util::emptyString, false, false))
{
}

DirectoryListing::~DirectoryListing() {
    delete root;
}

UserPtr DirectoryListing::getUserFromFilename(const string& fileName) {
    // General file list name format: [username].[CID].[xml|xml.bz2]

    string name = Util::getFileName(fileName);

    // Strip off any extensions
    if(Util::stricmp(name.c_str() + name.length() - 4, ".bz2") == 0) {
        name.erase(name.length() - 4);
    }

    if(Util::stricmp(name.c_str() + name.length() - 4, ".xml") == 0) {
        name.erase(name.length() - 4);
    }

    // Find CID
    string::size_type i = name.rfind('.');
    if(i == string::npos) {
        return UserPtr();
    }

    size_t n = name.length() - (i + 1);
    // CID's always 39 chars long...
    if(n != 39)
        return UserPtr();

    CID cid(name.substr(i + 1));
    if(!cid)
        return UserPtr();

    return ClientManager::getInstance()->getUser(cid);
}

void DirectoryListing::loadFile(const string& path) {
    string actualPath;
    if(dcpp::File::getSize(path + ".bz2") != -1) {
        actualPath = path + ".bz2";
    }

    // For now, we detect type by ending...
    auto ext = Util::getFileExt(actualPath.empty() ? path : actualPath);

    {
        dcpp::File file(actualPath.empty() ? path : actualPath, dcpp::File::READ, dcpp::File::OPEN);
        if(Util::stricmp(ext, ".bz2") == 0) {
            FilteredInputStream<UnBZFilter, false> f(&file);
            loadXML(f, false);
        } else if(Util::stricmp(ext, ".xml") == 0) {
            loadXML(file, false);
        } else {
            throw Exception(_("Invalid file list extension (must be .xml or .bz2)"));
        }
    }
}

class ListLoader : public SimpleXMLReader::CallBack {
public:
    ListLoader(DirectoryListing::Directory* root, bool aUpdating) :
        cur(root),
        base("/"),
        inListing(false),
        updating(aUpdating),
        m_is_mediainfo_list(false),
        m_is_first_check_mediainfo_list(false)
    {
    }

    virtual ~ListLoader() {
    }

    void startTag(const string& name, StringPairList& attribs, bool simple);
    void endTag(const string& name);

    const string& getBase() const { return base; }

private:
    DirectoryListing::Directory* cur;

    StringMap params;
    string base;
    bool inListing;
    bool updating;
    bool m_is_mediainfo_list;
    bool m_is_first_check_mediainfo_list;
};

string DirectoryListing::updateXML(const string& xml) {
    MemoryInputStream mis(xml);
    return loadXML(mis, true);
}

string DirectoryListing::loadXML(InputStream& is, bool updating) {
    ListLoader ll(getRoot(), updating);

    SimpleXMLReader(&ll).parse(is, SETTING(MAX_FILELIST_SIZE) ? (size_t)SETTING(MAX_FILELIST_SIZE)*1024*1024 : 0);

    return ll.getBase();
}

static const string sFileListing = "FileListing";
static const string sBase = "Base";
static const string sDirectory = "Directory";
static const string sIncomplete = "Incomplete";
static const string sFile = "File";
static const string sName = "Name";
static const string sSize = "Size";
static const string sTTH = "TTH";
static const string sBR = "BR";
static const string sWH = "WH";
static const string sMVideo = "MV";
static const string sMAudio = "MA";
static const string sTS = "TS";
static const string sHIT = "HIT";

void ListLoader::startTag(const string& name, StringPairList& attribs, bool simple) {
    if(inListing) {
        if(name == sFile) {
            const string& n = getAttrib(attribs, sName, 0);
            if(n.empty())
                return;
            const string& s = getAttrib(attribs, sSize, 1);
            if(s.empty())
                return;
            auto size = Util::toInt64(s);
            const string& h = getAttrib(attribs, sTTH, 2);
            if(h.empty())
                return;
            TTHValue tth(h); /// @todo verify validity?

            if(updating) {
                // just update the current file if it is already there.
                for(auto& i : cur->files) {
                    auto& file = *i;
                    /// @todo comparisons should be case-insensitive but it takes too long - add a cache
                    if(file.getTTH() == tth || file.getName() == n) {
                        file.setName(n);
                        file.setSize(size);
                        file.setTTH(tth);
                        return;
                    }
                }
            }

            DirectoryListing::File* f = new DirectoryListing::File(cur, n, size, tth);

            string l_ts = "";

            if (!m_is_first_check_mediainfo_list){
                m_is_first_check_mediainfo_list = true;
                l_ts = getAttrib(attribs, sTS, 3);
                m_is_mediainfo_list = !l_ts.empty();
            }
            else if (m_is_mediainfo_list) {
                l_ts = getAttrib(attribs, sTS, 3);
            }

            if (!l_ts.empty()){
                f->setTS(atol(l_ts.c_str()));
                f->setHit(atol(getAttrib(attribs, sHIT, 3).c_str()));
                f->mediaInfo.video_info = getAttrib(attribs, sMVideo, 3);
                f->mediaInfo.audio_info = getAttrib(attribs, sMAudio, 3);
                f->mediaInfo.resolution = getAttrib(attribs, sWH, 3);
                f->mediaInfo.bitrate    = atoi(getAttrib(attribs, sBR, 4).c_str());
            }

            cur->files.insert(f);
        } else if(name == sDirectory) {
            const string& n = getAttrib(attribs, sName, 0);
            if(n.empty()) {
                throw SimpleXMLException(_("Directory missing name attribute"));
            }
            bool incomp = getAttrib(attribs, sIncomplete, 1) == "1";
            DirectoryListing::Directory* d = NULL;
            if(updating) {
                for(auto& i : cur->directories) {
                    /// @todo comparisons should be case-insensitive but it takes too long - add a cache
                    if(i->getName() == n) {
                        d = i;
                        if(!d->getComplete())
                            d->setComplete(!incomp);
                        break;
                    }
                }
            }
            if(d == NULL) {
                d = new DirectoryListing::Directory(cur, n, false, !incomp);
                cur->directories.insert(d);
            }
            cur = d;

            if(simple) {
                // To handle <Directory Name="..." />
                endTag(name);
            }
        }
    } else if(name == sFileListing) {
        const string& b = getAttrib(attribs, sBase, 2);
        if(!b.empty() && b[0] == '/' && b[b.size()-1] == '/') {
            base = b;
        }
        StringList sl = StringTokenizer<string>(base.substr(1), '/').getTokens();
        for(auto& i: sl) {
            DirectoryListing::Directory* d = nullptr;
            for(auto j: cur->directories) {
                if(j->getName() == i) {
                    d = j;
                    break;
                }
            }
            if(!d) {
                d = new DirectoryListing::Directory(cur, i, false, false);
                cur->directories.insert(d);
            }
            cur = d;
        }
        cur->setComplete(true);
        inListing = true;

        if(simple) {
            // To handle <Directory Name="..." />
            endTag(name);
        }
    }
}

void ListLoader::endTag(const string& name) {
    if(inListing) {
        if(name == sDirectory) {
            cur = cur->getParent();
        } else if(name == sFileListing) {
            // cur should be root now...
            inListing = false;
        }
    }
}

string DirectoryListing::getPath(const Directory* d) const {
    if(d == root)
        return "";

    string dir;
    dir.reserve(128);
    dir.append(d->getName());
    dir.append(1, '\\');

    Directory* cur = d->getParent();
    while(cur!=root) {
        dir.insert(0, cur->getName() + '\\');
        cur = cur->getParent();
    }
    return dir;
}

StringList DirectoryListing::getLocalPaths(const File* f) const {
    try {
        return ShareManager::getInstance()->getRealPaths(Util::toAdcFile(getPath(f) + f->getName()));
    } catch(const ShareException&) {
        return StringList();
    }
}

StringList DirectoryListing::getLocalPaths(const Directory* d) const {
    try {
        return ShareManager::getInstance()->getRealPaths(Util::toAdcFile(getPath(d)));
    } catch(const ShareException&) {
        return StringList();
    }
}

void DirectoryListing::download(Directory* aDir, const string& aTarget, bool highPrio) {
    string target = (aDir == getRoot()) ? aTarget : aTarget + aDir->getName() + PATH_SEPARATOR;
    // First, recurse over the directories
    for(auto& j: aDir->directories) {
        download(j, target, highPrio);
    }
    // Then add the files
    for(auto file: aDir->files) {
        try {
            download(file, target + file->getName(), false, highPrio);
        } catch(const QueueException&) {
            // Catch it here to allow parts of directories to be added...
        } catch(const FileException&) {
            //..
        }
    }
}

void DirectoryListing::download(const string& aDir, const string& aTarget, bool highPrio) {
    dcassert(aDir.size() > 2);
    dcassert(aDir[aDir.size() - 1] == '\\'); // This should not be PATH_SEPARATOR
    Directory* d = find(aDir, getRoot());
    if(d)
        download(d, aTarget, highPrio);
}

void DirectoryListing::download(File* aFile, const string& aTarget, bool view, bool highPrio) {
    int flags = (view ? (QueueItem::FLAG_TEXT | QueueItem::FLAG_CLIENT_VIEW) : 0);

    QueueManager::getInstance()->add(aTarget, aFile->getSize(), aFile->getTTH(), getUser(), flags);

    if(highPrio)
        QueueManager::getInstance()->setPriority(aTarget, QueueItem::HIGHEST);
}

DirectoryListing::Directory* DirectoryListing::find(const string& aName, Directory* current) const {
    auto end = aName.find('\\');
    dcassert(end != string::npos);
    auto name = aName.substr(0, end);

    auto i = std::find(current->directories.cbegin(), current->directories.cend(), name);
    if(i != current->directories.cend()) {
        if(end == (aName.size() - 1))
            return *i;
        else
            return find(aName.substr(end + 1), *i);
    }
    return nullptr;
}

DirectoryListing::Directory::~Directory() {
    std::for_each(directories.begin(), directories.end(), DeleteFunction());
    std::for_each(files.begin(), files.end(), DeleteFunction());
}

void DirectoryListing::Directory::filterList(DirectoryListing& dirList) {
    DirectoryListing::Directory* d = dirList.getRoot();

    TTHSet l;
    d->getHashList(l);
    filterList(l);
}

void DirectoryListing::Directory::filterList(DirectoryListing::Directory::TTHSet& l) {
    for(auto i = directories.begin(); i != directories.end();) {
        auto d = *i;

        d->filterList(l);

        if(d->directories.empty() && d->files.empty()) {
            i = directories.erase(i);
            delete d;
        } else {
            ++i;
        }
    }

    for(auto i = files.begin(); i != files.end();) {
        auto f = *i;
        if(l.find(f->getTTH()) != l.end()) {
            i = files.erase(i);
            delete f;
        } else {
            ++i;
        }
    }
}

void DirectoryListing::Directory::getHashList(DirectoryListing::Directory::TTHSet& l) {
    for(auto i: directories) i->getHashList(l);
    for(auto i: files) l.insert(i->getTTH());
}

int64_t DirectoryListing::Directory::getTotalSize(bool adl) {
    int64_t x = getSize();
    for(auto i: directories) {
        if(!(adl && i->getAdls()))
            x += i->getTotalSize(adls);
    }
    return x;
}

size_t DirectoryListing::Directory::getTotalFileCount(bool adl) {
    size_t x = getFileCount();
    for(auto i: directories) {
        if(!(adl && i->getAdls()))
            x += i->getTotalFileCount(adls);
    }
    return x;
}
} // namespace dcpp
