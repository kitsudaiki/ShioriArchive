/**
 * @file        temp_file_handler.cpp
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

#include "temp_file_handler.h"

#include <libKitsunemimiCommon/methods/file_methods.h>
#include <libKitsunemimiCommon/files/binary_file.h>
#include <libKitsunemimiConfig/config_handler.h>

/**
 * @brief constructor
 */
TempFileHandler::TempFileHandler() {}

/**
 * @brief destructor and delete all registered temporary files
 */
TempFileHandler::~TempFileHandler()
{
    bool success = false;
    Kitsunemimi::ErrorContainer error;
    const std::string targetFilePath = GET_STRING_CONFIG("sagiri", "data_set_location", success);

    std::vector<std::string> result;
    std::map<std::string, Kitsunemimi::BinaryFile*>::iterator it;
    for(it = m_tempFiles.begin();
        it != m_tempFiles.end();
        it++)
    {
        Kitsunemimi::BinaryFile* ptr = it->second;
        if(ptr->closeFile(error) == false) {
            //TODO: handle error
        }
        delete ptr;
        Kitsunemimi::deleteFileOrDir(targetFilePath + "/" + it->first, error);
        m_tempFiles.erase(it);
    }
}

/**
 * @brief initialize new temporary file
 *
 * @param id id of the new temporary file
 * @param size size to allocate
 *
 * @return false, if id already exist or storage-allocation failed, else true
 */
bool
TempFileHandler::initNewFile(const std::string &id, const uint64_t size)
{
    if(m_tempFiles.find(id) != m_tempFiles.end()) {
        return false;
    }

    bool success = false;
    std::string targetFilePath = GET_STRING_CONFIG("sagiri", "data_set_location", success);
    targetFilePath += "/" + id;

    Kitsunemimi::ErrorContainer error;
    Kitsunemimi::BinaryFile* tempfile = new Kitsunemimi::BinaryFile(targetFilePath);
    if(tempfile->allocateStorage(size, error) == false)
    {
        LOG_ERROR(error);
        return false;
    }
    m_tempFiles.insert(std::make_pair(id, tempfile));

    return true;
}

/**
 * @brief add data to the temporary file
 *
 * @param uuid uuid of the temporary file
 * @param pos position in the file where to add the data
 * @param data pointer to the data to add
 * @param size size of the data to add
 *
 * @return false, if id not found, else true
 */
bool
TempFileHandler::addDataToPos(const std::string &uuid,
                              const uint64_t pos,
                              const void* data,
                              const uint64_t size)
{
    Kitsunemimi::ErrorContainer error;

    std::map<std::string, Kitsunemimi::BinaryFile*>::const_iterator it;
    it = m_tempFiles.find(uuid);
    if(it != m_tempFiles.end())
    {
        Kitsunemimi::BinaryFile* ptr = it->second;
        if(ptr->writeDataIntoFile(data, pos, size, error) == false)
        {
            LOG_ERROR(error);
            return false;
        }

        return true;
    }

    return false;
}

/**
 * @brief get data from the temporary file
 *
 * @param result data-buffer for the resulting data of the file
 * @param uuid uuid of the temporary file
 *
 * @return false, if id not found, else true
 */
bool
TempFileHandler::getData(Kitsunemimi::DataBuffer &result, const std::string &uuid)
{
    Kitsunemimi::ErrorContainer error;

    std::map<std::string, Kitsunemimi::BinaryFile*>::const_iterator it;
    it = m_tempFiles.find(uuid);
    if(it != m_tempFiles.end())
    {
        Kitsunemimi::BinaryFile* ptr = it->second;
        return ptr->readCompleteFile(result, error);
    }

    return false;
}

/**
 * @brief remove an id from this class and delete the file within the storage
 *
 * @param id id of the temporary file
 *
 * @return false, if id not found, else true
 */
bool
TempFileHandler::removeData(const std::string &id)
{
    bool success = false;
    Kitsunemimi::ErrorContainer error;
    std::string targetFilePath = GET_STRING_CONFIG("sagiri", "data_set_location", success);

    std::map<std::string, Kitsunemimi::BinaryFile*>::const_iterator it;
    it = m_tempFiles.find(id);
    if(it != m_tempFiles.end())
    {
        Kitsunemimi::BinaryFile* ptr = it->second;
        if(ptr->closeFile(error) == false) {
            //TODO: handle error
        }
        delete ptr;
        Kitsunemimi::deleteFileOrDir(targetFilePath + "/" + it->first, error);
        m_tempFiles.erase(it);

        return true;
    }

    return false;
}

/**
 * @brief move tempfile to its target-location after tempfile was finished
 *
 * @param uuid uuid of the tempfile, which should be moved
 * @param targetLocation target-location on the local direct, where to move the file to
 * @param error reference for error-output
 *
 * @return true, if successful, else false
 */
bool
TempFileHandler::moveData(const std::string &uuid,
                          const std::string &targetLocation,
                          Kitsunemimi::ErrorContainer &error)
{
    bool success = false;
    std::string targetFilePath = GET_STRING_CONFIG("sagiri", "data_set_location", success);

    std::map<std::string, Kitsunemimi::BinaryFile*>::const_iterator it;
    it = m_tempFiles.find(uuid);
    if(it != m_tempFiles.end())
    {
        Kitsunemimi::BinaryFile* ptr = it->second;
        if(ptr->closeFile(error) == false)
        {
            LOG_ERROR(error);
            return false;
        }

        if(Kitsunemimi::renameFileOrDir(targetFilePath + "/" + it->first,
                                        targetLocation,
                                        error) == false)
        {
            error.addMeesage("Failed to move temp-file with uuid '"
                             + uuid
                             + "' to target-locateion '"
                             + targetLocation
                             + "'");
            LOG_ERROR(error);
            return false;
        }

        delete ptr;
        m_tempFiles.erase(it);

        return true;
    }

    error.addMeesage("Failed to move temp-file with uuid '"
                     + uuid
                     + ", because it can not be found.");
    LOG_ERROR(error);

    return false;
}
