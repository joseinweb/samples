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
    cout << "[onAvailableDevices]Received the message " << endl;
    rtMessage_FromBytes(&req, rtMsg, rtMsgLength);
    rtMessage_ToString(req, (char **)&rtMsg, &rtMsgLength);
    rtLog_Info("req:%.*s", rtMsgLength, rtMsg);

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

        char *output;
        int outLen;

        rtMessage_Create(&device);
        rtMessage_SetString(device, "name", "Hewei-HDCAM-1234");
        rtMessage_SetString(device, "devType", "0");

        rtMessage_AddMessage(res, "devices", device);

        rtMessage_ToString(res, &output, &outLen);
        cout << "[onAvailableDevices]Returning the response " << output << endl;

        rtConnection_SendResponse(con, rtHeader, res, 1000);
        // rtMessage_Release(res);
    }
    else
    {
        cout << "[onAvailableDevices]Received  message not a request. Ignoring.." << endl;
    }
    // rtMessage_Release(req);
}
void onDeviceProperties(rtMessageHeader const *rtHeader, uint8_t const *rtMsg, uint32_t rtMsgLength, void *userData)
{
    rtConnection con = (rtConnection)userData;
    rtMessage req;
    cout << "[onDeviceProperties]Received  message .." << endl;

    rtMessage_FromBytes(&req, rtMsg, rtMsgLength);

    if (rtMessageHeader_IsRequest(rtHeader))
    {
        rtMessage res;
        rtMessage_Create(&res);

        char *uuid;
        rtMessage_GetString(req, "deviceId", &uuid);

        cout << "Device identifier is " << uuid << endl;



        rtMessage_AddString(res, "properties", "Prop1=Josekutty");
        rtMessage_AddString(res, "properties", "Prop2=Kottarathil");
        rtMessage_AddString(res, "properties", "Prop3=Comcast");
        rtMessage_AddString(res, "properties", "Prop4=Engineer");

        rtConnection_SendResponse(con, rtHeader, res, 1000);
        rtMessage_Release(res);
    }
    else
    {
        cout << "[onDeviceProperties]Received  message not a request. Ignoring.." << endl;
    }
    rtMessage_Release(req);
}
void onDeviceProperty(rtMessageHeader const *rtHeader, uint8_t const *rtMsg, uint32_t rtMsgLength, void *userData)
{
    rtConnection con = (rtConnection)userData;
    rtMessage req;

    cout << "[onDeviceProperty]Received  message .." << endl;
    rtMessage_FromBytes(&req, rtMsg, rtMsgLength);
    rtMessage_ToString(req, (char **)&rtMsg, &rtMsgLength);
    rtLog_Info("req:%.*s", rtMsgLength, rtMsg);

    if (rtMessageHeader_IsRequest(rtHeader))
    {
        rtMessage res;
        rtMessage_Create(&res);

        char *uuid, *property;
        rtMessage_GetString(req, "deviceId", &uuid);
        rtMessage_GetString(req, "property", &property);

        cout << "Device identifier is " << uuid << ", Property requested :" << property << endl;
        rtMessage_SetString(res, "value", "Hooked.");
        rtConnection_SendResponse(con, rtHeader, res, 1000);
        rtMessage_Release(res);
    }
    else
    {
        cout << "[onDeviceProperty]Received  message not a request. Ignoring.." << endl;
    }
    rtMessage_Release(req);
}
void onSendCommand(rtMessageHeader const *rtHeader, uint8_t const *rtMsg, uint32_t rtMsgLength, void *userData)
{
    rtConnection con = (rtConnection)userData;
    rtMessage req;

    cout << "[onSendCommand]Received  message .." << endl;
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
    else
    {
        cout << "[onSendCommand]Received  message not a request. Ignoring.." << endl;
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
    cout << "RIoT Sample Daemon 1.0" << endl;
    rtConnection con;
    rtConnection_Create(&con, "IOTGateway", argc == 1 ? "tcp://127.0.0.1:10001" : argv[1]);
    rtConnection_AddListener(con, "GetAvailableDevices", onAvailableDevices, con);
    rtConnection_AddListener(con, "GetDeviceProperties", onDeviceProperties, con);
    rtConnection_AddListener(con, "GetDeviceProperty", onDeviceProperty, con);
    rtConnection_AddListener(con, "SendCommand", onSendCommand, con);
    waitForTermSignal();
    rtConnection_Destroy(con);
    return 0;
}
