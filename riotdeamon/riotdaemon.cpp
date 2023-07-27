/*
##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
*/
#include <cstdlib>
#include <iostream>
#include <condition_variable>
#include <thread>

#include "rtConnection.h"
#include "rtLog.h"
#include "rtMessage.h"

using namespace std;
void onAvailableDevices(rtMessageHeader const *hdr, uint8_t const *buff, uint32_t n, void *closure);
void onDeviceProperties(rtMessageHeader const *hdr, uint8_t const *buff, uint32_t n, void *closure);
void onDeviceProperty(rtMessageHeader const *hdr, uint8_t const *buff, uint32_t n, void *closure);
void onSendCommand(rtMessageHeader const *hdr, uint8_t const *buff, uint32_t n, void *closure);

rtConnection con;
bool m_isActive = true;
std::condition_variable m_act_cv;
std::mutex m_lock;
void initialize()
{
    rtConnection_Create(&con, "IOTGateway", "tcp://127.0.0.1:10001");
}
void registerForServices()
{
    rtConnection_AddListener(con, "getAvailableDevices", onAvailableDevices, con);
    rtConnection_AddListener(con, "getDeviceProperties", onDeviceProperties, con);
    rtConnection_AddListener(con, "getDeviceProperty", onDeviceProperty, con);
    rtConnection_AddListener(con, "sendCommand", onSendCommand, con);
}

void onAvailableDevices(rtMessageHeader const *rtHeader, uint8_t const *rtMsg, uint32_t rtMsgLength, void *userData)
{
    rtConnection con = (rtConnection)userData;
    rtMessage req;
    rtMessage_FromBytes(&req, rtMsg, rtMsgLength);

    if (rtMessageHeader_IsRequest(rtHeader))
    {
        rtMessage res;
        rtMessage_Create(&res);

        rtMessage device;
        rtMessage_Create(&device);

        rtMessage_SetString(device, "name", "Philips");
        rtMessage_SetString(device, "uuid", "1234-PHIL-LIGHT-BULB");
        rtMessage_SetString(device, "devType", "1");
        rtMessage_AddMessage(res, "devices", device);
        rtMessage_Release(device);

        rtMessage_Create(&device);
        rtMessage_SetString(device, "name", "Hewei-HDCAM-1234");
        rtMessage_SetString(device, "devType", "0");

        rtMessage_AddMessage(res, "devices", device);
        rtMessage_Release(device);

        rtConnection_SendResponse(con, rtHeader, res, 1000);
        rtMessage_Release(res);
    }
    rtMessage_Release(req);
}
void onDeviceProperties(rtMessageHeader const *rtHeader, uint8_t const *rtMsg, uint32_t rtMsgLength, void *userData)
{
    rtConnection con = (rtConnection)userData;
    rtMessage req;
    rtMessage_FromBytes(&req, rtMsg, rtMsgLength);

    if (rtMessageHeader_IsRequest(rtHeader))
    {
        rtMessage res;
        rtMessage_Create(&res);

        char *uuid;
        rtMessage_GetString(req, "uuid", &uuid);

        cout << "Device identifier is " << uuid << endl;
        free(uuid);

        rtMessage props;
        rtMessage_Create(&props);

        rtMessage_AddString(props, "properties", "Prop1=Josekutty");
        rtMessage_AddString(props, "properties", "Prop2=Kottarathil");
        rtMessage_AddString(props, "properties", "Prop3=Comcast");
        rtMessage_AddString(props, "properties", "Prop4=Engineer");

        rtMessage_SetMessage(res, "properties", props);
        rtConnection_SendResponse(con, rtHeader, res, 1000);
        rtMessage_Release(res);
    }
    rtMessage_Release(req);
}
void onDeviceProperty(rtMessageHeader const *rtHeader, uint8_t const *rtMsg, uint32_t rtMsgLength, void *userData)
{
    rtConnection con = (rtConnection)userData;
    rtMessage req;
    rtMessage_FromBytes(&req, rtMsg, rtMsgLength);

    if (rtMessageHeader_IsRequest(rtHeader))
    {
        rtMessage res;
        rtMessage_Create(&res);

        char *uuid, *property;
        rtMessage_GetString(req, "uuid", &uuid);
        rtMessage_GetString(req, "property", &property);

        cout << "Device identifier is " << uuid << ", Property requested :" << property << endl;
        free(uuid);
        free(property);

        rtMessage_SetString(res, "value", "Hooked.");
        rtConnection_SendResponse(con, rtHeader, res, 1000);
        rtMessage_Release(res);
    }
    rtMessage_Release(req);
}
void onSendCommand(rtMessageHeader const *rtHeader, uint8_t const *rtMsg, uint32_t rtMsgLength, void *userData)
{
    rtConnection con = (rtConnection)userData;
    rtMessage req;
    rtMessage_FromBytes(&req, rtMsg, rtMsgLength);

    if (rtMessageHeader_IsRequest(rtHeader))
    {
        rtMessage res;
        rtMessage_Create(&res);

        char *uuid, *property;
        rtMessage_GetString(req, "uuid", &uuid);
        rtMessage_GetString(req, "command", &property);

        cout << "Device identifier is " << uuid << ", command requested :" << property << endl;
        free(uuid);
        free(property);

        rtMessage_SetInt32(res, "result", 1);
        rtConnection_SendResponse(con, rtHeader, res, 1000);
        rtMessage_Release(res);
    }
    rtMessage_Release(req);
}

void handleTermSignal(int _signal)
{
    cout << "Exiting from app.." << endl;

    unique_lock<std::mutex> ulock(m_lock);
    m_isActive = false;
    m_act_cv.notify_one();
}
void waitForTermSignal()
{
    cout << "Waiting for term signal.. " << endl;
    thread termThread([&]()
                      {
    while (m_isActive)
    {
        unique_lock<std::mutex> ulock(m_lock);
        m_act_cv.wait(ulock);
    }
    
    cout<<"[SmartMonitor::waitForTermSignal] Received term signal."<<endl; });
    termThread.join();
}
int main(int argc, char const *argv[])
{
    rtLog_SetLevel(RT_LOG_DEBUG);

    rtConnection con;
    rtConnection_Create(&con, "IOTGateway", "tcp://127.0.0.1:10001");
    rtConnection_AddListener(con, "getAvailableDevices", onAvailableDevices, con);
    rtConnection_AddListener(con, "getDeviceProperties", onDeviceProperties, con);
    rtConnection_AddListener(con, "getDeviceProperty", onDeviceProperty, con);
    rtConnection_AddListener(con, "sendCommand", onSendCommand, con);
    waitForTermSignal();
    rtConnection_Destroy(con);
    return 0;
}
