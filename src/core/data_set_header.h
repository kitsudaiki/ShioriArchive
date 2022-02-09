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

#ifndef SAGIRIARCHIVE_DATA_SET_HEADER_H
#define SAGIRIARCHIVE_DATA_SET_HEADER_H

#include <stdint.h>
#include <string>
#include <cstring>

enum DataSetType
{
    UNDEFINED_TYPE = 0,
    IMAGE_TYPE = 1,
    TABLE_TYPE = 2
};

struct DataSetHeader
{
    uint8_t type = UNDEFINED_TYPE;
    char name[256];
};

struct ImageTypeHeader
{
    uint64_t numberOfInputsX = 0;
    uint64_t numberOfInputsY = 0;
    uint64_t numberOfOutputs = 0;
    uint64_t numberOfImages = 0;
    float maxValue = 0.0f;
    float avgValue = 0.0f;
};

struct TableTypeHeader
{
    uint64_t numberOfColumns = 0;
    uint64_t numberOfLines = 0;
};

struct TableHeaderEntry
{
    char name[256];
    bool isInput = false;
    bool isOutput = false;
    float multiplicator = 1.0f;
    float averageVal = 0.0f;
    float maxVal = 0.0f;

    void setName(const std::string &name)
    {
        uint32_t nameSize = name.size();
        if(nameSize > 255) {
            nameSize = 255;
        }
        memcpy(this->name, name.c_str(), nameSize);
        this->name[nameSize] = '\0';
    }
};

#endif // SAGIRIARCHIVE_DATA_SET_HEADER_H
