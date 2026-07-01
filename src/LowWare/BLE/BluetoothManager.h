// #pragma once
// /**
//  * @file BluetoothManager.h
//  * @brief Lib C++ unifiée Bluetooth pour ESP32
//  *        Couvre deux modes, sélectionnables à la compilation :
//  *
//  *   MODE BLE (Bluetooth Low Energy) — via NimBLE-Arduino
//  *     → #define BT_MODE_BLE  (défaut)
//  *     → Rôles : PERIPHERAL (serveur GATT) ou CENTRAL (client BLE)
//  *     → Nordic UART Service (NUS) pour émulation série BLE
//  *     → Scan + connexion à un périphérique BLE distant
//  *
//  *   MODE CLASSIC (Bluetooth Classic SPP) — via BluetoothSerial (Bluedroid)
//  *     → #define BT_MODE_CLASSIC
//  *     → Rôles : SLAVE (serveur SPP) ou MASTER (client SPP)
//  *     → API stream-compatible (read/write/available/println)
//  *     → Scan des appareils voisins
//  *
//  * ──────────────────────────────────────────────────────────────────────
//  * IMPORTANT – Contraintes matérielles ESP32 :
//  *   • BLE et Classic BT ne peuvent PAS coexister avec WiFi simultanément
//  *     sans coexistence activée dans sdkconfig.
//  *   • Classic BT n'est disponible que sur ESP32 (WROOM/WROVER).
//  *     ESP32-S3, C3, etc. → BLE uniquement.
//  *   • NimBLE (~50 KB RAM) est recommandé plutôt que Bluedroid (~100 KB RAM).
//  * ──────────────────────────────────────────────────────────────────────
//  *
//  * Dépendances (PlatformIO) :
//  *   BLE    : h2zero/NimBLE-Arduino@^2.1.0
//  *   Classic: inclus dans arduino-esp32 (BluetoothSerial)
//  */
//
// #pragma once
// #include <Arduino.h>
// #include <cstdint>
// #include <functional>
// #include <string>
// #include <vector>
//
// // ─────────────────────────────────────────────
// //  Sélection du mode à la compilation
// // ─────────────────────────────────────────────
// #if !defined(BT_MODE_BLE) && !defined(BT_MODE_CLASSIC)
// #define BT_MODE_BLE // mode par défaut
// #endif
//
// // ─────────────────────────────────────────────
// //  UUIDs Nordic UART Service (NUS) — standard BLE serial
// // ─────────────────────────────────────────────
// #define NUS_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
// #define NUS_RX_CHARACTERISTIC "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // Write (host→device)
// #define NUS_TX_CHARACTERISTIC "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" // Notify (device→host)
//
// // ─────────────────────────────────────────────
// //  Structures communes
// // ─────────────────────────────────────────────
// struct BTDevice {
//     std::string name;
//     std::string address;
//     int8_t rssi;
//     bool connectable;
// };
//
// // ─────────────────────────────────────────────────────────────────────────────
// //  ═══════════════  MODE BLE  ═══════════════
// // ─────────────────────────────────────────────────────────────────────────────
// #ifdef BT_MODE_BLE
//
// #include <NimBLEAdvertising.h>
// #include <NimBLEClient.h>
// #include <NimBLEDevice.h>
// #include <NimBLEScan.h>
// #include <NimBLEServer.h>
//
// /** Rôle BLE */
// enum class BLERole : uint8_t {
//     PERIPHERAL, ///< Serveur GATT (advertise + accepte connexions)
//     CENTRAL, ///< Client BLE (scan + se connecte)
// };
//
// /**
//  * @brief Gestionnaire BLE (NimBLE) – mode PERIPHERAL (serveur NUS)
//  *        ou CENTRAL (client BLE).
//  */
// class BLEManager : public NimBLEServerCallbacks,
//                    public NimBLECharacteristicCallbacks,
//                    public NimBLEClientCallbacks,
//                    public NimBLEScanCallbacks {
// public:
//     /**
//      * @param deviceName  Nom Bluetooth visible
//      * @param role        PERIPHERAL ou CENTRAL
//      */
//     explicit BLEManager(const char *deviceName, BLERole role = BLERole::PERIPHERAL);
//
//     // ── Init ────────────────────────────────
//     /**
//      * @brief Initialise le stack NimBLE.
//      *        En mode PERIPHERAL : démarre advertising avec Nordic UART Service.
//      *        En mode CENTRAL    : prêt à scanner.
//      */
//     void begin();
//
//     /** Arrête le BLE et libère les ressources */
//     void end();
//
//     // ── État ────────────────────────────────
//     bool isConnected() const { return _connected; }
//
//     bool hasData() const { return _rxBuf.length() > 0; }
//
//     uint16_t getMTU() const;
//
//     // ── API "série" BLE (NUS) ───────────────
//     /** Envoie des octets via la caractéristique TX (notify) */
//     size_t write(const uint8_t *data, size_t len);
//     size_t write(const char *str);
//     size_t println(const char *str);
//     size_t printf(const char *fmt, ...);
//
//     /** Lit un octet depuis le buffer RX (ou -1 si vide) */
//     int read();
//
//     /** Nombre d'octets disponibles en lecture */
//     int available() const { return static_cast<int>(_rxBuf.length()); }
//
//     /** Retourne et vide le buffer RX complet */
//     std::string readString();
//
//     // ── Scan (CENTRAL) ──────────────────────
//     /**
//      * @brief Lance un scan BLE (CENTRAL uniquement)
//      * @param durationSec Durée en secondes
//      * @param onFound     Callback appelé pour chaque device trouvé
//      */
//     void scan(uint32_t durationSec, std::function<void(const BTDevice &)> onFound = nullptr);
//
//     /**
//      * @brief Connexion à un périphérique BLE par adresse MAC
//      * @param address  Adresse MAC (ex: "AA:BB:CC:DD:EE:FF")
//      * @return true si connexion + NUS trouvé
//      */
//     bool connect(const std::string &address);
//
//     /** Déconnexion (CENTRAL) */
//     void disconnect();
//
//     // ── Advertising (PERIPHERAL) ────────────
//     /** Redémarre l'advertising après une déconnexion */
//     void restartAdvertising();
//
//     /** Données de fabricant dans l'advertising (max ~20 octets) */
//     void setManufacturerData(const uint8_t *data, size_t len);
//
//     // ── Callbacks utilisateur ───────────────
//     void onConnect(std::function<void()> cb) { _onConnectCb = cb; }
//
//     void onDisconnect(std::function<void()> cb) { _onDisconnectCb = cb; }
//
//     void onReceive(std::function<void(const std::string &)> cb) { _onReceiveCb = cb; }
//
//     // ── NimBLE internal callbacks ───────────
//     void onConnect(NimBLEServer *s) override;
//     void onDisconnect(NimBLEServer *s, NimBLEConnInfo &info, int reason) override;
//     void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &info) override;
//     void onConnect(NimBLEClient *c) override;
//     void onDisconnect(NimBLEClient *c, int reason) override;
//     void onResult(const NimBLEAdvertisedDevice *device) override;
//
// private:
//     const char *_name;
//     BLERole _role;
//     bool _connected{false};
//     std::string _rxBuf;
//
//     // PERIPHERAL
//     NimBLEServer *_server{nullptr};
//     NimBLECharacteristic *_txChar{nullptr};
//     NimBLECharacteristic *_rxChar{nullptr};
//
//     // CENTRAL
//     NimBLEClient *_client{nullptr};
//     NimBLERemoteCharacteristic *_remTxChar{nullptr};
//     NimBLERemoteCharacteristic *_remRxChar{nullptr};
//     NimBLEScan *_scanner{nullptr};
//     std::function<void(const BTDevice &)> _scanCb;
//
//     // Callbacks utilisateur
//     std::function<void()> _onConnectCb;
//     std::function<void()> _onDisconnectCb;
//     std::function<void(const std::string &)> _onReceiveCb;
//
//     void _initPeripheral();
//     void _initCentral();
// };
//
// #endif // BT_MODE_BLE
//
//
// // ─────────────────────────────────────────────────────────────────────────────
// //  ═══════════════  MODE CLASSIC  ═══════════════
// // ─────────────────────────────────────────────────────────────────────────────
// #ifdef BT_MODE_CLASSIC
//
// #include <BluetoothSerial.h>
//
// // Vérifications de compatibilité
// #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
// #error "Bluetooth Classic nécessite CONFIG_BT_ENABLED + CONFIG_BLUEDROID_ENABLED dans sdkconfig"
// #endif
// #if !defined(CONFIG_BT_SPP_ENABLED)
// #error "Bluetooth Classic SPP nécessite CONFIG_BT_SPP_ENABLED dans sdkconfig"
// #endif
//
// /** Rôle Classic BT */
// enum class BTClassicRole : uint8_t {
//     SLAVE, ///< Accepte les connexions (périphérique visible)
//     MASTER, ///< Initie les connexions
// };
//
// /**
//  * @brief Gestionnaire Bluetooth Classic SPP
//  *        Wrapper autour de BluetoothSerial avec gestion propre
//  *        des callbacks et buffer circulaire.
//  */
// class BTClassicManager {
// public:
//     /**
//      * @param deviceName  Nom visible en Bluetooth Classic
//      * @param role        SLAVE ou MASTER
//      * @param pin         Code PIN pour le pairing (défaut : "1234")
//      */
//     explicit BTClassicManager(const char *deviceName, BTClassicRole role = BTClassicRole::SLAVE,
//                               const char *pin = "1234");
//
//     // ── Init ────────────────────────────────
//     void begin();
//     void end();
//
//     // ── État ────────────────────────────────
//     bool isConnected() const;
//
//     bool isReady() const { return _ready; }
//
//     // ── API série ───────────────────────────
//     size_t write(const uint8_t *data, size_t len);
//     size_t write(const char *str);
//     size_t println(const char *str);
//     size_t printf(const char *fmt, ...);
//     int read();
//     int available() const;
//     String readString();
//     int peek() const;
//
//     // ── Master : scan + connexion ───────────
//     /**
//      * @brief Scan synchrone des appareils Classic à portée
//      * @param durationMs Durée en ms (recommandé ≥ 5000 ms)
//      * @return Liste des appareils trouvés
//      */
//     std::vector<BTDevice> scan(uint32_t durationMs = 10000);
//
//     /**
//      * @brief Connexion à un appareil par adresse MAC (Master)
//      * @param address  "AA:BB:CC:DD:EE:FF" ou "AABBCCDDEEFF"
//      * @return true si connexion réussie
//      */
//     bool connect(const std::string &address);
//     bool connect(const BTAddress &address);
//
//     /** Déconnexion */
//     void disconnect();
//
//     /** Supprimer tous les appareils appairés */
//     void clearPairedDevices();
//
//     // ── Callbacks ───────────────────────────
//     void onConnect(std::function<void(BTAddress addr)> cb) { _onConnectCb = cb; }
//
//     void onDisconnect(std::function<void(BTAddress addr)> cb) { _onDisconnectCb = cb; }
//
// private:
//     const char *_name;
//     BTClassicRole _role;
//     const char *_pin;
//     bool _ready{false};
//     BluetoothSerial _bt;
//
//     std::function<void(BTAddress)> _onConnectCb;
//     std::function<void(BTAddress)> _onDisconnectCb;
// };
//
// #endif // BT_MODE_CLASSIC
