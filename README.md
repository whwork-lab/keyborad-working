# Pico W Keyboard WiFi Bridge

Combines USB keyboard input with WiFi transmission to send keyboard data to a PC over the network.

## Hardware Setup

- **VBUS** → Power (5V)
- **Pin 38** → Ground (GND)
- **GPIO 0** → USB D+ (keyboard)
- **GPIO 1** → USB D- (keyboard)

## Configuration

Edit `main.c` and update these values:

```c
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#define SERVER_IP "192.168.1.100"  // Your PC's IP address
#define UDP_PORT 4444              // UDP port for data
```

## Building

```bash
# Set up Pico SDK environment
export PICO_SDK_PATH=/path/to/pico-sdk

# Create build directory
mkdir build
cd build

# Configure and build
cmake ..
make
```

## Flashing to Pico W

1. Hold the **BOOTSEL** button on Pico W
2. Connect USB to your PC
3. Copy `keyboard_wifi.uf2` to the Pico W drive that appears
4. Pico W will reboot automatically

## Receiving Data on PC

### Python (Recommended)

Create `receive_keyboard.py`:

```python
import socket

UDP_IP = "0.0.0.0"
UDP_PORT = 4444

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for keyboard data on port {UDP_PORT}...")
print(f"Make sure to set SERVER_IP to your PC's IP address in main.c")

# USB HID Keycode mapping
KEYCODE_MAP = {
    0x04: 'A', 0x05: 'B', 0x06: 'C', 0x07: 'D', 0x08: 'E', 0x09: 'F',
    0x0A: 'G', 0x0B: 'H', 0x0C: 'I', 0x0D: 'J', 0x0E: 'K', 0x0F: 'L',
    0x10: 'M', 0x11: 'N', 0x12: 'O', 0x13: 'P', 0x14: 'Q', 0x15: 'R',
    0x16: 'S', 0x17: 'T', 0x18: 'U', 0x19: 'V', 0x1A: 'W', 0x1B: 'X',
    0x1C: 'Y', 0x1D: 'Z', 0x1E: '1', 0x1F: '2', 0x20: '3', 0x21: '4',
    0x22: '5', 0x23: '6', 0x24: '7', 0x25: '8', 0x26: '9', 0x27: '0',
    0x28: 'Enter', 0x29: 'Esc', 0x2A: 'Backspace', 0x2B: 'Tab',
    0x2C: 'Space', 0x2D: '-', 0x2E: '=', 0x2F: '[', 0x30: ']',
    0x31: '\\', 0x33: ';', 0x34: "'", 0x35: '`', 0x36: ',',
    0x37: '.', 0x38: '/', 0x39: 'CapsLock'
}

while True:
    data, addr = sock.recvfrom(1024)
    if len(data) >= 8:
        modifier = data[0]
        reserved = data[1]
        keycodes = data[2:8]
        
        # Decode modifier keys
        mod_str = []
        if modifier & 0x01: mod_str.append('LCtrl')
        if modifier & 0x02: mod_str.append('LShift')
        if modifier & 0x04: mod_str.append('LAlt')
        if modifier & 0x08: mod_str.append('LWin')
        if modifier & 0x10: mod_str.append('RCtrl')
        if modifier & 0x20: mod_str.append('RShift')
        if modifier & 0x40: mod_str.append('RAlt')
        if modifier & 0x80: mod_str.append('RWin')
        
        # Decode keycodes
        key_str = []
        for kc in keycodes:
            if kc != 0:
                key_str.append(KEYCODE_MAP.get(kc, f'0x{kc:02X}'))
        
        mod_display = '+'.join(mod_str) if mod_str else 'None'
        key_display = ', '.join(key_str) if key_str else 'None'
        
        print(f"Modifiers: {mod_display} | Keys: {key_display}")
```

Run it:
```bash
python3 receive_keyboard.py
```

### Node.js

Create `receive_keyboard.js`:

```javascript
const dgram = require('dgram');
const server = dgram.createSocket('udp4');

const KEYCODE_MAP = {
    0x04: 'A', 0x05: 'B', 0x06: 'C', 0x07: 'D', 0x08: 'E', 0x09: 'F',
    0x0A: 'G', 0x0B: 'H', 0x0C: 'I', 0x0D: 'J', 0x0E: 'K', 0x0F: 'L',
    0x10: 'M', 0x11: 'N', 0x12: 'O', 0x13: 'P', 0x14: 'Q', 0x15: 'R',
    0x16: 'S', 0x17: 'T', 0x18: 'U', 0x19: 'V', 0x1A: 'W', 0x1B: 'X',
    0x1C: 'Y', 0x1D: 'Z', 0x1E: '1', 0x1F: '2', 0x27: '0',
    0x28: 'Enter', 0x29: 'Esc', 0x2A: 'Backspace', 0x2B: 'Tab', 0x2C: 'Space'
};

server.on('message', (msg, rinfo) => {
    const modifier = msg[0];
    const keycodes = msg.slice(2, 8);
    
    const keys = keycodes
        .filter(kc => kc !== 0)
        .map(kc => KEYCODE_MAP[kc] || `0x${kc.toString(16).padStart(2, '0')}`)
        .join(', ');
    
    console.log(`Modifier: 0x${modifier.toString(16).padStart(2, '0')} | Keys: ${keys || 'None'}`);
});

server.bind(4444, '0.0.0.0', () => {
    console.log('Listening for keyboard data on port 4444...');
});
```

Run it:
```bash
node receive_keyboard.js
```

## Keyboard Data Format

Each UDP packet contains 8 bytes:

| Byte | Content |
|------|----------|
| 0 | Modifier keys (bitmask: Ctrl, Shift, Alt, Cmd) |
| 1 | Reserved (always 0) |
| 2-7 | Keycode array (up to 6 simultaneous keys) |

### Modifier Key Bitmask
- `0x01` = Left Ctrl
- `0x02` = Left Shift
- `0x04` = Left Alt
- `0x08` = Left Windows/Cmd
- `0x10` = Right Ctrl
- `0x20` = Right Shift
- `0x40` = Right Alt
- `0x80` = Right Windows/Cmd

## Troubleshooting

**Pico W won't connect to WiFi:**
- Double-check SSID and password in `main.c`
- Make sure 2.4GHz WiFi is available (Pico W doesn't support 5GHz)
- Check if your router uses hidden SSID (may need adjustment)

**No keyboard data received:**
- Verify keyboard is detected (watch serial output)
- Confirm SERVER_IP is your PC's actual IP address
- Check firewall isn't blocking UDP port 4444
- Ensure keyboard and Pico W are on same network

**Serial output/debugging:**
```bash
python3 -m serial.tools.miniterm /dev/ttyACM0 115200
```

## References

- [USB HID Keyboard Codes](https://en.wikipedia.org/wiki/USB_human_interface_device_class#Keyboard)
- [Raspberry Pi Pico W Datasheet](https://datasheets.raspberrypi.com/picow/pico-w-datasheet.pdf)
- [TinyUSB Documentation](https://docs.tinyusb.org/)
