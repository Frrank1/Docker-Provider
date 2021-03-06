/* @migen@ */
#include <MI.h>
#include "Container_HostInventory_Class_Provider.h"

#include <vector>
#include <string>
#include <syslog.h>
#include <stdio.h>
#include <uuid/uuid.h>
#include <cstdlib>

#include "../cjson/cJSON.h"
#include "../dockerapi/DockerRemoteApi.h"
#include "../dockerapi/DockerRestHelper.h"
#include "Helper.h"

using namespace std;

MI_BEGIN_NAMESPACE

class ContainerHostInventoryQuery
{
public:
    ///
    /// \returns Object representing the host
    ///
    static Container_HostInventory_Class InspectHost()
    {
        Container_HostInventory_Class hostInventoryInstance;

        // Request containers
        vector<string> request(1, DockerRestHelper::restDockerInfo());
        vector<cJSON*> response;

        //check to see if its a test environment
        if(getenv(DOCKER_TESTRUNNER_STRING) != NULL)
        {
            //fake response for test
            char* testResponseStr = getenv(DOCKER_TESTRUNNER_STRING);
            string tempStr = testResponseStr;
            cJSON* testJsonObject = parseJson(tempStr);
            response.push_back(testJsonObject);
        }
        else
        {
            //regular code path, get response from rest call
            response = getResponse(request);
        }

        // See https://docs.docker.com/engine/api/v1.27/#section/Versioning for example output
        if (!response.empty() && response[0])
        {   
            PopulateInstanceFromJson(hostInventoryInstance, response[0]);
            // Clean up object
            cJSON_Delete(response[0]);
        }
        else
        {
            syslog(LOG_WARNING, "API call in Container_HostInventory::InspectHost to get host info returned null");
        }
        return hostInventoryInstance;
    }

    static void PopulateInstanceFromJson(Container_HostInventory_Class &instance, cJSON* responseJson)
    {
            instance.InstanceID_value(Guid::NewToString().c_str());
            string computerName = string((cJSON_GetObjectItem(responseJson, "Name")->valuestring));
            instance.Computer_value(computerName.c_str());
            instance.DockerVersion_value((cJSON_GetObjectItem(responseJson, "ServerVersion")->valuestring));
            instance.OperatingSystem_value((cJSON_GetObjectItem(responseJson, "OperatingSystem")->valuestring));
            //Get Volume and Network info from "Plugins"
            cJSON* plugins_entry = cJSON_GetObjectItem(responseJson, "Plugins");
            if(plugins_entry != NULL)
            {
                //Get volume info
                cJSON* volumeEntry = cJSON_GetObjectItem(plugins_entry, "Volume");
                string volumeList;
                if(volumeEntry != NULL)
                {
                    for(int i =0; i < cJSON_GetArraySize(volumeEntry);i++)
                    {
                        volumeList.append(string(cJSON_GetArrayItem(volumeEntry, i)->valuestring));
                    }
                }
                instance.Volume_value(volumeList.c_str());
                //Get network info
                cJSON* networkEntry = cJSON_GetObjectItem(plugins_entry, "Network");
                string networkList;
                if(networkEntry != NULL)
                {
                    for(int i =0; i < cJSON_GetArraySize(networkEntry);i++)
                    {
                        networkList.append(string(cJSON_GetArrayItem(networkEntry, i)->valuestring));
                        networkList.append(" ");
                    }
                }
                instance.Network_value(networkList.c_str());
            }

            //Node role is obtained from the computer name. e.g. k8s-master-71E8D996-0 => master role
            if(computerName.find("master") != string::npos || computerName.find("manager") != string::npos)
            {
                //swarm
                instance.NodeRole_value("Master");  
            }
            else if(computerName.find("agent") != string::npos)
            {
                //swarm
                instance.NodeRole_value("Agent");  
            }
            else
            {
                instance.NodeRole_value("Not Orchestrated");  
            }   

            //Orchestrator type is obtained from the node name. example of node names swarm-manager-vmss_0 for swarm, k8s-master-71E8D996-0 for kubernetes
            if(computerName.find("swarmm") != string::npos)
            {
                //swarm
                instance.OrchestratorType_value("Swarm Mode");  
            }
            else if(computerName.find("swarm") != string::npos)
            {
                //swarm
                instance.OrchestratorType_value("Swarm");  
            }
            else if(computerName.find("k8") != string::npos)
            {
                //swarm
                instance.OrchestratorType_value("Kubernetes");  
            }
            else if(computerName.find("dcos") != string::npos)
            {
                //swarm
                instance.OrchestratorType_value("DC/OS");  
            }
            else
            {
                instance.OrchestratorType_value("None");  
            }   
    }
};

Container_HostInventory_Class_Provider::Container_HostInventory_Class_Provider(
    Module* module) :
    m_Module(module)
{
}

Container_HostInventory_Class_Provider::~Container_HostInventory_Class_Provider()
{
}

void Container_HostInventory_Class_Provider::Load(
        Context& context)
{
    context.Post(MI_RESULT_OK);
}

void Container_HostInventory_Class_Provider::Unload(
        Context& context)
{
    context.Post(MI_RESULT_OK);
}

void Container_HostInventory_Class_Provider::EnumerateInstances(
    Context& context,
    const String& nameSpace,
    const PropertySet& propertySet,
    bool keysOnly,
    const MI_Filter* filter)
{
    try
    {
        Container_HostInventory_Class queryResult = ContainerHostInventoryQuery::InspectHost();
        context.Post(queryResult);
        context.Post(MI_RESULT_OK);
    }
    catch (std::exception &e)
    {
        syslog(LOG_ERR, "Container_HostInventory %s", e.what());
        context.Post(MI_RESULT_FAILED);
    }
    catch (...)
    {
        syslog(LOG_ERR, "Container_HostInventory Unknown exception");
        context.Post(MI_RESULT_FAILED);
    }
}

void Container_HostInventory_Class_Provider::GetInstance(
    Context& context,
    const String& nameSpace,
    const Container_HostInventory_Class& instanceName,
    const PropertySet& propertySet)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void Container_HostInventory_Class_Provider::CreateInstance(
    Context& context,
    const String& nameSpace,
    const Container_HostInventory_Class& newInstance)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void Container_HostInventory_Class_Provider::ModifyInstance(
    Context& context,
    const String& nameSpace,
    const Container_HostInventory_Class& modifiedInstance,
    const PropertySet& propertySet)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}

void Container_HostInventory_Class_Provider::DeleteInstance(
    Context& context,
    const String& nameSpace,
    const Container_HostInventory_Class& instanceName)
{
    context.Post(MI_RESULT_NOT_SUPPORTED);
}


MI_END_NAMESPACE

