/*----------------------------------------------------------------------------
 * Copyright (c) <2018>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
/**
 *  DATE                AUTHOR      INSTRUCTION
 *  2020-02-06 14:36  zhangqianfu  The first version
 *
 */
////< this file used to package the data for the profile and you must make sure the data format is right

#include <profile_package.h>
#include <cJSON.h>


///< format the report data to json string mode
static cJSON  *JsonCreateKv(profile_kv_t  *kv)
{
    cJSON  *ret = NULL;
    switch (kv->type)
    {
        case PROFILE_VALUE_INT:
            ret = cJSON_CreateNumber((double)(*(int *)kv->value));
            break;
        case PROFILE_VALUE_LONG:
            ret = cJSON_CreateNumber((double)(*(long *)kv->value));
            break;
        case PROFILE_VALUE_FLOAT:
            ret = cJSON_CreateNumber((double)(*(float *)kv->value));
            break;
        case PROFILE_VALUE_DOUBLE:
            ret = cJSON_CreateNumber((*(double *)kv->value));
            break;
        case PROFILE_VALUE_STRING:
            ret = cJSON_CreateString((const char *)kv->value);
            break;
        default:
            break;
    }

    return ret;
}


static cJSON *JsonCreateKvLst(profile_kv_t *kvlst)
{

    cJSON *root;
    cJSON *kv;
    profile_kv_t  *kv_info;

    ///< build a root node
    root = cJSON_CreateObject();
    if(NULL == root)
    {
       goto EXIT_MEM;
    }

    ///< add all the property to the properties
    kv_info = kvlst;
    while(NULL != kv_info)
    {
        kv = JsonCreateKv( kv_info);
        if(NULL == kv)
        {
            goto EXIT_MEM;
        }

        cJSON_AddItemToObject(root,kv_info->key,kv);
        kv_info = kv_info->nxt;
    }

    ///< OK, now we return it
    return root;

EXIT_MEM:
    if(NULL != root)
    {
       cJSON_Delete(root);
       root = NULL;
    }
    return root;


}

static cJSON *JsonCreateService(profile_service_t *service_info)
{

    cJSON *root;
    cJSON *service_id;
    cJSON *properties;
    cJSON *event_time;

    ///< build a root node
    root = cJSON_CreateObject();
    if(NULL == root)
    {
       goto EXIT_MEM;
    }

    ///< add the service_id node to the root node
    service_id = cJSON_CreateString(service_info->service_id);
    if(NULL == service_id)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,JSON_KEY_SERVICEID,service_id);

    ///< add the properties node to the root
    properties = JsonCreateKvLst(service_info->service_property);
    if(NULL == properties)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,JSON_KEY_PROPERTIES,properties);


    ///< OK, now we return it
    return root;

EXIT_MEM:
    if(NULL != root)
    {
       cJSON_Delete(root);
       root = NULL;
    }
    return root;
}

static cJSON *JsonCreateServices(profile_service_t *service_info)
{
    cJSON *services = NULL;
    cJSON *service;
    profile_service_t  *service_tmp;

    ///< create the services array node
    services = cJSON_CreateArray();
    if(NULL == services)
    {
       goto EXIT_MEM;
    }

    service_tmp = service_info;
    while(NULL != service_tmp)
    {
        service = JsonCreateService(service_tmp);
        if(NULL == service)
        {
            goto EXIT_MEM;
        }

        cJSON_AddItemToArray(services,service);
        service_tmp = service_tmp->nxt;
    }

    ///< now we return the services
    return services;

EXIT_MEM:
    if(NULL != services)
    {
       cJSON_Delete(services);
       services = NULL;
    }
    return services;
}

char *profile_package_propertyreport(profile_service_t *payload)
{
    char *ret = NULL;
    cJSON *root;
    cJSON *services;

    ///< create the root node
    root = cJSON_CreateObject();
    if(NULL == root)
    {
       goto EXIT_MEM;
    }

    ///< create the services array node to the root
    services = JsonCreateServices(payload);
    if(NULL == services)
    {
        goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,JSON_KEY_SERVICES,services);

    ///< OK, now we make it to a buffer
    ret = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return ret;

EXIT_MEM:
    if(NULL != root)
    {
       cJSON_Delete(root);
    }
    return ret;
}


static cJSON *JsonCreateProperty(profile_property_t *property_info)
{

    cJSON *root;
    cJSON *name;
    cJSON *propertyname;
    cJSON *unit;


    ///< build a root node
    root = cJSON_CreateObject();
    if(NULL == root)
    {
       goto EXIT_MEM;
    }

    ///< add the service_id node to the root node
    name = cJSON_CreateString(property_info->name);
    if(NULL == name)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"name",name);

    
    ///< add the service_id node to the root node
    propertyname = cJSON_CreateString(property_info->propertyName);
    if(NULL == propertyname)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"propertyName",propertyname);

    ///< add the service_id node to the root node
    unit= cJSON_CreateString(property_info->unit);
    if(NULL == unit)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"unit",unit);


    ///< OK, now we return it
    return root;

EXIT_MEM:
    if(NULL != root)
    {
       cJSON_Delete(root);
       root = NULL;
    }
    return root;
}

static cJSON *JsonCreateProperties(profile_property_t *property_info)
{
    cJSON *properties = NULL;
    cJSON *property;
    profile_property_t  *property_tmp;

    ///< create the services array node
    properties = cJSON_CreateArray();
    if(NULL == properties)
    {
       goto EXIT_MEM;
    }

    property_tmp = property_info;
    while(NULL != property_tmp)
    {
        property = JsonCreateProperty(property_tmp);
        if(NULL == property)
        {
            goto EXIT_MEM;
        }

        cJSON_AddItemToArray(properties,property);
        property_tmp = property_tmp->nxt;
    }

    ///< now we return the services
    return properties;

EXIT_MEM:
    if(NULL != properties)
    {
       cJSON_Delete(properties);
       properties = NULL;
    }
    return properties;
}

static cJSON *JsonCreateCommand(profile_commad_t *command_info)
{

    cJSON *root;
    cJSON *name;
    cJSON *paraName;
    cJSON *type;


    ///< build a root node
    root = cJSON_CreateObject();
    if(NULL == root)
    {
       goto EXIT_MEM;
    }

    ///< add the service_id node to the root node
    name = cJSON_CreateString(command_info->name);
    if(NULL == name)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"name",name);

    
    ///< add the service_id node to the root node
    paraName = cJSON_CreateString(command_info->commadName);
    if(NULL == paraName)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"paraName",paraName);

    ///< add the service_id node to the root node
    type= cJSON_CreateString(command_info->type);
    if(NULL == type)
    {
       goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"type",type);


    ///< OK, now we return it
    return root;

EXIT_MEM:
    if(NULL != root)
    {
       cJSON_Delete(root);
       root = NULL;
    }
    return root;
}

static cJSON *JsonCreateCommands(profile_commad_t *commad_info)
{
    cJSON *commands = NULL;
    cJSON *command;
    profile_commad_t  *command_tmp;

    ///< create the services array node
    commands = cJSON_CreateArray();
    if(NULL == commands)
    {
       goto EXIT_MEM;
    }

    command_tmp = commad_info;
    while(NULL != command_tmp)
    {
        command = JsonCreateCommand(command_tmp);
        if(NULL == command)
        {
            goto EXIT_MEM;
        }

        cJSON_AddItemToArray(commands,command);
        command_tmp = command_tmp->nxt;
    }

    ///< now we return the services
    return commands;

EXIT_MEM:
    if(NULL != commands)
    {
       cJSON_Delete(commands);
       commands = NULL;
    }
    return commands;
}

char *profile_package_property(char *service_name,profile_property_t *property,profile_commad_t *commad,char *ip,char *port)
{
    char*ret = NULL;
    cJSON *root;
    cJSON *service;
    cJSON *properties;
    cJSON *commads;
    cJSON *ipAddress;
    cJSON *Port;

    ///< create the root node
    root = cJSON_CreateObject();
    if(NULL == root)
    {
       goto EXIT_MEM;
    }

    service = cJSON_CreateString(service_name);
    cJSON_AddItemToObjectCS(root,"service",service);

    ///< create the properties array node to the root
    properties = JsonCreateProperties(property);
    if(NULL == properties)
    {
        goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"property",properties);

    ///< create the properties array node to the root
    commads = JsonCreateCommands(commad);
    if(NULL == commads)
    {
        goto EXIT_MEM;
    }
    cJSON_AddItemToObjectCS(root,"command",commads);

    ipAddress = cJSON_CreateString(ip);
    cJSON_AddItemToObjectCS(root,"ipAddress",ipAddress);

    Port = cJSON_CreateString(port);
    cJSON_AddItemToObjectCS(root,"port",Port);


    ///< OK, now we make it to a buffer
    ret = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return ret;

EXIT_MEM:
    if(NULL != root)
    {
       cJSON_Delete(root);
    }
    return ret;
}



