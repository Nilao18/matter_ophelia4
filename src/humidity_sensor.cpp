#include "humidity_sensor.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/DataModelTypes.h>
#include <protocols/interaction_model/StatusCode.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(humidity_sensor, CONFIG_CHIP_APP_LOG_LEVEL);

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using chip::Protocols::InteractionModel::Status;

/* Endpoint 1 configuration */
static constexpr EndpointId kHumidityEndpointId = 1;
static constexpr uint16_t kHumidityDeviceType = 0x0307; /* Humidity Sensor */

/* Attribute storage for Relative Humidity Measurement cluster (0x0405) */
static uint16_t sMeasuredValue = 0;
static uint16_t sMinMeasuredValue = 0;
static uint16_t sMaxMeasuredValue = 10000;

/* Attribute storage for Identify cluster (0x0003) */
static uint16_t sIdentifyTime = 0;
static uint8_t sIdentifyType = 0; /* None */

// clang-format off

/* Identify cluster attributes (0x0003) - MANDATORY for Humidity Sensor */
static constexpr EmberAfAttributeMetadata sIdentifyAttrs[] = {
    /* IdentifyTime */
    { ZAP_SIMPLE_DEFAULT(0), 0x0000, 2, ZAP_TYPE(INT16U),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) | ZAP_ATTRIBUTE_MASK(WRITABLE) },
    /* IdentifyType */
    { ZAP_SIMPLE_DEFAULT(0), 0x0001, 1, ZAP_TYPE(ENUM8),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* FeatureMap */
    { ZAP_EMPTY_DEFAULT(), 0xFFFC, 4, ZAP_TYPE(BITMAP32),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* ClusterRevision */
    { ZAP_SIMPLE_DEFAULT(4), 0xFFFD, 2, ZAP_TYPE(INT16U),
      ZAP_ATTRIBUTE_MASK(READABLE) },
};

/* RelativeHumidityMeasurement cluster attributes (0x0405) */
static constexpr EmberAfAttributeMetadata sHumidityAttrs[] = {
    /* MeasuredValue */
    { ZAP_EMPTY_DEFAULT(), 0x0000, 2, ZAP_TYPE(INT16U),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE) },
    /* MinMeasuredValue */
    { ZAP_EMPTY_DEFAULT(), 0x0001, 2, ZAP_TYPE(INT16U),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE) },
    /* MaxMeasuredValue */
    { ZAP_EMPTY_DEFAULT(), 0x0002, 2, ZAP_TYPE(INT16U),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) | ZAP_ATTRIBUTE_MASK(NULLABLE) },
    /* FeatureMap */
    { ZAP_EMPTY_DEFAULT(), 0xFFFC, 4, ZAP_TYPE(BITMAP32),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* ClusterRevision */
    { ZAP_SIMPLE_DEFAULT(3), 0xFFFD, 2, ZAP_TYPE(INT16U),
      ZAP_ATTRIBUTE_MASK(READABLE) },
};

/* Descriptor cluster attributes (0x001D) */
static constexpr EmberAfAttributeMetadata sDescriptorAttrs[] = {
    /* DeviceTypeList */
    { ZAP_EMPTY_DEFAULT(), 0x0000, 0, ZAP_TYPE(ARRAY),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* ServerList */
    { ZAP_EMPTY_DEFAULT(), 0x0001, 0, ZAP_TYPE(ARRAY),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* ClientList */
    { ZAP_EMPTY_DEFAULT(), 0x0002, 0, ZAP_TYPE(ARRAY),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* PartsList */
    { ZAP_EMPTY_DEFAULT(), 0x0003, 0, ZAP_TYPE(ARRAY),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* FeatureMap */
    { ZAP_EMPTY_DEFAULT(), 0xFFFC, 4, ZAP_TYPE(BITMAP32),
      ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) | ZAP_ATTRIBUTE_MASK(READABLE) },
    /* ClusterRevision */
    { ZAP_SIMPLE_DEFAULT(2), 0xFFFD, 2, ZAP_TYPE(INT16U),
      ZAP_ATTRIBUTE_MASK(READABLE) },
};
// clang-format on

/* Identify command lists */
static constexpr CommandId sIdentifyAcceptedCommands[] = {
    0x0000, /* Identify */
    0x0040, /* TriggerEffect */
    kInvalidCommandId,
};

static constexpr EmberAfCluster sHumidityClusters[] = {
    {
        .clusterId = Descriptor::Id,
        .attributes = sDescriptorAttrs,
        .attributeCount = ARRAY_SIZE(sDescriptorAttrs),
        .clusterSize = 0,
        .mask = ZAP_CLUSTER_MASK(SERVER),
        .functions = nullptr,
        .acceptedCommandList = nullptr,
        .generatedCommandList = nullptr,
        .eventList = nullptr,
        .eventCount = 0,
    },
    {
        .clusterId = Identify::Id,
        .attributes = sIdentifyAttrs,
        .attributeCount = ARRAY_SIZE(sIdentifyAttrs),
        .clusterSize = 0,
        .mask = ZAP_CLUSTER_MASK(SERVER),
        .functions = nullptr,
        .acceptedCommandList = sIdentifyAcceptedCommands,
        .generatedCommandList = nullptr,
        .eventList = nullptr,
        .eventCount = 0,
    },
    {
        .clusterId = RelativeHumidityMeasurement::Id,
        .attributes = sHumidityAttrs,
        .attributeCount = ARRAY_SIZE(sHumidityAttrs),
        .clusterSize = 0,
        .mask = ZAP_CLUSTER_MASK(SERVER),
        .functions = nullptr,
        .acceptedCommandList = nullptr,
        .generatedCommandList = nullptr,
        .eventList = nullptr,
        .eventCount = 0,
    },
};

static constexpr EmberAfEndpointType sHumidityEndpoint = {
    .cluster = sHumidityClusters,
    .clusterCount = ARRAY_SIZE(sHumidityClusters),
    .endpointSize = 0,
};

static constexpr EmberAfDeviceType sDeviceTypeArray[] = {
    { kHumidityDeviceType, 2 },
};

static const Span<const EmberAfDeviceType> sDeviceTypes(sDeviceTypeArray);

static DataVersion sDataVersions[ARRAY_SIZE(sHumidityClusters)];

/* External attribute read callback */
Status emberAfExternalAttributeReadCallback(
    EndpointId endpoint, ClusterId clusterId,
    const EmberAfAttributeMetadata *attributeMetadata,
    uint8_t *buffer, uint16_t maxReadLength)
{
    if (endpoint != kHumidityEndpointId) {
        return Status::Failure;
    }

    uint16_t attrId = attributeMetadata->attributeId;

    if (clusterId == Identify::Id) {
        if (attrId == 0x0000) { /* IdentifyTime */
            memcpy(buffer, &sIdentifyTime, sizeof(sIdentifyTime));
            return Status::Success;
        }
        if (attrId == 0x0001) { /* IdentifyType */
            memcpy(buffer, &sIdentifyType, sizeof(sIdentifyType));
            return Status::Success;
        }
        if (attrId == 0xFFFC) { /* FeatureMap */
            uint32_t featureMap = 0;
            memcpy(buffer, &featureMap, sizeof(featureMap));
            return Status::Success;
        }
    }

    if (clusterId == RelativeHumidityMeasurement::Id) {
        if (attrId == 0x0000) {
            memcpy(buffer, &sMeasuredValue, sizeof(sMeasuredValue));
            return Status::Success;
        }
        if (attrId == 0x0001) {
            memcpy(buffer, &sMinMeasuredValue, sizeof(sMinMeasuredValue));
            return Status::Success;
        }
        if (attrId == 0x0002) {
            memcpy(buffer, &sMaxMeasuredValue, sizeof(sMaxMeasuredValue));
            return Status::Success;
        }
        if (attrId == 0xFFFC) {
            uint32_t featureMap = 0;
            memcpy(buffer, &featureMap, sizeof(featureMap));
            return Status::Success;
        }
    }

    return Status::Failure;
}

/* External attribute write callback (needed for Identify IdentifyTime) */
Status emberAfExternalAttributeWriteCallback(
    EndpointId endpoint, ClusterId clusterId,
    const EmberAfAttributeMetadata *attributeMetadata,
    uint8_t *buffer)
{
    if (endpoint != kHumidityEndpointId) {
        return Status::Failure;
    }

    if (clusterId == Identify::Id && attributeMetadata->attributeId == 0x0000) {
        memcpy(&sIdentifyTime, buffer, sizeof(sIdentifyTime));
        return Status::Success;
    }

    return Status::Failure;
}

CHIP_ERROR InitHumiditySensorEndpoint()
{
    CHIP_ERROR err = emberAfSetDynamicEndpoint(
        0,
        kHumidityEndpointId,
        &sHumidityEndpoint,
        Span<DataVersion>(sDataVersions),
        sDeviceTypes);

    if (err != CHIP_NO_ERROR) {
        LOG_ERR("Failed to add humidity endpoint: %" CHIP_ERROR_FORMAT, err.Format());
    } else {
        LOG_INF("Humidity Sensor endpoint %d added successfully", kHumidityEndpointId);
    }

    return err;
}

void SetMeasuredHumidity(uint16_t value)
{
    sMeasuredValue = value;
    MatterReportingAttributeChangeCallback(
        kHumidityEndpointId,
        RelativeHumidityMeasurement::Id,
        RelativeHumidityMeasurement::Attributes::MeasuredValue::Id);
}
