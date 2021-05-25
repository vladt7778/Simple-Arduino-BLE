#include "Packet.h"

Packet::~Packet()
{
    if (m_Buffer)
    {
        delete[] m_Buffer;
    }
}

bool Packet::Available() const
{
    return m_Available;
}

bool Packet::Read() const
{
    return m_Read;
}

uint8_t Packet::GetType() const
{
    return m_Type;
}

uint8_t Packet::GetLength() const
{
    return m_Length;
}

uint8_t *Packet::GetData() const
{
    return m_Buffer;
}

void Packet::Read(uint8_t byte)
{
    delayMicroseconds(160);

    if (byte == HEADER)
    {
        if (m_CurrentByte)
        {
            return;
        }
        m_Readable = true;
    }
    else
    {
        if (m_Readable)
        {
            switch (m_CurrentByte)
            {
            case 1:
            {
                m_Type = byte;
                break;
            }

            case 2:
            {
                m_Length = byte;
                if (m_Length > 0)
                {
                    m_Buffer = new uint8_t[m_Length];
                }
                else
                {
                    m_Available = false;
                    m_Read = true;
                }
                break;
            }

            default:
            {
                if (m_CurrentByte - 3 < m_Length)
                {
                    m_Buffer[m_CurrentByte - 3] = byte;
                }
                else
                {
                    m_Available = false;
                    m_Read = true;
                }
            }
            }
        }
    }
    ++m_CurrentByte;
}

void Packet::Write(uint8_t type, uint8_t length, uint8_t *buffer)
{
    m_Type = type;
    m_Length = length;
    m_Buffer = new uint8_t[m_Length];
    memcpy(m_Buffer, buffer, sizeof(uint8_t) * m_Length);
}