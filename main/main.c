#include "esp_log.h"
#include "esp_event.h"

#include "helper/nvs_helper.h"
#include "helper/wifi_helper.h"
#include "helper/dns_helper.h"
#include "helper/sntp_helper.h"

#include "example_provisioning.h"
#include "example_iot_hub.h"

#include "esp32_iot_azure/azure_iot_sdk.h"

static const char TAG[] = "main";
static void initialize_infra();

static char WIFI_SSID[] = "";
static char WIFI_PASSWORD[] = "";
static utf8_string_t DEVICE_SYMMETRIC_KEY = UTF8_STRING_FROM_LITERAL("");
static utf8_string_t DEVICE_REGISTRATION_ID = UTF8_STRING_FROM_LITERAL("test-device");

void app_main(void)
{
    initialize_infra();

    utf8_string_t hostname = UTF8_STRING_WITH_FIXED_LENGTH(AZURE_CONST_HOSTNAME_MAX_LENGTH);
    utf8_string_t device_id = UTF8_STRING_WITH_FIXED_LENGTH(AZURE_CONST_DEVICE_ID_MAX_LENGTH);

    azure_iot_sdk_init();

    if (!example_dps_run(&DEVICE_SYMMETRIC_KEY, &DEVICE_REGISTRATION_ID, &hostname, &device_id))
    {
        ESP_LOGE(TAG, "failure running example: example_dps_run");
        abort();
    }

    if (!example_iot_hub_run(&hostname, &device_id, &DEVICE_SYMMETRIC_KEY))
    {
        ESP_LOGE(TAG, "failure running example: example_iot_hub_run");
        abort();
    }

    azure_iot_sdk_deinit();

    ESP_LOGW(TAG, "examples finished");
}

//
// INFRA
//

static void initialize_infra()
{
    if (esp_event_loop_create_default() != ESP_OK)
    {
        ESP_LOGE(TAG, "failure creating default event loop");
        abort();
    }

    if (!helper_nvs_init())
    {
        ESP_LOGE(TAG, "failure initializing flash");
        abort();
    }

    if (!helper_wifi_init())
    {
        ESP_LOGE(TAG, "failure initializing wifi");
        abort();
    }

    if (!helper_wifi_connect(WIFI_SSID, sizeof(WIFI_SSID) - 1, WIFI_PASSWORD, sizeof(WIFI_PASSWORD) - 1))
    {
        ESP_LOGE(TAG, "failure connecting to wifi");
        abort();
    }

    if (!helper_dns_set_servers())
    {
        ESP_LOGE(TAG, "failure setting dns");
        abort();
    }

    if (!helper_sntp_init())
    {
        ESP_LOGE(TAG, "failure initializing sntp");
        abort();
    }

    helper_sntp_wait_for_sync();
}