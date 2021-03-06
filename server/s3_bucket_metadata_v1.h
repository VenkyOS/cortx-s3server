/*
 * Copyright (c) 2020 Seagate Technology LLC and/or its Affiliates
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For any questions about this software or licensing,
 * please email opensource@seagate.com or cortx-questions@seagate.com.
 *
 */

#pragma once

#ifndef __S3_SERVER_S3_BUCKET_METADATA_V1_H__
#define __S3_SERVER_S3_BUCKET_METADATA_V1_H__

#include <gtest/gtest_prod.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include "s3_global_bucket_index_metadata.h"
#include "s3_bucket_metadata.h"
#include "s3_log.h"
#include "s3_timer.h"

// Forward declarations
class S3GlobalBucketIndexMetadataFactory;

class S3BucketMetadataV1 : public S3BucketMetadata {
  // Holds mainly system-defined metadata (creation date etc)
  // Partially supported on need bases, some of these are placeholders
 protected:
  std::string bucket_owner_account_id;

  std::shared_ptr<S3GlobalBucketIndexMetadataFactory>
      global_bucket_index_metadata_factory;

  std::shared_ptr<S3GlobalBucketIndexMetadata> global_bucket_index_metadata;

  S3Timer s3_timer;

 private:
  void fetch_global_bucket_account_id_info();
  void fetch_global_bucket_account_id_info_success();
  void fetch_global_bucket_account_id_info_failed();

  void load_bucket_info();
  void load_bucket_info_successful();
  void load_bucket_info_failed();

  void create_object_list_index();
  void create_object_list_index_successful();
  void create_object_list_index_failed();

  void create_multipart_list_index();
  void create_multipart_list_index_successful();
  void create_multipart_list_index_failed();

  void create_objects_version_list_index();
  void create_objects_version_list_index_successful();
  void create_objects_version_list_index_failed();

  void save_bucket_info(bool clean_glob_on_err = false);
  void save_bucket_info_successful();
  void save_bucket_info_failed();

  void remove_bucket_info();
  void remove_bucket_info_successful();
  void remove_bucket_info_failed();

  void remove_global_bucket_account_id_info();
  void remove_global_bucket_account_id_info_successful();
  void remove_global_bucket_account_id_info_failed();

  void save_global_bucket_account_id_info();
  void save_global_bucket_account_id_info_successful();
  void save_global_bucket_account_id_info_failed();

  // CreateBucket operation does following (in context of account A1):
  // 1. Create entry in global bucket list index (e.g., with key=bucket_name,
  // value=A1)
  // 2. On success of above(1), create further:
  // 2.1 Create object list index
  // 2.2 Create multipart list index
  // 2.3 Create object version list index
  // 2.4 Create entry in bucket metadata list index (key=A1/<bucket_name>,
  // value=<bucket metadata info>)
  // If any of the sub operations from 2.1 through 2.4 fail, CreateBucket
  // operation is marked as failed, sending response to
  // client(ServiceUnavailable).
  // However, on failure in point (2) above, the entry created in point (1)
  // above is never reverted/removed. This will lead to inconsistency in backend
  // S3 metadata. i.e. entry is there in global bucket list index, but not in
  // bucket metadata list index.
  // Following function should be called in case of errors in points 2.1 through
  // 2.4 to revert entry from global bucket list index
  void cleanup_on_create_err_global_bucket_account_id_info(
      S3MotrKVSWriterOpState op_state);
  void cleanup_on_create_err_global_bucket_account_id_info_fini_cb();
  S3BucketMetadataState on_cleanup_state;
  bool should_cleanup_global_idx;

  std::string get_bucket_metadata_index_key_name() {
    return bucket_owner_account_id + "/" + bucket_name;
  }

  // for UTs only
  struct m0_uint128 get_bucket_metadata_list_index_oid();
  void set_bucket_metadata_list_index_oid(struct m0_uint128 id);
  void set_state(S3BucketMetadataState state);

 public:
  S3BucketMetadataV1(
      std::shared_ptr<S3RequestObject> req,
      std::shared_ptr<MotrAPI> motr_api = nullptr,
      std::shared_ptr<S3MotrKVSReaderFactory> motr_s3_kvs_reader_factory =
          nullptr,
      std::shared_ptr<S3MotrKVSWriterFactory> motr_s3_kvs_writer_factory =
          nullptr,
      std::shared_ptr<S3GlobalBucketIndexMetadataFactory>
          s3_global_bucket_index_metadata_factory = nullptr);

  S3BucketMetadataV1(
      std::shared_ptr<S3RequestObject> req, const std::string& str_bucket_name,
      std::shared_ptr<MotrAPI> motr_api = nullptr,
      std::shared_ptr<S3MotrKVSReaderFactory> motr_s3_kvs_reader_factory =
          nullptr,
      std::shared_ptr<S3MotrKVSWriterFactory> motr_s3_kvs_writer_factory =
          nullptr,
      std::shared_ptr<S3GlobalBucketIndexMetadataFactory>
          s3_global_bucket_index_metadata_factory = nullptr);

  virtual void load(std::function<void(void)> on_success,
                    std::function<void(void)> on_failed);

  virtual void save(std::function<void(void)> on_success,
                    std::function<void(void)> on_failed);

  virtual void update(std::function<void(void)> on_success,
                      std::function<void(void)> on_failed);

  virtual void remove(std::function<void(void)> on_success,
                      std::function<void(void)> on_failed);

  virtual S3BucketMetadataState get_state() { return state; }

  // Google tests
  FRIEND_TEST(S3BucketMetadataV1Test, ConstructorTest);
  FRIEND_TEST(S3BucketMetadataV1Test, GetSystemAttributesTest);
  FRIEND_TEST(S3BucketMetadataV1Test, GetSetOIDsPolicyAndLocation);
  FRIEND_TEST(S3BucketMetadataV1Test, DeletePolicy);
  FRIEND_TEST(S3BucketMetadataV1Test, GetTagsAsXml);
  FRIEND_TEST(S3BucketMetadataV1Test, GetSpecialCharTagsAsXml);
  FRIEND_TEST(S3BucketMetadataV1Test, AddSystemAttribute);
  FRIEND_TEST(S3BucketMetadataV1Test, AddUserDefinedAttribute);
  FRIEND_TEST(S3BucketMetadataV1Test, Load);
  FRIEND_TEST(S3BucketMetadataV1Test, FetchBucketListIndexOid);
  FRIEND_TEST(S3BucketMetadataV1Test,
              FetchBucketListIndexOIDSuccessStateIsSaving);
  FRIEND_TEST(S3BucketMetadataV1Test,
              FetchBucketListIndexOIDSuccessStateIsFetching);
  FRIEND_TEST(S3BucketMetadataV1Test,
              FetchBucketListIndexOIDSuccessStateIsDeleting);
  FRIEND_TEST(S3BucketMetadataV1Test, FetchBucketListIndexOIDFailedState);
  FRIEND_TEST(S3BucketMetadataV1Test,
              FetchBucketListIndexOIDFailedStateIsSaving);
  FRIEND_TEST(S3BucketMetadataV1Test,
              FetchBucketListIndexOIDFailedStateIsFetching);
  FRIEND_TEST(S3BucketMetadataV1Test,
              FetchBucketListIndexOIDFailedIndexMissingStateSaved);
  FRIEND_TEST(S3BucketMetadataV1Test,
              FetchBucketListIndexOIDFailedIndexFailedStateIsFetching);
  FRIEND_TEST(S3BucketMetadataV1Test, FetchBucketListIndexOIDFailedIndexFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, LoadBucketInfo);
  FRIEND_TEST(S3BucketMetadataV1Test, SetBucketPolicy);
  FRIEND_TEST(S3BucketMetadataV1Test, SetAcl);
  FRIEND_TEST(S3BucketMetadataV1Test, LoadBucketInfoFailedJsonParsingFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, LoadBucketInfoFailedMetadataMissing);
  FRIEND_TEST(S3BucketMetadataV1Test, LoadBucketInfoFailedMetadataFailed);
  FRIEND_TEST(S3BucketMetadataV1Test,
              LoadBucketInfoFailedMetadataFailedToLaunch);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveMetadataIndexOIDMissing);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveMetadataIndexOIDPresent);
  FRIEND_TEST(S3BucketMetadataV1Test, UpdateMetadataIndexOIDPresent);
  FRIEND_TEST(S3BucketMetadataV1Test, UpdateMetadataIndexOIDMissing);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateObjectIndexOIDNotPresent);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateBucketListIndexCollisionCount0);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateBucketListIndexCollisionCount1);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateBucketListIndexSuccessful);
  FRIEND_TEST(S3BucketMetadataV1Test,
              CreateBucketListIndexFailedCollisionHappened);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateBucketListIndexFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateBucketListIndexFailedToLaunch);
  FRIEND_TEST(S3BucketMetadataV1Test, HandleCollision);
  FRIEND_TEST(S3BucketMetadataV1Test, HandleCollisionMaxAttemptExceeded);
  FRIEND_TEST(S3BucketMetadataV1Test, RegeneratedNewIndexName);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketListIndexOid);
  FRIEND_TEST(S3BucketMetadataV1Test,
              SaveBucketListIndexOidSucessfulStateSaving);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketListIndexOidSucessful);
  FRIEND_TEST(S3BucketMetadataV1Test,
              SaveBucketListIndexOidSucessfulBcktMetaMissing);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketListIndexOIDFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketListIndexOIDFailedToLaunch);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketInfo);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketInfoFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketInfoSuccess);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketInfoFailedToLaunch);
  FRIEND_TEST(S3BucketMetadataV1Test, SaveBucketInfoFailedWithGlobCleanup);
  FRIEND_TEST(S3BucketMetadataV1Test,
              SaveBucketInfoFailedToLaunchWithGlobCleanup);
  FRIEND_TEST(S3BucketMetadataV1Test, RemovePresentMetadata);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveAbsentMetadata);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveBucketInfo);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveBucketInfoSuccessful);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveBucketAccountidInfoSuccessful);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveBucketAccountidInfoFailedToLaunch);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveBucketAccountidInfoFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveBucketInfoFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, RemoveBucketInfoFailedToLaunch);
  FRIEND_TEST(S3BucketMetadataV1Test, ToJson);
  FRIEND_TEST(S3BucketMetadataV1Test, FromJson);
  FRIEND_TEST(S3BucketMetadataV1Test, GetEncodedBucketAcl);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateMultipartListIndexCollisionCount0);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateObjectListIndexCollisionCount0);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateObjectListIndexSuccessful);
  FRIEND_TEST(S3BucketMetadataV1Test,
              CreateObjectListIndexFailedCollisionHappened);
  FRIEND_TEST(S3BucketMetadataV1Test,
              CreateMultipartListIndexFailedCollisionHappened);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateObjectListIndexFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateObjectListIndexFailedToLaunch);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateMultipartListIndexFailed);
  FRIEND_TEST(S3BucketMetadataV1Test, CreateMultipartListIndexFailedToLaunch);
};

#endif
