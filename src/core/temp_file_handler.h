/**
 * @file        temp_file_handler.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2021 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef SHIORIARCHIVE_TEMPFILEHANDLER_H
#define SHIORIARCHIVE_TEMPFILEHANDLER_H

#include <string>
#include <map>
#include <libKitsunemimiCommon/logger.h>

namespace Kitsunemimi {
class BinaryFile;
struct DataBuffer;
}

class TempFileHandler
{
public:
    TempFileHandler();
    ~TempFileHandler();

    bool initNewFile(const std::string &id,
                     const uint64_t size);
    bool addDataToPos(const std::string &uuid,
                      const uint64_t pos,
                      const void* data,
                      const uint64_t size);
    bool getData(Kitsunemimi::DataBuffer &result,
                 const std::string &uuid);
    bool removeData(const std::string &id);
    bool moveData(const std::string &uuid,
                  const std::string &targetLocation,
                  Kitsunemimi::ErrorContainer &error);

private:
    std::map<std::string, Kitsunemimi::BinaryFile*> m_tempFiles;
};

#endif // SHIORIARCHIVE_TEMPFILEHANDLER_H
