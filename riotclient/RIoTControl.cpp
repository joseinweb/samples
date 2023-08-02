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

#include "RIoTControl.h"
#include "AvahiClient.h"
#include "RIoTConnector.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{
    namespace Plugin
    {

        std::string getRemoteAddress()
        {
            if (avahi::initialize())
            {
                std::list<std::shared_ptr<avahi::RDKDevice> > devices;
                if (avahi::discoverDevices(devices) > 0)
                {
                    // Find the one with ipv4 address and return
                    for (std::list<std::shared_ptr<avahi::RDKDevice> >::iterator it = devices.begin(); it != devices.end(); ++it)
                    {
                        std::shared_ptr<avahi::RDKDevice> device = *it;
                        std::cout << "Found IOT Gateway device " << device->ipAddress << ":" << device->port << std::endl;
                        if (device->addrType == avahi::IP_ADDR_TYPE::IPV4)
                        {
                            std::cout << "Found ipv4 device " << device->ipAddress << ":" << device->port << std::endl;
                            return "tcp://" + device->ipAddress + ":" + std::to_string(device->port);
                        }
                    }
                }
                else
                {
                    std::cout << " Failed to identify RDK IoT Gateway." << std::endl;
                }
                avahi::unInitialize();
                std::cout << " Failed to identify IPV4 RDK IoT Gateway." << std::endl;
            }
            else
            {
                std::cout << " Failed to initialize avahi " << std::endl;
            }

            return "";
        }

        RIoTControl::RIoTControl()
            : m_apiVersionNumber(API_VERSION_NUMBER_MAJOR)
        {
        }

        RIoTControl::~RIoTControl()
        {
        }

        bool RIoTControl::connectToRemote()
        {
            if (remote_addr.empty())
            {
                remote_addr = getRemoteAddress();
            }

            if (!remote_addr.empty())
                return iotbridge::initializeIPC(remote_addr);
            return false;
        }

        // Supported methods
        bool RIoTControl::getAvailableDevicesWrapper()
        {
            bool success = false;
            if (connectToRemote())
            {
                std::list<std::shared_ptr<WPEFramework::iotbridge::IOTDevice> > deviceList;
                if (iotbridge::getDeviceList(deviceList) > 0)
                {
                    std::cout << "[getAvailableDevicesWrapper] total count "<<deviceList.size()<<std::endl;

                    for (const auto &device : deviceList)
                    {
                        std::cout << "Found device " << device->deviceName << " , " << device->deviceId << std::endl;
                    }
                }
                success = true;
                iotbridge::cleanupIPC();
            }
            else
            {
                std::cout << "Failed to connect to IoT Gateway" << std::endl;
            }
            return (success);
        }

        bool RIoTControl::getDeviceProperties(const std::string &uuid,std::list<std::string> &properties)
        {
            bool success = false;

            if (connectToRemote())
            {
                iotbridge::getDeviceProperties(uuid,properties);
                std::cout << " Value returned is " << properties.size() << std::endl;
                success = true;
                iotbridge::cleanupIPC();
            }
            else
            {
                std::cout << "Failed to connect to IoT Gateway" << std::endl;
            }

            return (success);
        }
        bool RIoTControl::getDeviceProperty(const std::string &uuid, const std::string &prop)
        {
            bool success = false;

            if (connectToRemote())
            {
                std::string propVal = iotbridge::getDeviceProperty(uuid, prop);
                std::cout << " Value returned is " << propVal << std::endl;
                success = true;
                iotbridge::cleanupIPC();
            }
            else
            {
                std::cout << "Failed to connect to IoT Gateway" << std::endl;
            }

            return (success);
        }
        bool RIoTControl::sendCommand(const std::string &uuid, const std::string &cmd)
        {
            bool success = false;

            if (connectToRemote())
            {
                if (0 == iotbridge::sendCommand(uuid, cmd))
                    success = true;
                std::cout << "Send command successfully. " << std::endl;
                iotbridge::cleanupIPC();
            }
            else
            {
                std::cout << "Failed to connect to IoT Gateway" << std::endl;
            }

            return (success);
        }

    } // namespace Plugin
} // namespace WPEFramework

int main(int argc, char const *argv[])
{
    /* code */

    using WPEFramework::Plugin::RIoTControl;

    RIoTControl *client = new RIoTControl();
    std::list<std::string> properties;
    //client->getDeviceProperty("123-45-876","doomed");
    //client->getAvailableDevicesWrapper();
    client->getDeviceProperties("1234-123-321312",properties);
    delete client;

    return 0;
}