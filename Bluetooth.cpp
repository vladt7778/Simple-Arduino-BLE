#include "Bluetooth.h"

void Bluetooth::BootSerial()
{
    if (!m_Interrupted)
    {
        m_DeviceInstance->begin(m_BaudRate);
        m_DeviceInstance->setTimeout(DEFAULT_TIMEOUT);
    }
}

void Bluetooth::CheckInterrupt()
{
    if (digitalRead(INTERRUPT_PIN_READ))
    {
        if (m_Interrupted)
        {
            m_Interrupted = false;
            onBluetoothModuleResumed();
        }
    }
    else
    {
        if (!m_Interrupted)
        {
            m_Interrupted = true;
            onBluetoothModuleInterrupted();
        }
    }
}

void Bluetooth::GetName()
{
    if (!m_Interrupted)
    {
        char nameResponse[MAX_CHARS + 1];
        memset(nameResponse, ' ', MAX_CHARS);
        nameResponse[MAX_CHARS] = 0;
        GetCharacteristics("AT+NAME?", nameResponse);
        strcpy(m_DeviceName, nameResponse + strlen("OK+NAME:"));
    }
}

void Bluetooth::GetAutoSleepMode()
{
    if (!m_Interrupted)
    {
        char sleepResponse[MAX_CHARS + 1];
        memset(sleepResponse, ' ', MAX_CHARS);
        sleepResponse[MAX_CHARS] = 0;
        GetCharacteristics("AT+PWRM?", sleepResponse);
        m_AutoSleepMode = (sleepResponse[strlen("OK+GET:")] == '0') ? true : false;
    }
}

Bluetooth::Bluetooth(uint8_t interruptPin, uint8_t interruptPinRead, uint8_t connectionPin, uint8_t rx, uint8_t tx, long baud, uint8_t wakePin) : INTERRUPT_PIN(interruptPin), INTERRUPT_PIN_READ(interruptPinRead), CONNECTION_PIN(connectionPin), WAKE_PIN(wakePin)
{
    pinMode(interruptPin, OUTPUT);
    m_DeviceInstance = new SoftwareSerial(rx, tx);
    m_BaudRate = baud;
}

Bluetooth::~Bluetooth()
{
    if (m_DeviceInstance)
    {
        delete m_DeviceInstance;
    }
    ClearBuffer();
}

bool Bluetooth::AutoSleepMode() const
{
    return m_AutoSleepMode;
}

bool Bluetooth::BufferMode() const
{
    return m_BufferMode;
}

bool Bluetooth::Connected() const
{
    return m_Connected;
}

bool Bluetooth::Interrupted() const
{
    return m_Interrupted;
}

bool Bluetooth::Listening() const
{
    return m_Listening;
}

bool Bluetooth::SleepMode() const
{
    return m_SleepMode;
}

const char *Bluetooth::DeviceName() const
{
    return m_DeviceName;
}

long Bluetooth::BaudRate() const
{
    return m_BaudRate;
}

uint8_t Bluetooth::BufferLenght() const
{
    return m_BufferLength;
}

void Bluetooth::CheckConnection()
{
    if (!m_Interrupted)
    {
        if (digitalRead(CONNECTION_PIN))
        {
            if (!m_Connected)
            {
                m_Connected = true;
                onBluetoothConnected();
            }
        }
        else
        {
            if (m_Connected)
            {
                m_Connected = false;
                onBluetoothDisconnected();
            }
        }
    }
}

void Bluetooth::ClearBuffer()
{
    if (m_BufferMode)
    {
        if (m_Buffer)
        {
            delete[] m_Buffer;
            m_Buffer = nullptr;
        }
    }
}

void Bluetooth::ConnectModule()
{
    digitalWrite(INTERRUPT_PIN, 1);
    delay(100);
}

void Bluetooth::Debug()
{
    if (m_DeviceInstance->available())
    {
        Serial.println(m_DeviceInstance->readString());
    }

    if (Serial.available())
    {
        m_DeviceInstance->write(Serial.read());
    }
}

void Bluetooth::DisconnectModule()
{
    digitalWrite(INTERRUPT_PIN, 0);
    delay(50);
}

void Bluetooth::FactoryReset()
{
    if (!m_Interrupted)
    {
        m_DeviceInstance->println("AT+RENEW");
        delay(10);
        SetPassword(DEFAULT_PASS);
        m_DeviceInstance->println("AT+MODE0"); // transmission mode
        m_DeviceInstance->println("AT+ROLE0"); //receiver
        m_DeviceInstance->println("AT+PIO11"); // pin state: low - disconnected, high - connected
        m_DeviceInstance->println("AT+NOTI1"); //notify sender
        m_DeviceInstance->println("AT+NOTP1"); //notify includes address
        SetName(DEFAULT_NAME);
        Reset();
    }
}

void Bluetooth::GetCharacteristics(const char *characteristics, char *value)
{
    if (!m_Interrupted)
    {
        m_DeviceInstance->println(characteristics);
        size_t bufferSize = m_DeviceInstance->readBytes(value, strlen(value) - 1);
        value[bufferSize] = '\0';
    }
    else
    {
        value[1] = 0;
    }
}

void Bluetooth::Init()
{
    if (!m_Initialized)
    {
        m_Initialized = true;
        Reconnect();
    }

    CheckInterrupt();
    BootSerial();

    GetAutoSleepMode();
    if (m_AutoSleepMode)
    {
        Wake();
        GetName();
        Sleep();
    }
    else
    {
        GetName();
    }
}

void Bluetooth::Listen()
{
    if (m_Interrupted)
    {
        return;
    }

    if (WAKE_PIN != -1)
    {
        if (digitalRead(WAKE_PIN))
        {
            Wake();
            return;
        }
    }

    if (!m_Connected)
    {
        return;
    }

    if (!m_Listening)
    {
        return;
    }

    Packet *receivedPacket = new Packet();
    while (m_DeviceInstance->available())
    {
        uint8_t currentByte = (uint8_t)m_DeviceInstance->read();
        if (receivedPacket->Available())
            receivedPacket->Read(currentByte);
    }

    if (receivedPacket->Read())
    {
        onBluetoothDataReceived(receivedPacket);
    }

    if (receivedPacket)
    {
        delete receivedPacket;
    }
}

void Bluetooth::onBluetoothConnected()
{
    m_SleepMode = false;

    if (*m_ConnectedHandler)
    {
        m_ConnectedHandler();
    }
}

void Bluetooth::onBluetoothModuleInterrupted()
{
    m_Connected = false;
    m_SleepMode = false;
    if (m_DeviceInstance)
    {
        m_DeviceInstance->end();
    }
    ClearBuffer();

    if (*m_ModuleInterruptedHandler)
    {
        m_ModuleInterruptedHandler();
    }
}

void Bluetooth::onBluetoothModuleResumed()
{
    Init();

    if (*m_ModuleResumedHandler)
    {
        m_ModuleResumedHandler();
    }
}

void Bluetooth::onBluetoothDisconnected()
{
    if (*m_DisconnectedHandler)
    {
        m_DisconnectedHandler();
    }
}

void Bluetooth::onBluetoothDataReceived(Packet *packet)
{
    if (*m_ReceivedHandler)
    {
        m_ReceivedHandler(packet);
    }
}

void Bluetooth::Reconnect()
{
    DisconnectModule();
    ConnectModule();
}

void Bluetooth::Reset()
{
    if (!m_Interrupted)
    {
        m_DeviceInstance->println("AT+RESET"); //restart module
        delay(10);
    }
}

void Bluetooth::SetBufferMode(bool bufferMode)
{
    if (!m_Interrupted)
    {
        m_BufferMode = bufferMode;
        if (!m_BufferMode)
        {
            ClearBuffer();
        }
    }
}

void Bluetooth::SendBuffer()
{
    if (!m_Interrupted)
    {
        if (m_BufferMode)
        {
            for (uint8_t i = 0; i < m_BufferLength; ++i)
            {
                unsigned char *packetBytes = reinterpret_cast<unsigned char *>(&m_Buffer[i]);

                m_DeviceInstance->write((const char *)packetBytes);
            }
        }
    }
}

void Bluetooth::SetCharacteristics(const char *characteristics, const char *value)
{
    if (!m_Interrupted)
    {
        bool reservedCharacteristic = false;

        size_t size = sizeof(RESERVED_CHARACTERISTICS) / sizeof(RESERVED_CHARACTERISTICS[0]);
        for (size_t i = 0; i < size; ++i)
        {
            if (!strcmp(characteristics, RESERVED_CHARACTERISTICS[i]))
            {
                reservedCharacteristic = true;
                break;
            }
        }

        if (reservedCharacteristic)
        {
            m_DeviceInstance->print(characteristics);
            m_DeviceInstance->println(value);
        }
    }
}

void Bluetooth::SetConnectedHandler(void (*handler)())
{
    m_ConnectedHandler = handler;
}

void Bluetooth::SetModuleInterruptedHandler(void (*handler)())
{
    m_ModuleInterruptedHandler = handler;
}

void Bluetooth::SetModuleResumedHandler(void (*handler)())
{
    m_ModuleResumedHandler = handler;
}

void Bluetooth::SetDisconnectedHandler(void (*handler)())
{
    m_DisconnectedHandler = handler;
}

void Bluetooth::SetName(const char *name)
{
    if (!m_Interrupted)
    {
        if (strlen(m_DeviceName) < MAX_NAME_LEN + 1)
        {
            m_DeviceInstance->print("AT+NAME");
            m_DeviceInstance->println(name);
            strcpy(m_DeviceName, name);
        }
    }
}

void Bluetooth::SetPassword(const char *password)
{
    if (!m_Interrupted)
    {
        if (strlen(password) == REQUIRED_PASS_DIGITS)
        {
            m_DeviceInstance->println("AT+TYPE2"); //pin code auth
            m_DeviceInstance->print("AT+PASS");
            m_DeviceInstance->println(password);
        }
    }
}

void Bluetooth::SetReceivedHandler(void (*handler)(Packet *))
{
    m_ReceivedHandler = handler;
}

void Bluetooth::SetAutoSleepMode(bool sleepMode)
{
    if (!m_Interrupted)
    {
        m_DeviceInstance->print("AT+PWRM");
        if (sleepMode)
        {
            m_DeviceInstance->println("0");
        }
        else
        {
            m_DeviceInstance->println("1");
        }
    }
}

void Bluetooth::Sleep()
{
    if (!m_Interrupted)
    {
        if (!m_SleepMode)
        {
            m_DeviceInstance->println("AT+SLEEP");
            m_SleepMode = true;
        }
    }
}

void Bluetooth::StartListening()
{
    if (!m_Listening)
    {
        m_Listening = true;
    }
}

void Bluetooth::StopListening()
{
    if (m_Listening)
    {
        m_Listening = false;
    }
}

void Bluetooth::Update()
{
    CheckInterrupt();
    if (!m_Interrupted)
    {
        CheckConnection();
#ifdef DBG_MODE
        Debug();
#else
        Listen();
#endif
    }
}

void Bluetooth::Wake()
{
    if (!m_Interrupted)
    {
        if (m_SleepMode || m_AutoSleepMode)
        {
            m_DeviceInstance->println("AT+WAKE");
            m_SleepMode = false;
            delay(20);
        }
    }
}

template <class T>
void Bluetooth::Send(T data, Security::Packet::PacketType packetType)
{
    if (!m_Interrupted)
    {
        unsigned char *dataBytes = reinterpret_cast<unsigned char *>(&data);

        if (m_BufferMode)
        {
            Packet *tempBuffer = new Packet[++m_BufferLength];
            memcpy(tempBuffer, m_Buffer, sizeof(Packet *) * (m_BufferLength - 1));
            ClearBuffer();
            m_Buffer = tempBuffer;
            m_Buffer[m_BufferLength - 1].Write(packetType, sizeof(data), dataBytes);
        }
        else
        {
            Packet packet;

            packet.Write(packetType, sizeof(data), dataBytes);

            unsigned char *packetBytes = reinterpret_cast<unsigned char *>(&packet);

            m_DeviceInstance->write((const char *)packetBytes);

            if (packetBytes)
            {
                delete[] packetBytes;
            }
        }
        if (dataBytes)
        {
            delete[] dataBytes;
        }
    }
}

void Bluetooth::Send(const char *data, Security::Packet::PacketType packetType)
{
    if (!m_Interrupted)
    {
        unsigned char *dataBytes = reinterpret_cast<unsigned char *>(&data);

        if (m_BufferMode)
        {
            Packet *tempBuffer = new Packet[++m_BufferLength];
            memcpy(tempBuffer, m_Buffer, sizeof(Packet *) * (m_BufferLength - 1));
            ClearBuffer();
            m_Buffer = tempBuffer;
            m_Buffer[m_BufferLength - 1].Write(packetType, strlen(data), dataBytes);
        }
        else
        {
            Packet packet;

            packet.Write(packetType, strlen(data), dataBytes);

            unsigned char *packetBytes = reinterpret_cast<unsigned char *>(&packet);

            m_DeviceInstance->write((const char *)packetBytes);

            if (packetBytes)
            {
                delete[] packetBytes;
            }
        }
        if (dataBytes)
        {
            delete[] dataBytes;
        }
    }
}

void Bluetooth::Send(char *data, Security::Packet::PacketType packetType)
{
    if (!m_Interrupted)
    {
        unsigned char *dataBytes = reinterpret_cast<unsigned char *>(&data);

        if (m_BufferMode)
        {
            Packet *tempBuffer = new Packet[++m_BufferLength];
            memcpy(tempBuffer, m_Buffer, sizeof(Packet *) * (m_BufferLength - 1));
            ClearBuffer();
            m_Buffer = tempBuffer;
            m_Buffer[m_BufferLength - 1].Write(packetType, strlen(data), dataBytes);
        }
        else
        {
            Packet packet;

            packet.Write(packetType, strlen(data), dataBytes);

            unsigned char *packetBytes = reinterpret_cast<unsigned char *>(&packet);

            m_DeviceInstance->write((const char *)packetBytes);

            if (packetBytes)
            {
                delete[] packetBytes;
            }
        }
        if (dataBytes)
        {
            delete[] dataBytes;
        }
    }
}