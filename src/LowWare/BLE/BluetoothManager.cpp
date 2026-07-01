// #include "BluetoothManager.h"
// #include <cstdarg>
// #include <cstdio>
//
// // ─────────────────────────────────────────────────────────────────────────────
// //  ═══════════════  BLE IMPLEMENTATION  ═══════════════
// // ─────────────────────────────────────────────────────────────────────────────
// #ifdef BT_MODE_BLE
//
// BLEManager::BLEManager(const char *deviceName, BLERole role) : _name(deviceName), _role(role) {}
//
// void BLEManager::begin() {
//     NimBLEDevice::init(_name);
//     NimBLEDevice::setPower(ESP_PWR_LVL_P9); // puissance max (+9 dBm)
//     NimBLEDevice::setSecurityAuth(false, false, true); // bonding sans MITM
//
//     if (_role == BLERole::PERIPHERAL) {
//         _initPeripheral();
//     } else {
//         _initCentral();
//     }
// }
//
// void BLEManager::end() {
//     NimBLEDevice::deinit(true);
//     _connected = false;
//     _rxBuf.clear();
// }
//
// // ── PERIPHERAL ───────────────────────────────────────────────────────────────
//
// void BLEManager::_initPeripheral() {
//     _server = NimBLEDevice::createServer();
//     _server->setCallbacks(this);
//     _server->advertiseOnDisconnect(true);
//
//     NimBLEService *nus = _server->createService(NUS_SERVICE_UUID);
//
//     // TX characteristic : Notify (device → host)
//     _txChar = nus->createCharacteristic(NUS_TX_CHARACTERISTIC, NIMBLE_PROPERTY::NOTIFY);
//
//     // RX characteristic : Write (host → device)
//     _rxChar = nus->createCharacteristic(NUS_RX_CHARACTERISTIC, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
//     _rxChar->setCallbacks(this);
//
//     nus->start();
//
//     NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
//     adv->addServiceUUID(NUS_SERVICE_UUID);
//     adv->setScanResponse(true);
//     adv->start();
// }
//
// void BLEManager::_initCentral() {
//     _scanner = NimBLEDevice::getScan();
//     _scanner->setScanCallbacks(this, false);
//     _scanner->setActiveScan(true);
//     _scanner->setInterval(100);
//     _scanner->setWindow(99);
// }
//
// void BLEManager::restartAdvertising() {
//     if (_role == BLERole::PERIPHERAL) {
//         NimBLEDevice::getAdvertising()->start();
//     }
// }
//
// void BLEManager::setManufacturerData(const uint8_t *data, size_t len) {
//     std::string mfr(reinterpret_cast<const char *>(data), len);
//     NimBLEDevice::getAdvertising()->setManufacturerData(mfr);
// }
//
// uint16_t BLEManager::getMTU() const {
//     if (_role == BLERole::PERIPHERAL && _server && _connected) {
//         auto peers = _server->getPeerDevices();
//         if (!peers.empty())
//             return _server->getPeerMTU(peers[0]);
//     }
//     if (_role == BLERole::CENTRAL && _client)
//         return _client->getMTU();
//     return 23; // MTU par défaut
// }
//
// // ── Envoi / réception ────────────────────────────────────────────────────────
//
// size_t BLEManager::write(const uint8_t *data, size_t len) {
//     if (!_connected)
//         return 0;
//     if (_role == BLERole::PERIPHERAL && _txChar) {
//         _txChar->setValue(data, len);
//         _txChar->notify();
//         return len;
//     }
//     if (_role == BLERole::CENTRAL && _remRxChar) {
//         return _remRxChar->writeValue(data, len, false) ? len : 0;
//     }
//     return 0;
// }
//
// size_t BLEManager::write(const char *str) { return write(reinterpret_cast<const uint8_t *>(str), strlen(str)); }
//
// size_t BLEManager::println(const char *str) {
//     std::string s(str);
//     s += "\r\n";
//     return write(reinterpret_cast<const uint8_t *>(s.c_str()), s.length());
// }
//
// size_t BLEManager::printf(const char *fmt, ...) {
//     char buf[256];
//     va_list args;
//     va_start(args, fmt);
//     int n = vsnprintf(buf, sizeof(buf), fmt, args);
//     va_end(args);
//     if (n > 0)
//         return write(reinterpret_cast<const uint8_t *>(buf), static_cast<size_t>(n));
//     return 0;
// }
//
// int BLEManager::read() {
//     if (_rxBuf.empty())
//         return -1;
//     int c = static_cast<uint8_t>(_rxBuf[0]);
//     _rxBuf.erase(0, 1);
//     return c;
// }
//
// std::string BLEManager::readString() {
//     std::string s = _rxBuf;
//     _rxBuf.clear();
//     return s;
// }
//
// // ── Scan + Connexion CENTRAL ──────────────────────────────────────────────────
//
// void BLEManager::scan(uint32_t durationSec, std::function<void(const BTDevice &)> onFound) {
//     _scanCb = onFound;
//     NimBLEScanResults results = _scanner->getResults(durationSec * 1000, false);
//     (void) results;
// }
//
// void BLEManager::onResult(const NimBLEAdvertisedDevice *device) {
//     if (_scanCb) {
//         BTDevice d;
//         d.name = device->getName();
//         d.address = device->getAddress().toString();
//         d.rssi = device->getRSSI();
//         d.connectable = device->isAdvertisingType(BLE_HCI_ADV_TYPE_ADV_IND) ||
//                         device->isAdvertisingType(BLE_HCI_ADV_TYPE_ADV_DIRECT_IND_HD);
//         _scanCb(d);
//     }
// }
//
// bool BLEManager::connect(const std::string &address) {
//     NimBLEAddress bleAddr(address);
//     _client = NimBLEDevice::createClient();
//     _client->setClientCallbacks(this, false);
//     _client->setConnectionParams(12, 12, 0, 51);
//     _client->setConnectTimeout(10);
//
//     if (!_client->connect(bleAddr)) {
//         NimBLEDevice::deleteClient(_client);
//         _client = nullptr;
//         return false;
//     }
//
//     NimBLERemoteService *svc = _client->getService(NUS_SERVICE_UUID);
//     if (!svc) {
//         disconnect();
//         return false;
//     }
//
//     _remTxChar = svc->getCharacteristic(NUS_TX_CHARACTERISTIC);
//     _remRxChar = svc->getCharacteristic(NUS_RX_CHARACTERISTIC);
//
//     if (!_remTxChar || !_remRxChar) {
//         disconnect();
//         return false;
//     }
//
//     // Souscription aux notifications TX
//     if (_remTxChar->canNotify()) {
//         _remTxChar->subscribe(true, [this](NimBLERemoteCharacteristic *c, uint8_t *data, size_t len, bool) {
//             _rxBuf.append(reinterpret_cast<char *>(data), len);
//             if (_onReceiveCb)
//                 _onReceiveCb(_rxBuf);
//         });
//     }
//
//     _connected = true;
//     if (_onConnectCb)
//         _onConnectCb();
//     return true;
// }
//
// void BLEManager::disconnect() {
//     if (_client) {
//         _client->disconnect();
//         NimBLEDevice::deleteClient(_client);
//         _client = nullptr;
//     }
//     _connected = false;
// }
//
// // ── NimBLE Callbacks ─────────────────────────────────────────────────────────
//
// void BLEManager::onConnect(NimBLEServer *s) {
//     _connected = true;
//     if (_onConnectCb)
//         _onConnectCb();
// }
//
// void BLEManager::onDisconnect(NimBLEServer *s, NimBLEConnInfo &info, int reason) {
//     _connected = false;
//     if (_onDisconnectCb)
//         _onDisconnectCb();
// }
//
// void BLEManager::onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) {
//     std::string val = c->getValue();
//     _rxBuf += val;
//     if (_onReceiveCb)
//         _onReceiveCb(val);
// }
//
// void BLEManager::onConnect(NimBLEClient *c) {
//     _connected = true;
//     if (_onConnectCb)
//         _onConnectCb();
// }
//
// void BLEManager::onDisconnect(NimBLEClient *c, int reason) {
//     _connected = false;
//     if (_onDisconnectCb)
//         _onDisconnectCb();
// }
//
// #endif // BT_MODE_BLE
//
//
// // ─────────────────────────────────────────────────────────────────────────────
// //  ═══════════════  CLASSIC IMPLEMENTATION  ═══════════════
// // ─────────────────────────────────────────────────────────────────────────────
// #ifdef BT_MODE_CLASSIC
//
// BTClassicManager::BTClassicManager(const char *deviceName, BTClassicRole role, const char *pin) :
//     _name(deviceName), _role(role), _pin(pin) {}
//
// void BTClassicManager::begin() {
//     bool isMaster = (_role == BTClassicRole::MASTER);
//     _bt.begin(_name, isMaster);
//     _bt.setPin(_pin);
//
//     _bt.onConfirmRequest([](uint32_t numVal) { Serial.printf("[BT] Confirm pairing code: %06u\n", numVal); });
//
//     _bt.register_callback([this](esp_spp_cb_event_t evt, esp_spp_cb_param_t *param) {
//         if (evt == ESP_SPP_SRV_OPEN_EVT || evt == ESP_SPP_OPEN_EVT) {
//             BTAddress addr(param->srv_open.rem_bda);
//             if (_onConnectCb)
//                 _onConnectCb(addr);
//         }
//         if (evt == ESP_SPP_CLOSE_EVT) {
//             BTAddress addr(param->close.rem_bda);
//             if (_onDisconnectCb)
//                 _onDisconnectCb(addr);
//         }
//     });
//
//     _ready = true;
// }
//
// void BTClassicManager::end() {
//     _bt.end();
//     _ready = false;
// }
//
// bool BTClassicManager::isConnected() const { return _bt.connected(); }
//
// // ── API série ─────────────────────────────────────────────────────────────────
//
// size_t BTClassicManager::write(const uint8_t *data, size_t len) {
//     return static_cast<size_t>(_bt.write(data, static_cast<int>(len)));
// }
//
// size_t BTClassicManager::write(const char *str) { return _bt.print(str); }
//
// size_t BTClassicManager::println(const char *str) { return _bt.println(str); }
//
// size_t BTClassicManager::printf(const char *fmt, ...) {
//     char buf[256];
//     va_list args;
//     va_start(args, fmt);
//     int n = vsnprintf(buf, sizeof(buf), fmt, args);
//     va_end(args);
//     if (n > 0)
//         return write(reinterpret_cast<const uint8_t *>(buf), static_cast<size_t>(n));
//     return 0;
// }
//
// int BTClassicManager::read() { return _bt.read(); }
//
// int BTClassicManager::available() const { return _bt.available(); }
//
// int BTClassicManager::peek() const { return _bt.peek(); }
//
// String BTClassicManager::readString() {
//     String s;
//     while (_bt.available())
//         s += static_cast<char>(_bt.read());
//     return s;
// }
//
// // ── Scan (Master) ─────────────────────────────────────────────────────────────
//
// std::vector<BTDevice> BTClassicManager::scan(uint32_t durationMs) {
//     std::vector<BTDevice> result;
//     BTScanResults *raw = _bt.discover(static_cast<int>(durationMs));
//     if (!raw)
//         return result;
//     for (int i = 0; i < raw->getCount(); i++) {
//         BTAdvertisedDevice *d = raw->getDevice(i);
//         BTDevice dev;
//         dev.name = d->getName().c_str();
//         dev.address = d->getAddress().toString().c_str();
//         dev.rssi = d->getRSSI();
//         dev.connectable = true;
//         result.push_back(dev);
//     }
//     return result;
// }
//
// bool BTClassicManager::connect(const std::string &address) {
//     BTAddress addr(address);
//     return connect(addr);
// }
//
// bool BTClassicManager::connect(const BTAddress &address) { return _bt.connect(address); }
//
// void BTClassicManager::disconnect() { _bt.disconnect(); }
//
// void BTClassicManager::clearPairedDevices() { _bt.deleteAllBondedDevices(); }
//
// #endif // BT_MODE_CLASSIC
