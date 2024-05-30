// myBLE.hpp
#ifndef myBLE_H
#define myBLE_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class BLEHandler {
  private:
    BLEServer* pServer;
    BLEService* pService;
    BLECharacteristic* pCharacteristic;
    bool isConnected;
    std::string deviceName;
    std::string serviceUUID;
    std::string characteristicUUID;

    class ServerCallbacks : public BLEServerCallbacks {
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    };

  public:
    static BLEHandler* instance;

    BLEHandler(const std::string& deviceName, const std::string& serviceUUID, const std::string& characteristicUUID);

    void init();
    void notify(const std::string& value);
    bool getIsConnected();
};

#endif
