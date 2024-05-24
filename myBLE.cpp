// myBLE.cpp
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "myBLE.hpp"

BLEHandler* BLEHandler::instance = nullptr;

BLEHandler::BLEHandler(const std::string& deviceName, const std::string& serviceUUID, const std::string& characteristicUUID)
    : deviceName(deviceName), serviceUUID(serviceUUID), characteristicUUID(characteristicUUID), isConnected(false) {}

void BLEHandler::init() {
    instance = this;

    BLEDevice::init(deviceName);

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());

    pService = pServer->createService(serviceUUID);

    pCharacteristic = pService->createCharacteristic(
                          characteristicUUID,
                          BLECharacteristic::PROPERTY_READ |
                          BLECharacteristic::PROPERTY_NOTIFY
                      );

    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();
    pServer->getAdvertising()->start();
}

void BLEHandler::notify(const std::string& value) {
    if (isConnected) {
        pCharacteristic->setValue(value);
        pCharacteristic->notify();
    }
}

bool BLEHandler::getIsConnected() {
    return isConnected;
}

void BLEHandler::ServerCallbacks::onConnect(BLEServer* pServer) {
    BLEHandler* handler = BLEHandler::instance;
    handler->isConnected = true;
}

void BLEHandler::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    BLEHandler* handler = BLEHandler::instance;
    handler->isConnected = false;
}
