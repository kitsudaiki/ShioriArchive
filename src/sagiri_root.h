#ifndef SAGIRIROOT_H
#define SAGIRIROOT_H

namespace Kitsunemimi {
namespace Sakura {
class SqlDatabase;
}
}
class TrainDataTable;

class SagiriRoot
{
public:
    SagiriRoot();

    bool init();

    static TrainDataTable* trainDataTable;
    static Kitsunemimi::Sakura::SqlDatabase* database;
};

#endif // SAGIRIROOT_H
