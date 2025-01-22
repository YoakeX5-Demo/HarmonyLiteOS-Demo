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
 *  2020-02-06 14:40  zhangqianfu  The first version
 *
 */
#ifndef _PROFILE_PACKAGE_H_
#define _PROFILE_PACKAGE_H_


///< define all the json key will be used in the profile
#define JSON_KEY_SERVICEID        "service_id"
#define JSON_KEY_PROPERTIES       "properties"
#define JSON_KEY_SERVICES         "services"



////< enum all the data type for the oc profile
typedef enum
{
    PROFILE_VALUE_INT = 0,
    PROFILE_VALUE_LONG,
    PROFILE_VALUE_FLOAT,
    PROFILE_VALUE_DOUBLE,
    PROFILE_VALUE_STRING,           ///< must be ended with '\0'
    PROFILE_VALUE_LAST,
}profile_data_t;


typedef struct
{
    void                 *nxt;   ///< ponit to the next key
    char                 *key;
    profile_data_t  type;
    void                 *value;
}profile_kv_t;


typedef struct
{
    char *request_id;                              ///< get from the topic, NULL if not supplied
    void *msg;                                     ///< the send from the cloud platform
    int   msg_len;                                 ///< the message length
}profile_msgrcv_t;

typedef struct
{
   void *nxt;
   char *service_id;                         ///< the service id in the profile, which could not be NULL
   char *event_time;                         ///< eventtime, which could be NULL means use the platform time
   profile_kv_t *service_property;           ///< the property in the profile, which could not be NULL
}profile_service_t;

typedef struct
{
   void *nxt;
   char *name;                         ///< the service id in the profile, which could not be NULL
   char *propertyName;
   char *unit;                         ///< eventtime, which could be NULL means use the platform time
}profile_property_t;

typedef struct
{
   void *nxt;
   char *name;                         ///< the service id in the profile, which could not be NULL
   char *commadName;
   char *type;
}profile_commad_t;

///< defines for the package tools

char *profile_package_propertyreport(profile_service_t *payload);
char *profile_package_property(char *service_name,profile_property_t *property,profile_commad_t *commad,char *ip,char *port);

#endif /* _PROFILE_PACKAGE_H_ */
