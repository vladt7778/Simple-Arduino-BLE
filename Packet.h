#pragma once
#include "Security.h"

class Packet
{
    constexpr static uint8_t HEADER = Security::Packet::PACKET_HEADER;

    bool m_Available = true;
    bool m_Readable = false;
    bool m_Read = false;

    uint8_t *m_Buffer = nullptr;
    uint8_t m_CurrentByte = 0;
    uint8_t m_Length;
    uint8_t m_Type;

public:
    ~Packet();

    bool Available() const;
    bool Read() const;

    uint8_t *GetData() const;
    uint8_t GetLength() const;
    uint8_t GetType() const;

    void Read(uint8_t byte);
    void Write(uint8_t type, uint8_t length, uint8_t *buffer);
};