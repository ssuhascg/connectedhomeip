/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * @file DeviceCallbacks.cpp
 *
 * Implements all the callbacks to the application from the CHIP Stack
 *
 **/

#include "DeviceCallbacks.h"
#include "AppConfig.h"
#include "BoltLockManager.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/ids/Clusters.h>

static const char * TAG = "lock-devicecallbacks";

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::System;
using namespace ::chip::DeviceLayer;

void AppDeviceCallbacks::PostAttributeChangeCallback(EndpointId endpointId, ClusterId clusterId, AttributeId attributeId,
                                                     uint8_t type, uint16_t size, uint8_t * value)
{
    ESP_LOGI(TAG, "PostAttributeChangeCallback - Cluster ID: '0x%04x', EndPoint ID: '0x%02x', Attribute ID: '0x%04x'", clusterId,
             endpointId, attributeId);

    switch (clusterId)
    {
    case app::Clusters::OnOff::Id:
        OnOnOffPostAttributeChangeCallback(endpointId, attributeId, value);
        break;

    default:
        ESP_LOGI(TAG, "Unhandled cluster ID: %d", clusterId);
        break;
    }

    ESP_LOGI(TAG, "Current free heap: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void AppDeviceCallbacks::OnOnOffPostAttributeChangeCallback(EndpointId endpointId, AttributeId attributeId, uint8_t * value)
{
    VerifyOrExit(attributeId == ZCL_ON_OFF_ATTRIBUTE_ID, ESP_LOGI(TAG, "Unhandled Attribute ID: '0x%04x", attributeId));
    VerifyOrExit(endpointId == 1 || endpointId == 2, ESP_LOGE(TAG, "Unexpected EndPoint ID: `0x%02x'", endpointId));
    if (*value)
    {
        BoltLockMgr().InitiateAction(AppEvent::kEventType_Lock, BoltLockManager::LOCK_ACTION);
		ESP_LOGI(TAG, "\n\n Lock Action \n\n");
    }
    else
    {
        BoltLockMgr().InitiateAction(AppEvent::kEventType_Lock, BoltLockManager::UNLOCK_ACTION);
		ESP_LOGI(TAG, "\n\n Unlock Action \n\n");
    }
exit:
    return;
}

//plugin for lock-door command
bool emberAfPluginDoorLockOnDoorLockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    BoltLockManager::State_t mState;
    mState = BoltLockManager::kState_UnlockingCompleted;
    ESP_LOGI(TAG, "\n\n lock-door command plugin  \n\n");
    if(mState == BoltLockManager::kState_UnlockingCompleted) {
            BoltLockMgr().InitiateAction(AppEvent::kEventType_Lock, BoltLockManager::LOCK_ACTION);
            ESP_LOGI(TAG, "Door Locked\n");
     } else {
            ESP_LOGI(TAG, "Door is already locked\n");
     }
    err = DlOperationError::kUnspecified;
    return true;
}

//plugin for unlock-door command
bool emberAfPluginDoorLockOnDoorUnlockCommand(chip::EndpointId endpointId, const Optional<ByteSpan> & pinCode, DlOperationError & err)
{
    BoltLockManager::State_t mState;
    mState = BoltLockManager::kState_LockingCompleted;
    ESP_LOGI(TAG, "\n\n unlock-door command plugin \n\n");
    if(mState == BoltLockManager::kState_LockingCompleted) {
        BoltLockMgr().InitiateAction(AppEvent::kEventType_Lock, BoltLockManager::UNLOCK_ACTION);
        ESP_LOGI(TAG, "Door unlocked\n");
    } else {
        ESP_LOGI(TAG, "Door is already unlocked!!!!\n");
    }
    err = DlOperationError::kUnspecified;
    return true;
}

//plugin for getuser api
bool emberAfPluginDoorLockGetUser(chip::EndpointId endpointId, uint16_t userIndex, EmberAfPluginDoorLockUserInfo & user)
{
        ESP_LOGI(TAG, "emberAfPluginDoorLockGetUser\n");
        user.userName = BoltLockMgr().user1.userName;
        user.userUniqueId = BoltLockMgr().user1.userUniqueId;
        user.userStatus = BoltLockMgr().user1.userStatus;
        user.userType = BoltLockMgr().user1.userType;
        user.credentialRule = BoltLockMgr().user1.credentialRule;
        user.createdBy = BoltLockMgr().user1.createdBy;
        ESP_LOGI(TAG, "userUniqueId = 0x%" PRIx32, user.userUniqueId);
        return true;
}

//plugin for set-user command
bool emberAfPluginDoorLockSetUser(chip::EndpointId endpointId, uint16_t userIndex, FabricIndex creator, FabricIndex modifier, const chip::CharSpan & userName, uint32_t uniqueId, DlUserStatus userStatus, DlUserType usertype, DlCredentialRule credentialRule, const DlCredential * credentials, size_t totalCredentials)
{
    ESP_LOGI(TAG, "set-user command plugin\n");
    MutableCharSpan uName;
    uName = MutableCharSpan(static_cast<char *>(Platform::MemoryAlloc(userName.size())),userName.size());
    CopyCharSpanToMutableCharSpan(userName, uName);
    BoltLockMgr().user1.userName = CharSpan(uName);
    ESP_LOGI(TAG,"Username = \"%.*s\" from set-user", static_cast<int>(BoltLockMgr().user1.userName.size()), BoltLockMgr().user1.userName.data());
    BoltLockMgr().user1.userUniqueId = uniqueId;
    BoltLockMgr().user1.userStatus = userStatus;
    BoltLockMgr().user1.userType = usertype;
    BoltLockMgr().user1.credentialRule = credentialRule;
    BoltLockMgr().user1.createdBy = creator;
    return true;
}


