#include "config/ConfigTypes.h"

extern "C" {

uint8_t configComputeChecksum(const uint8_t* data, size_t length) {
    uint8_t sum = 0;
    if (!data || length == 0) {
        return 0;
    }

    for (size_t i = 0; i < length; i++) {
        sum = static_cast<uint8_t>(sum + data[i]);
    }

    return sum;
}

uint8_t configComputeChecksumStruct(const struct ConfigData* data) {
    if (!data) {
        return 0;
    }

    return configComputeChecksum(reinterpret_cast<const uint8_t*>(data), sizeof(ConfigData) - 1);
}

uint8_t configComputeChecksumV1(const struct ConfigDataV1* data) {
    if (!data) {
        return 0;
    }

    return configComputeChecksum(reinterpret_cast<const uint8_t*>(data), sizeof(ConfigDataV1) - 1);
}

bool configValidateChecksumStruct(const struct ConfigData* data) {
    if (!data) {
        return false;
    }

    return configComputeChecksumStruct(data) == data->checksum;
}

bool configValidateChecksumV1(const struct ConfigDataV1* data) {
    if (!data) {
        return false;
    }

    return configComputeChecksumV1(data) == data->checksum;
}

} // extern "C"
