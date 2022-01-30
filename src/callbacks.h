/**
 * @file        callbacks.h
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

#ifndef SAGIRIARCHIVE_CALLBACKS_H
#define SAGIRIARCHIVE_CALLBACKS_H

#include <libKitsunemimiCommon/logger.h>

#include <libKitsunemimiSakuraNetwork/session.h>

#include <core/temp_file_handler.h>
#include <sagiri_root.h>

void streamDataCallback(void*,
                        Kitsunemimi::Sakura::Session*,
                        const void* data,
                        const uint64_t dataSize)
{
    const uint8_t* ptr = static_cast<const uint8_t*>(data);

    const std::string id(reinterpret_cast<const char*>(ptr), 36);
    ptr += 36;

    const uint64_t pos = *reinterpret_cast<const uint64_t*>(ptr);
    ptr += 8;

    SagiriRoot::tempFileHandler->addDataToPos(id, pos, ptr, dataSize - 44);
}


void dataRequestCallback(Kitsunemimi::Sakura::Session* session,
                         const void* data,
                         const uint64_t dataSize,
                         const uint64_t blockerId)
{
    const char* cData = static_cast<const char*>(data);
    const std::string location(cData, dataSize);

    Kitsunemimi::BinaryFile targetFile(location, false);
    Kitsunemimi::DataBuffer buffer;
    if(targetFile.readCompleteFile(buffer))
    {
        Kitsunemimi::ErrorContainer error;
        session->sendResponse(buffer.data, buffer.usedBufferSize, blockerId, error);
    }
    targetFile.closeFile();
}

#endif // SAGIRIARCHIVE_CALLBACKS_H
