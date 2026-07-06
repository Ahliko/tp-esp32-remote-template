#include <Arduino.h>
#include <unity.h>
#include "RAMTest.h"

// Instance globale du pointeur (obligatoire pour setup/teardown)
RAMTest* ram = nullptr;

// Ces fonctions sont automatiquement appelées par Unity
void setUp(void) {
    // Initialisation avant chaque test
    ram = new RAMTest(10);
    ram->init();
}

void tearDown(void) {
    // Nettoyage après chaque test
    delete ram;
    ram = nullptr;
}

void test_initial_state(void) {
    TEST_ASSERT_TRUE(ram->isEmpty());
    TEST_ASSERT_FALSE(ram->isFull());
    TEST_ASSERT_EQUAL_INT(0, ram->available());
}

void test_write_basic(void) {
    const char* data = "Hello";
    size_t written = ram->write((const uint8_t*)data, 5);

    TEST_ASSERT_EQUAL_INT(5, written);
    TEST_ASSERT_EQUAL_INT(5, ram->available());
    TEST_ASSERT_FALSE(ram->isEmpty());

    TEST_ASSERT_EQUAL_INT(5, ram->getHead());
    TEST_ASSERT_EQUAL_INT(0, ram->getTail());
}

void test_circular_overwrite(void) {
    // Écriture de 8 octets
    ram->write((const uint8_t*)"12345678", 8);
    TEST_ASSERT_EQUAL_INT(8, ram->available());
    TEST_ASSERT_FALSE(ram->isFull());

    // Écriture de 4 octets supplémentaires (dépassement de capacité 10)
    ram->write((const uint8_t*)"9ABC", 4);

    TEST_ASSERT_TRUE(ram->isFull());
    TEST_ASSERT_EQUAL_INT(10, ram->available());

    TEST_ASSERT_EQUAL_INT(2, ram->getHead());
    TEST_ASSERT_EQUAL_INT(2, ram->getTail());

    const char* buf = ram->getBuffer();
    TEST_ASSERT_EQUAL_INT('B', buf[0]);
    TEST_ASSERT_EQUAL_INT('C', buf[1]);
    TEST_ASSERT_EQUAL_INT('3', buf[2]);
    TEST_ASSERT_EQUAL_INT('9', buf[8]);
    TEST_ASSERT_EQUAL_INT('A', buf[9]);
}

void test_clear(void) {
    ram->write((const uint8_t*)"Test", 4);
    ram->clear();

    TEST_ASSERT_TRUE(ram->isEmpty());
    TEST_ASSERT_EQUAL_INT(0, ram->available());
    TEST_ASSERT_EQUAL_INT(0, ram->getHead());
}

// Fonction principale du test sur ESP32
void setup() {
    // Délai de démarrage pour laisser le temps au Serial Monitor de s'attacher
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_initial_state);
    RUN_TEST(test_write_basic);
    RUN_TEST(test_circular_overwrite);
    RUN_TEST(test_clear);

    UNITY_END();
}

void loop() {
    // Rien à faire ici
}