/**
 * If not stated otherwise in this file or this component's LICENSE
 * file the following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/

#include <string>
#include <list>
#include <memory>
#include <iostream>
#include "rtConnection.h"
#include "rtMessage.h"

#include "RIoTConnector.h"
#define RBUS_METHOD_GETDEVICES "GetAvailableDevices"
#define RBUS_METHOD_GETDEVICEPROPERTIES "GetDeviceProperties"
#define RBUS_METHOD_GETDEVICEPROPERTY "GetDeviceProperty"
#define RBUS_METHOD_SENDCOMMAND "SendCommand"

#define RTMESSAGE_TIMEOUT_MILLIS 2000
namespace WPEFramework
{
    namespace iotbridge
    {

        rtConnection con;
        rtError rtConnStatus = RT_NO_CONNECTION;

        bool initializeIPC(const std::string &remoteAddr)
        {
            std::cout << "[initializeIPC]Connecting to remote server " << remoteAddr << std::endl;
            rtConnStatus = rtConnection_Create(&con, "RIoT", remoteAddr.c_str());
            std::cout << "[initializeIPC]Connection status " << rtConnStatus << std::endl;
            return rtConnStatus == RT_OK ? true : false;
        }

        void cleanupIPC()
        {
            if (RT_OK == rtConnStatus)
                rtConnection_Destroy(con);
            rtConnStatus = RT_NO_CONNECTION;
        }
        int getDeviceList(std::list<std::shared_ptr<IOTDevice>> &deviceList)
        {
            int count = -1;
            std::cout << "[getDeviceList] Connection status  " << rtConnStatus << std::endl;
            if (RT_OK != rtConnStatus)
                return count;

            rtError result;
            rtMessage res;
            rtMessage req;
            rtMessage_Create(&req);
            rtMessage_SetSendTopic(req, "RIoT");
            result = rtConnection_SendRequest(con, req, RBUS_METHOD_GETDEVICES, &res, RTMESSAGE_TIMEOUT_MILLIS);
            std::cout << "[getDeviceList] RPC returns " << rtStrError(result) << std::endl;
            if (RT_OK == result)
            {

                rtMessage devices;
                char *entry;
                uint32_t outLen;
                rtMessage_ToString(res, &entry, &outLen);
                std::cout << "[getDeviceList]Returning the response " << entry << std::endl;
                free(entry);
                rtMessage_GetMessage(res, "devices", &devices);
                rtMessage_GetArrayLength(res, "devices", &count);
                std::cout << "[getDeviceList] array count " << count << std::endl;

                for (int i = 0; i < count; i++)
                {
                    std::shared_ptr<IOTDevice> device(new IOTDevice());

                    rtMessage devEntry;
                    result = rtMessage_GetMessageItem(res, "devices", i, &devEntry);
                    std::cout << "[getDeviceList] rtMessage_GetMessageItem for "<<i<<" returns " <<result<<std::endl;
                
                rtMessage_ToString(devEntry, &entry, &outLen);
                std::cout << "[getDeviceList]Returning the response " << entry << std::endl;
                free(entry);

                    rtMessage_GetString(devEntry, "name", (const char **)&entry);
                    device->deviceName = entry;

                    rtMessage_GetString(devEntry, "uuid", (const char **)&entry);
                    device->deviceId = entry;

                    rtMessage_GetString(devEntry, "devType", (const char **)&entry);
                    int x = std::stoi(entry);
                    device->devType = x == 0 ? CAMERA : LIGHT_BULB;

                    deviceList.push_back(device);
                }
                std::cout << "[getDeviceList] Done "<<std::endl;
                rtMessage_Release(res);
            }
            else
            {
                std::cout << "[getDeviceList] rtConnection_SendRequest failed   " << std::endl;
            }
            std::cout << "[getDeviceList] Total count " << deviceList.size() << std::endl;
            rtMessage_Release(req);

            return count;
        }
        int getDeviceProperties(const std::string &uuid, std::list<std::string> &propList)
        {
            int count = -1;

            if (RT_OK != rtConnStatus)
                return count;
            rtError err;
            rtMessage res, req;
            rtMessage_Create(&req);
            rtMessage_SetString(req, "deviceId", uuid.c_str());

            rtMessage_SetSendTopic(req, "RIoT");
            err = rtConnection_SendRequest(con, req, RBUS_METHOD_GETDEVICEPROPERTIES, &res, RTMESSAGE_TIMEOUT_MILLIS);
            std::cout << "RPC returns " << rtStrError(err) << std::endl;
            if (RT_OK == err)
            {

                rtMessage properties;
                rtMessage_GetArrayLength(res, "properties", &count);
                rtMessage_GetMessage(res, "properties", &properties);

                for (int i = 0; i < count; i++)
                {
                    char *entry;
                    rtMessage_GetStringItem(res, "devices", count, (const char **)&entry);
                    propList.push_back(entry);
                }
                rtMessage_Release(res);
            }
            rtMessage_Release(req);
            return count;
        }
        std::string getDeviceProperty(const std::string &uuid, const std::string &propertyName)
        {
            std::string value;

            if (RT_OK != rtConnStatus)
                return "";

            rtError err;
            rtMessage res;
            rtMessage req;

            rtMessage_Create(&req);
            rtMessage_SetSendTopic(req, "RIoT");
            rtMessage_SetString(req, "deviceId", uuid.c_str());
            rtMessage_SetString(req, "property", propertyName.c_str());
            err = rtConnection_SendRequest(con, req, RBUS_METHOD_GETDEVICEPROPERTY, &res, RTMESSAGE_TIMEOUT_MILLIS);
            std::cout << "RPC returns " << rtStrError(err) << std::endl;

            if (RT_OK == err)
            {
                char *entry;
                rtMessage_GetString(res, "value", (const char **)&entry);
                value = entry;
                rtMessage_Release(res);
            }

            rtMessage_Release(req);
            return value;
        }
        int sendCommand(const std::string &uuid, const std::string &cmd)
        {
            int status = -1;

            if (RT_OK != rtConnStatus)
                return status;

            rtError err;
            rtMessage res;
            rtMessage req;

            rtMessage_Create(&req);
            rtMessage_SetSendTopic(req, "RIoT");
            rtMessage_SetString(req, "deviceId", uuid.c_str());
            rtMessage_SetString(req, "command", cmd.c_str());
            err = rtConnection_SendRequest(con, req, RBUS_METHOD_SENDCOMMAND, &res, RTMESSAGE_TIMEOUT_MILLIS);
            std::cout << "RPC returns " << rtStrError(err) << std::endl;

            if (RT_OK == err)
            {
                rtMessage_Release(res);
                status = 0;
            }

            rtMessage_Release(req);
            return status;
        }

    } // namespace iotbridge
} // namespace wpeframework