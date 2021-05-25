#pragma once
#include "Arduino.h"

namespace Security
{
    namespace Packet
    {
        constexpr uint8_t PACKET_HEADER = 0x100;

        enum PacketType
        {
            Default = 0x123,
            Special = 0x567
        };
    }
}