/* Copyright (c) 2020 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License,
 * attached with Common Clause Condition 1.0, found in the LICENSES directory.
 */

#ifndef STORAGE_ADMIN_REBUILDINDEXTASK_H_
#define STORAGE_ADMIN_REBUILDINDEXTASK_H_

#include "kvstore/LogEncoder.h"
#include "meta/IndexManager.h"
#include "storage/admin/AdminTask.h"
#include "interface/gen-cpp2/storage_types.h"

namespace nebula {
namespace storage {

class RebuildIndexTask : public AdminTask {
public:
    explicit RebuildIndexTask(StorageEnv* env, TaskContext&& ctx)
    : AdminTask(env, std::move(ctx)) {}

    ErrorOr<cpp2::ErrorCode, std::vector<AdminSubTask>> genSubTasks() override;

protected:
    virtual StatusOr<std::shared_ptr<nebula::meta::cpp2::IndexItem>>
    getIndex(GraphSpaceID space, IndexID indexID) = 0;

    virtual kvstore::ResultCode
    buildIndexGlobal(GraphSpaceID space,
                     PartitionID part,
                     meta::cpp2::SchemaID SchemaID,
                     IndexID indexID,
                     const std::vector<meta::cpp2::ColumnDef>& cols) = 0;

    StatusOr<IndexValues> collectIndexValues(RowReader* reader,
                                             const std::vector<meta::cpp2::ColumnDef>& cols);

    void cancel() override {
        canceled_ = true;
    }

private:
    kvstore::ResultCode buildIndexOnOperations(GraphSpaceID space,
                                               PartitionID part);

    kvstore::ResultCode processModifyOperation(GraphSpaceID space,
                                               PartitionID part,
                                               std::vector<kvstore::KV>& data,
                                               kvstore::KVStore* kvstore);

    kvstore::ResultCode processRemoveOperation(GraphSpaceID space,
                                               PartitionID part,
                                               std::vector<std::string>& keys,
                                               kvstore::KVStore* kvstore);

    kvstore::ResultCode genSubTask(GraphSpaceID space,
                                   PartitionID part,
                                   meta::cpp2::SchemaID schemaID,
                                   int32_t indexID,
                                   std::shared_ptr<meta::cpp2::IndexItem> item,
                                   bool isOffline);

protected:
    std::atomic<bool>   canceled_{false};
    std::mutex          lock_;
    int32_t             callingNum_{0};
    meta::IndexManager* indexMan_{nullptr};
};

}  // namespace storage
}  // namespace nebula
#endif  // STORAGE_ADMIN_REBUILDINDEXTASK_H_