#pragma once
#include <SoftwareSerial.h>
#include "Packet.h"

class Bluetooth
{
public:
    constexpr static uint8_t REQUIRED_PASS_DIGITS = 6;
    constexpr static uint8_t MAX_NAME_LEN = 12;
    constexpr static uint8_t MAX_CHARS = 80;

    constexpr static ulong DEFAULT_TIMEOUT = 250;

    constexpr static const char *DEFAULT_NAME = "DEFAULT_BT";
    constexpr static const char *DEFAULT_PASS = "997788";
    constexpr static const char *RESERVED_CHARACTERISTICS[] = {"AT+NAME", "AT+PASS", "AT+PWRM", "AT+SLEEP"};

    const uint8_t CONNECTION_PIN;
    const uint8_t INTERRUPT_PIN;
    const uint8_t INTERRUPT_PIN_READ;
    const uint8_t WAKE_PIN;

    char m_DeviceName[MAX_NAME_LEN + 1];

    uint8_t m_BufferLength = 0;

    SoftwareSerial *m_DeviceInstance = nullptr;
    Packet *m_Buffer = nullptr;

    long m_BaudRate;

    bool m_AutoSleepMode = false;
    bool m_BufferMode = false;
    bool m_Connected = false;
    bool m_Initialized = false;
    bool m_Interrupted = true;
    bool m_Listening = true;
    bool m_SleepMode = false;

    void (*m_ConnectedHandler)();
    void (*m_ModuleInterruptedHandler)();
    void (*m_DisconnectedHandler)();
    void (*m_ModuleResumedHandler)();
    void (*m_ReceivedHandler)(Packet *);

    void BootSerial();
    void CheckInterrupt();
    void GetName();
    void GetAutoSleepMode();

public:
    Bluetooth(uint8_t interruptPin, uint8_t interruptPinRead, uint8_t connectionPin, uint8_t rx, uint8_t tx, long baud, uint8_t wakePin = -1);
    ~Bluetooth();

    bool AutoSleepMode() const;
    bool BufferMode() const;
    bool Connected() const;
    bool Interrupted() const;
    bool Listening() const;
    bool SleepMode() const;

    const char *DeviceName() const;

    long BaudRate() const;

    uint8_t BufferLenght() const;

    void CheckConnection();
    void ClearBuffer();
    void ConnectModule();
    void Debug();
    void DisconnectModule();
    void FactoryReset();
    void GetCharacteristics(const char *characteristics, char *val);
    void Init();
    void Listen();
    void onBluetoothConnected();
    void onBluetoothModuleInterrupted();
    void onBluetoothModuleResumed();
    void onBluetoothDisconnected();
    void onBluetoothDataReceived(Packet *packet);
    void Reconnect();
    void Reset();
    void SetBufferMode(bool bufferMode);
    void SendBuffer();
    void SetCharacteristics(const char *characteristics, const char *value);
    void SetConnectedHandler(void (*handler)());
    void SetModuleInterruptedHandler(void (*handler)());
    void SetModuleResumedHandler(void (*handler)());
    void SetDisconnectedHandler(void (*handler)());
    void SetName(const char *name);
    void SetPassword(const char *password);
    void SetReceivedHandler(void (*handler)(Packet *packet));
    void SetAutoSleepMode(bool mode);
    void Sleep();
    void StartListening();
    void StopListening();
    void Update();
    void Wake();

    template <class T>
    void Send(T data, Security::Packet::PacketType packetType = Security::Packet::PacketType::Default);
    void Send(const char *data, Security::Packet::PacketType packetType = Security::Packet::PacketType::Default);
    void Send(char *data, Security::Packet::PacketType packetType = Security::Packet::PacketType::Default);
};