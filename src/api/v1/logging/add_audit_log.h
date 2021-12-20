#ifndef ADDAUDITLOG_H
#define ADDAUDITLOG_H

#include <libKitsunemimiSakuraLang/blossom.h>

class AddAuditLog
        : public Kitsunemimi::Sakura::Blossom
{
public:
    AddAuditLog();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 const Kitsunemimi::DataMap &context,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);
};

#endif // ADDAUDITLOG_H
