#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include "hardware/gpio.h"

// USB Host includes
#include "tusb.h"
#include "class/hid/hid.h"

// WiFi Configuration
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#define UDP_PORT 4444
#define SERVER_IP "192.168.1.100"  // Change to your PC's IP

// Keyboard report structure
typedef struct {
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
} keyboard_report_t;

static keyboard_report_t last_keyboard_report = {0};
static struct udp_pcb *udp_socket = NULL;
static ip_addr_t server_addr;
static bool wifi_connected = false;

// ===== WiFi Functions =====

void wifi_connect(void) {
    printf("Connecting to WiFi: %s\n", WIFI_SSID);
    cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
}

void wifi_link_status_callback(void) {
    int link_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
    
    if (link_status == CYW43_LINK_UP) {
        if (!wifi_connected) {
            wifi_connected = true;
            printf("WiFi connected!\n");
            printf("IP: %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));
            
            // Create UDP socket
            if (udp_socket == NULL) {
                udp_socket = udp_new();
                inet_aton(SERVER_IP, &server_addr);
            }
        }
    } else if (link_status < CYW43_LINK_UP) {
        if (wifi_connected) {
            wifi_connected = false;
            printf("WiFi disconnected\n");
        }
    }
}

void send_keyboard_data(keyboard_report_t *report) {
    if (!wifi_connected || udp_socket == NULL) {
        return;
    }
    
    // Create a packet with keyboard data
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, sizeof(keyboard_report_t), PBUF_RAM);
    if (p != NULL) {
        memcpy(p->payload, report, sizeof(keyboard_report_t));
        udp_sendto(udp_socket, p, &server_addr, UDP_PORT);
        pbuf_free(p);
        
        // Debug output
        printf("Sent keyboard data: mod=%02x keys=%02x %02x %02x %02x %02x %02x\n",
               report->modifier, report->keycode[0], report->keycode[1],
               report->keycode[2], report->keycode[3], report->keycode[4],
               report->keycode[5]);
    }
}

// ===== USB HID Functions =====

void process_kbd_report(hid_keyboard_report_t const *report) {
    keyboard_report_t kbd_report;
    kbd_report.modifier = report->modifier;
    kbd_report.reserved = 0;
    memcpy(kbd_report.keycode, report->keycode, 6);
    
    // Check if report changed
    if (memcmp(&last_keyboard_report, &kbd_report, sizeof(keyboard_report_t)) != 0) {
        last_keyboard_report = kbd_report;
        send_keyboard_data(&kbd_report);
    }
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
    printf("HID device mounted at addr %d, instance %d\n", dev_addr, instance);
    
    if (!tuh_hid_receive_report(dev_addr, instance)) {
        printf("Error: cannot request report\n");
    }
}

void tuh_hid_unmount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID device unmounted at addr %d, instance %d\n", dev_addr, instance);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {
    if (len > 0) {
        // Determine if this is a keyboard report
        if (len == sizeof(hid_keyboard_report_t)) {
            process_kbd_report((hid_keyboard_report_t const *)report);
        }
    }
    
    // Request next report
    if (!tuh_hid_receive_report(dev_addr, instance)) {
        printf("Error: cannot request report\n");
    }
}

// ===== Main Program =====

int main(void) {
    stdio_init_all();
    sleep_ms(1000);
    
    printf("=== Pico W Keyboard WiFi Bridge ===\n");
    
    // Initialize Pico W WiFi
    printf("Initializing WiFi...\n");
    if (cyw43_arch_init()) {
        printf("WiFi init failed\n");
        return -1;
    }
    
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_async(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);
    
    // Initialize USB Host
    printf("Initializing USB Host...\n");
    tusb_init();
    
    // Main loop
    while (1) {
        // Service WiFi
        cyw43_arch_poll();
        wifi_link_status_callback();
        
        // Service USB
        tuh_task();
        
        sleep_ms(10);
    }
    
    return 0;
}
