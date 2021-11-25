#ifndef ADDERRORLOG_H
#define ADDERRORLOG_H

#include <libKitsunemimiSakuraLang/blossom.h>

class AddErrorLog
        : public Kitsunemimi::Sakura::Blossom
{
public:
    AddErrorLog();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 const Kitsunemimi::DataMap &context,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);
};

#endif // ADDERRORLOG_H
