/**
 * @file        list_request_result.h
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

#ifndef LISTREQUESTRESULT_H
#define LISTREQUESTRESULT_H

#include <libKitsunemimiSakuraLang/blossom.h>

class ListRequestResult
        : public Kitsunemimi::Sakura::Blossom
{
public:
    ListRequestResult();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomIO &blossomIO,
                 const Kitsunemimi::DataMap &,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);
};

#endif // LISTREQUESTRESULT_H
