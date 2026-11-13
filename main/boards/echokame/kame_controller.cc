/*
    Kame32 robot controller, protocol described in README.md
*/

#include "kame_controller.h"

#include <driver/uart.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_timer.h>

#include <cstdio>
#include <cstring>

#include "config.h"

#define TAG "KameController"

namespace {

// TX is buffered so uart_write_bytes() never blocks the caller
constexpr int kRxBufferSize = 256;
constexpr int kTxBufferSize = 256;
constexpr size_t kMaxEmotionLength = 32;

}  // namespace

class KameController {
public:
    explicit KameController(gpio_num_t tx_pin) {
        uart_config_t uart_config = {
            .baud_rate = KAME_UART_BAUD_RATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };

        esp_err_t err = uart_driver_install(KAME_UART_PORT_NUM, kRxBufferSize, kTxBufferSize, 0,
                                            nullptr, 0);
        if (err == ESP_OK) {
            err = uart_param_config(KAME_UART_PORT_NUM, &uart_config);
        }
        if (err == ESP_OK) {
            err = uart_set_pin(KAME_UART_PORT_NUM, tx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE);
        }
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "UART init failed: %s", esp_err_to_name(err));
            return;
        }
        ready_ = true;

        const esp_timer_create_args_t timer_args = {
            .callback = &KameController::HeartbeatTimerCallback,
            .arg = this,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "kame_heartbeat",
            .skip_unhandled_events = true,
        };
        err = esp_timer_create(&timer_args, &heartbeat_timer_);
        if (err == ESP_OK) {
            err = esp_timer_start_periodic(heartbeat_timer_, KAME_HEARTBEAT_INTERVAL_MS * 1000ULL);
        }
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Heartbeat timer failed: %s", esp_err_to_name(err));
        }

        ESP_LOGI(TAG, "Kame32 link ready, TX GPIO%d, %d baud", static_cast<int>(tx_pin),
                 KAME_UART_BAUD_RATE);
    }

    void SendEmotion(const char* emotion) {
        if (!ready_ || !IsValidEmotion(emotion)) {
            return;
        }
        char line[kMaxEmotionLength + 4];
        int len = snprintf(line, sizeof(line), "E:%s\n", emotion);
        uart_write_bytes(KAME_UART_PORT_NUM, line, len);
    }

private:
    bool ready_ = false;
    esp_timer_handle_t heartbeat_timer_ = nullptr;

    static bool IsValidEmotion(const char* emotion) {
        if (emotion == nullptr) {
            return false;
        }
        size_t len = strnlen(emotion, kMaxEmotionLength + 1);
        if (len == 0 || len > kMaxEmotionLength) {
            return false;
        }
        for (size_t i = 0; i < len; i++) {
            char c = emotion[i];
            bool valid = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
            if (!valid) {
                return false;
            }
        }
        return true;
    }

    static void HeartbeatTimerCallback(void* arg) {
        uart_write_bytes(KAME_UART_PORT_NUM, "H\n", 2);
    }
};

static KameController* g_kame_controller = nullptr;

void InitializeKameController(gpio_num_t tx_pin) {
    if (g_kame_controller == nullptr) {
        g_kame_controller = new KameController(tx_pin);
    }
}

void KameSendEmotion(const char* emotion) {
    if (g_kame_controller != nullptr) {
        g_kame_controller->SendEmotion(emotion);
    }
}
