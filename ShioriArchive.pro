QT -= qt core gui

TARGET = ShioriArchive
CONFIG += console c++17
CONFIG -= app_bundle


LIBS += -L../libShioriArchive/src -lShioriArchive
LIBS += -L../libShioriArchive/src/debug -lShioriArchive
LIBS += -L../libShioriArchive/src/release -lShioriArchive
INCLUDEPATH += ../libShioriArchive/include

LIBS += -L../libAzukiHeart/src -lAzukiHeart
LIBS += -L../libAzukiHeart/src/debug -lAzukiHeart
LIBS += -L../libAzukiHeart/src/release -lAzukiHeart
INCLUDEPATH += ../libAzukiHeart/include

LIBS += -L../libMisakiGuard/src -lMisakiGuard
LIBS += -L../libMisakiGuard/src/debug -lMisakiGuard
LIBS += -L../libMisakiGuard/src/release -lMisakiGuard
INCLUDEPATH += ../libMisakiGuard/include

LIBS += -L../libKitsunemimiHanamiNetwork/src -lKitsunemimiHanamiNetwork
LIBS += -L../libKitsunemimiHanamiNetwork/src/debug -lKitsunemimiHanamiNetwork
LIBS += -L../libKitsunemimiHanamiNetwork/src/release -lKitsunemimiHanamiNetwork
INCLUDEPATH += ../libKitsunemimiHanamiNetwork/include

LIBS += -L../libKitsunemimiHanamiDatabase/src -lKitsunemimiHanamiDatabase
LIBS += -L../libKitsunemimiHanamiDatabase/src/debug -lKitsunemimiHanamiDatabase
LIBS += -L../libKitsunemimiHanamiDatabase/src/release -lKitsunemimiHanamiDatabase
INCLUDEPATH += ../libKitsunemimiHanamiDatabase/include

LIBS += -L../libKitsunemimiHanamiCommon/src -lKitsunemimiHanamiCommon
LIBS += -L../libKitsunemimiHanamiCommon/src/debug -lKitsunemimiHanamiCommon
LIBS += -L../libKitsunemimiHanamiCommon/src/release -lKitsunemimiHanamiCommon
INCLUDEPATH += ../libKitsunemimiHanamiCommon/include

LIBS += -L../libKitsunemimiSakuraDatabase/src -lKitsunemimiSakuraDatabase
LIBS += -L../libKitsunemimiSakuraDatabase/src/debug -lKitsunemimiSakuraDatabase
LIBS += -L../libKitsunemimiSakuraDatabase/src/release -lKitsunemimiSakuraDatabase
INCLUDEPATH += ../libKitsunemimiSakuraDatabase/include

LIBS += -L../libKitsunemimiArgs/src -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/debug -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/release -lKitsunemimiArgs
INCLUDEPATH += ../libKitsunemimiArgs/include

LIBS += -L../libKitsunemimiConfig/src -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/debug -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/release -lKitsunemimiConfig
INCLUDEPATH += ../libKitsunemimiConfig/include

LIBS += -L../libKitsunemimiSakuraNetwork/src -lKitsunemimiSakuraNetwork
LIBS += -L../libKitsunemimiSakuraNetwork/src/debug -lKitsunemimiSakuraNetwork
LIBS += -L../libKitsunemimiSakuraNetwork/src/release -lKitsunemimiSakuraNetwork
INCLUDEPATH += ../libKitsunemimiSakuraNetwork/include

LIBS += -L../libKitsunemimiSqlite/src -lKitsunemimiSqlite
LIBS += -L../libKitsunemimiSqlite/src/debug -lKitsunemimiSqlite
LIBS += -L../libKitsunemimiSqlite/src/release -lKitsunemimiSqlite
INCLUDEPATH += ../libKitsunemimiSqlite/include

LIBS += -L../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../libKitsunemimiCommon/include

LIBS += -L../libKitsunemimiNetwork/src -lKitsunemimiNetwork
LIBS += -L../libKitsunemimiNetwork/src/debug -lKitsunemimiNetwork
LIBS += -L../libKitsunemimiNetwork/src/release -lKitsunemimiNetwork
INCLUDEPATH += ../libKitsunemimiNetwork/include

LIBS += -L../libKitsunemimiJson/src -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/debug -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/release -lKitsunemimiJson
INCLUDEPATH += ../libKitsunemimiJson/include

LIBS += -L../libKitsunemimiIni/src -lKitsunemimiIni
LIBS += -L../libKitsunemimiIni/src/debug -lKitsunemimiIni
LIBS += -L../libKitsunemimiIni/src/release -lKitsunemimiIni
INCLUDEPATH += ../libKitsunemimiIni/include

LIBS += -L../libKitsunemimiJwt/src -lKitsunemimiJwt
LIBS += -L../libKitsunemimiJwt/src/debug -lKitsunemimiJwt
LIBS += -L../libKitsunemimiJwti/src/release -lKitsunemimiJwt
INCLUDEPATH += ../libKitsunemimiJwt/include

LIBS += -L../libKitsunemimiCrypto/src -lKitsunemimiCrypto
LIBS += -L../libKitsunemimiCrypto/src/debug -lKitsunemimiCrypto
LIBS += -L../libKitsunemimiCrypto/src/release -lKitsunemimiCrypto
INCLUDEPATH += ../libKitsunemimiCrypto/include

LIBS += -lcryptopp -lssl -lsqlite3 -luuid -lcrypto -pthread -lprotobuf -lpthread

INCLUDEPATH += $$PWD \
               src

SOURCES += src/main.cpp \
    src/api/v1/cluster_snapshot/create_cluster_snapshot.cpp \
    src/api/v1/cluster_snapshot/delete_cluster_snapshot.cpp \
    src/api/v1/cluster_snapshot/finish_cluster_snapshot.cpp \
    src/api/v1/cluster_snapshot/get_cluster_snapshot.cpp \
    src/api/v1/cluster_snapshot/list_cluster_snapshot.cpp \
    src/api/v1/data_files/check_data_set.cpp \
    src/api/v1/data_files/csv/create_csv_data_set.cpp \
    src/api/v1/data_files/csv/finalize_csv_data_set.cpp \
    src/api/v1/data_files/delete_data_set.cpp \
    src/api/v1/data_files/get_data_set.cpp \
    src/api/v1/data_files/get_progress_data_set.cpp \
    src/api/v1/data_files/list_data_set.cpp \
    src/api/v1/data_files/mnist/create_mnist_data_set.cpp \
    src/api/v1/data_files/mnist/finalize_mnist_data_set.cpp \
    src/api/v1/logs/get_audit_log.cpp \
    src/api/v1/logs/get_error_log.cpp \
    src/api/v1/request_results/delete_request_result.cpp \
    src/api/v1/request_results/get_request_result.cpp \
    src/api/v1/request_results/list_request_result.cpp \
    src/core/data_set_files/data_set_file.cpp \
    src/core/data_set_files/image_data_set_file.cpp \
    src/core/data_set_files/table_data_set_file.cpp \
    src/core/temp_file_handler.cpp \
    src/database/audit_log_table.cpp \
    src/database/cluster_snapshot_table.cpp \
    src/database/data_set_table.cpp \
    src/database/error_log_table.cpp \
    src/database/request_result_table.cpp \
    src/shiori_root.cpp

HEADERS += \
    src/api/blossom_initializing.h \
    src/api/v1/cluster_snapshot/create_cluster_snapshot.h \
    src/api/v1/cluster_snapshot/delete_cluster_snapshot.h \
    src/api/v1/cluster_snapshot/finish_cluster_snapshot.h \
    src/api/v1/cluster_snapshot/get_cluster_snapshot.h \
    src/api/v1/cluster_snapshot/list_cluster_snapshot.h \
    src/api/v1/data_files/check_data_set.h \
    src/api/v1/data_files/csv/create_csv_data_set.h \
    src/api/v1/data_files/csv/finalize_csv_data_set.h \
    src/api/v1/data_files/delete_data_set.h \
    src/api/v1/data_files/get_data_set.h \
    src/api/v1/data_files/get_progress_data_set.h \
    src/api/v1/data_files/list_data_set.h \
    src/api/v1/data_files/mnist/create_mnist_data_set.h \
    src/api/v1/data_files/mnist/finalize_mnist_data_set.h \
    src/api/v1/logs/get_audit_log.h \
    src/api/v1/logs/get_error_log.h \
    src/api/v1/request_results/delete_request_result.h \
    src/api/v1/request_results/get_request_result.h \
    src/api/v1/request_results/list_request_result.h \
    src/args.h \
    src/callbacks.h \
    src/config.h \
    src/core/data_set_files/data_set_file.h \
    src/core/data_set_files/image_data_set_file.h \
    src/core/data_set_files/table_data_set_file.h \
    src/core/temp_file_handler.h \
    src/database/audit_log_table.h \
    src/database/cluster_snapshot_table.h \
    src/database/data_set_table.h \
    src/database/error_log_table.h \
    src/database/request_result_table.h \
    src/shiori_root.h \
    ../libKitsunemimiHanamiMessages/hanami_messages/shiori_messages.h

SHIORI_PROTO_BUFFER = ../libKitsunemimiHanamiMessages/protobuffers/shiori_messages.proto3

OTHER_FILES += $$SHIORI_PROTO_BUFFER

protobuf_decl.name = protobuf headers
protobuf_decl.input = SHIORI_PROTO_BUFFER
protobuf_decl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.proto3.pb.h
protobuf_decl.commands = protoc --cpp_out=${QMAKE_FILE_IN_PATH} --proto_path=${QMAKE_FILE_IN_PATH} ${QMAKE_FILE_NAME}
protobuf_decl.variable_out = HEADERS
QMAKE_EXTRA_COMPILERS += protobuf_decl

protobuf_impl.name = protobuf sources
protobuf_impl.input = SHIORI_PROTO_BUFFER
protobuf_impl.output = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.proto3.pb.cc
protobuf_impl.depends = ${QMAKE_FILE_IN_PATH}/${QMAKE_FILE_BASE}.proto3.pb.h
protobuf_impl.commands = $$escape_expand(\n)
protobuf_impl.variable_out = SOURCES
QMAKE_EXTRA_COMPILERS += protobuf_impl
