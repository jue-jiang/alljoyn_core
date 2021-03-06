#ifndef _STUNATTRIBUTEALLOCATEDXORSERVERREFLEXIVEADDRESS_H
#define _STUNATTRIBUTEALLOCATEDXORSERVERREFLEXIVEADDRESS_H
/**
 * @file
 *
 * This file defines the ALLOCATED-XOR-SERVER-REFLEXIVE-ADDRESS  STUN message attribute.
 */

/******************************************************************************
 * Copyright 2012 Qualcomm Innovation Center, Inc.
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
 ******************************************************************************/

#ifndef __cplusplus
#error Only include attr/StunAttributeAllocatedXorServerReflexiveAddress.h in C++ code.
#endif


#include <string>
#include <qcc/platform.h>
#include <StunAttributeXorMappedAddress.h>
#include <types.h>
#include "Status.h"

using namespace qcc;

/** @internal */
#define QCC_MODULE "STUN_ATTRIBUTE"

/**
 * Allocated XOR Server Reflexive Address STUN attribute class.
 */
class StunAttributeAllocatedXorServerReflexiveAddress : public StunAttributeXorMappedAddress {
  public:
    /**
     * This constructor just sets the attribute type to STUN_ATTR_ALLOCATED_XOR_SERVER_REFLEXIVE_ADDRESS.
     *
     * @param msg Reference to the containing message.
     */
    StunAttributeAllocatedXorServerReflexiveAddress(const StunMessage& msg) :
        StunAttributeXorMappedAddress(STUN_ATTR_ALLOCATED_XOR_SERVER_REFLEXIVE_ADDRESS,
                                      "ALLOCATED-XOR-SERVER-REFLEXIVE-ADDRESS",
                                      msg)
    { }

    /**
     * This constructor just sets the attribute type to STUN_ATTR_ALLOCATED_XOR_SERVER_REFLEXIVE_ADDRESS
     * and initializes the IP address and port.
     *
     * @param msg Reference to the containing message.
     * @param addr  IP Address.
     * @param port  IP Port.
     */
    StunAttributeAllocatedXorServerReflexiveAddress(const StunMessage& msg, const IPAddress& addr, uint16_t port) :
        StunAttributeXorMappedAddress(STUN_ATTR_ALLOCATED_XOR_SERVER_REFLEXIVE_ADDRESS,
                                      "ALLOCATED-XOR-SERVER-REFLEXIVE-ADDRESS",
                                      msg, addr, port)
    { }

    static const uint16_t ATTR_SIZE = sizeof(uint8_t) +   // Unused octet.
                                      sizeof(uint8_t) +                                       // Address family.
                                      sizeof(uint16_t) +                                      // Port.
                                      sizeof(IPAddress);                                      // IP Address.

    static const uint16_t ATTR_SIZE_WITH_HEADER = ((StunAttribute::ATTR_HEADER_SIZE + ATTR_SIZE + 3) & 0xfffc);
};


#undef QCC_MODULE
#endif
