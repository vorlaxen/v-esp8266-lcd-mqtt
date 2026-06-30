#include <unity.h>
#include "config/ConfigTypes.h"
#include <string.h>

void test_config_v3_size_stable() {
    TEST_ASSERT_EQUAL(302, sizeof(ConfigData));
}

void test_config_v1_size_stable() {
    TEST_ASSERT_EQUAL(107, sizeof(ConfigDataV1));
}

void test_checksum_empty_data() {
    TEST_ASSERT_EQUAL(0, configComputeChecksum(nullptr, 0));
}

void test_checksum_known_vector() {
    const uint8_t bytes[] = {0x01, 0x02, 0x03};
    TEST_ASSERT_EQUAL(6, configComputeChecksum(bytes, sizeof(bytes)));
}

void test_checksum_excludes_stored_byte() {
    ConfigData data{};
    memset(&data, 0, sizeof(data));
    data.magic = CONFIG_MAGIC;
    data.version = CONFIG_VERSION;
    strncpy(data.deviceId, "123456", sizeof(data.deviceId));
    data.checksum = 0;
    const uint8_t expected = configComputeChecksumStruct(&data);
    data.checksum = expected;
    TEST_ASSERT_TRUE(configValidateChecksumStruct(&data));
}

void test_checksum_detects_tamper() {
    ConfigData data{};
    memset(&data, 0, sizeof(data));
    data.magic = CONFIG_MAGIC;
    data.version = CONFIG_VERSION;
    data.checksum = configComputeChecksumStruct(&data);
    data.ssid[0] = 'X';
    TEST_ASSERT_FALSE(configValidateChecksumStruct(&data));
}

void test_v1_checksum_roundtrip() {
    ConfigDataV1 data{};
    memset(&data, 0, sizeof(data));
    data.magic = CONFIG_MAGIC;
    data.version = 1;
    strncpy(data.deviceId, "654321", sizeof(data.deviceId));
    strncpy(data.ssid, "TestNet", sizeof(data.ssid));
    data.checksum = configComputeChecksumV1(&data);
    TEST_ASSERT_TRUE(configValidateChecksumV1(&data));
}

void setUp() {}
void tearDown() {}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();
    RUN_TEST(test_config_v3_size_stable);
    RUN_TEST(test_config_v1_size_stable);
    RUN_TEST(test_checksum_empty_data);
    RUN_TEST(test_checksum_known_vector);
    RUN_TEST(test_checksum_excludes_stored_byte);
    RUN_TEST(test_checksum_detects_tamper);
    RUN_TEST(test_v1_checksum_roundtrip);
    return UNITY_END();
}
