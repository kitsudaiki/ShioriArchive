#ifndef SAGIRIARCHIVE_CREATETRAINDATA_H
#define SAGIRIARCHIVE_CREATETRAINDATA_H

#include <libKitsunemimiSakuraLang/blossom.h>

class CreateTrainData
        : public Kitsunemimi::Sakura::Blossom
{
public:
    CreateTrainData();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 const Kitsunemimi::DataMap &context,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);
};

#endif // SAGIRIARCHIVE_CREATETRAINDATA_H
