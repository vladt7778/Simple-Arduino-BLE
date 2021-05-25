//#define DBG_MODE ///if this is defined then the bluetooth library will listen to the USB serial instead of Bluetooth Serial

#include "Bluetooth.h"

Bluetooth receiver(11, A0, A1, 3, 10, 9600, A2);

void onBluetoothDataReceived(Packet *packet);

void onBluetoothModuleInterrupted()
{
	Serial.println("BT Module interrupted");
}

void onBluetoothModuleResumed()
{
	Serial.println("BT Module resumed");
}

void onBluetoothConnected()
{
	Serial.println("BT Module connected");
}

void onBluetoothDisconnected()
{
	Serial.println("BT Module disconnected");
}

void onBluetoothDataReceived(Packet *packet)
{
	switch (packet->GetType())
	{
	case Security::Packet::PacketType::Special:
	{
		Serial.println("BT Module: received special packet");
		break;
	}

	default:
		break;
	}
}

void setup()
{
	Serial.begin(9600);
	Serial.setTimeout(250);

	receiver.Init();
	receiver.SetConnectedHandler(onBluetoothConnected);
	receiver.SetDisconnectedHandler(onBluetoothDisconnected);
	receiver.SetReceivedHandler(onBluetoothDataReceived);
	receiver.SetModuleInterruptedHandler(onBluetoothModuleInterrupted);
	receiver.SetModuleResumedHandler(onBluetoothModuleResumed);
	receiver.SetAutoSleepMode(false);
}

void loop()
{
	receiver.Update();
}