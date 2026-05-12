# DIY RP2040 Zero + SX1262 LoRa Variant

This variant targets a custom board combining a **Raspberry Pi RP2040 Zero** with an
**SX1262-based LoRa module** (e.g. Waveshare LoRa breakout).

---

## Hardware Wiring

Connect the SX1262 LoRa module to the RP2040 Zero as follows:

| RP2040 Zero GPIO | LoRa Module Pin | Function        |
|-----------------|-----------------|-----------------|
| GPIO 7          | LED             | TX indicator LED |
| GPIO 9          | DIO1            | LoRa IRQ        |
| GPIO 10         | BUSY            | LoRa busy       |
| GPIO 11         | RESET           | LoRa reset      |
| GPIO 12         | MISO            | SPI1 MISO       |
| GPIO 13         | NSS / CS        | SPI1 chip select |
| GPIO 14         | SCK             | SPI1 clock      |
| GPIO 15         | MOSI            | SPI1 MOSI       |
| 3.3V            | VCC             | Power           |
| GND             | GND             | Ground          |

**Battery voltage measurement (optional):**
Connect a voltage divider to GPIO 28 using two 1% resistors:

```
BAT+ ──┬──────────────┐
       │              │
     VSYS         200kΩ resistor
                      │
                   GPIO 28
                      │
                   100kΩ resistor
                      │
BAT- ──┴──────────────┘
```

---

## Available Build Targets

| Target | Description |
|--------|-------------|
| `diy_rp2040_zero_repeater` | Mesh repeater |
| `diy_rp2040_zero_repeater_bridge_rs232` | Repeater with RS232 bridge |
| `diy_rp2040_zero_room_server` | Chat room server |
| `diy_rp2040_zero_companion_radio_usb` | Companion radio via USB |
| `diy_rp2040_zero_companion_radio_usb_fwd` | Companion radio via USB + serial forward to ESP32 |
| `diy_rp2040_zero_terminal_chat` | Standalone terminal chat |

---

## Building the Firmware

**Prerequisites:** PlatformIO must be installed and available as `pio` in your terminal.

**Windows (cmd.exe):**
```cmd
set FIRMWARE_VERSION=v1.15.0
python build.py build-firmware diy_rp2040_zero_companion_radio_usb
```

**Windows (PowerShell):**
```powershell
$env:FIRMWARE_VERSION="v1.15.0"
python build.py build-firmware diy_rp2040_zero_companion_radio_usb
```

**Linux / macOS:**
```bash
export FIRMWARE_VERSION=v1.15.0
python build.py build-firmware diy_rp2040_zero_companion_radio_usb
```

The compiled firmware is placed in the `out/` directory as `.bin` and `.uf2` files.

**Flashing:** Hold the BOOTSEL button while plugging in the RP2040 Zero via USB.
A mass storage device appears — copy the `.uf2` file onto it.

---

## Serial Forward Feature (`_usb_fwd` target)

The `diy_rp2040_zero_companion_radio_usb_fwd` target adds a second UART that
forwards incoming MeshCore messages to an external device (e.g. an ESP32).

### Wiring (RP2040 Zero → ESP32)

| RP2040 Zero | ESP32       |
|-------------|-------------|
| GPIO 0 (TX) | RX pin      |
| GPIO 1 (RX) | TX pin      |
| GND         | GND         |

> **Note:** Do NOT connect 3.3V/5V between the boards — only TX, RX, and GND.

### Message Format

Messages arrive on the ESP32 as newline-terminated strings:

```
DM|SenderName|timestamp|message text
CH|ChannelName|timestamp|message text
```

**Examples:**
```
DM|Alice|1747044123|Hey, are you there?
CH|Alarm|1747044123|Motion detected at front door
```

- `timestamp` is Unix time (seconds since 1970-01-01)
- Lines are terminated with `\n`

### Controlling the Forward from the ESP32

Send commands over Serial (115200 baud) from the ESP32 to the RP2040 Zero:

| Command | Effect |
|---------|--------|
| `SET_CHANNEL\|Alarm\n` | Forward only the channel named "Alarm" |
| `SET_CHANNEL\|*\n` | Forward all channels |
| `SET_CHANNEL\|0\n` | Disable channel forwarding entirely |
| `SET_DM\|1\n` | Enable direct message forwarding |
| `SET_DM\|0\n` | Disable direct message forwarding |

**Arduino example (ESP32 side):**
```cpp
// Send on startup: forward channel "Alarm" and enable DMs
Serial2.println("SET_CHANNEL|Alarm");
Serial2.println("SET_DM|1");

// Read incoming messages
void loop() {
  if (Serial2.available()) {
    String line = Serial2.readStringUntil('\n');
    // Parse: split by '|'
    // line = "CH|Alarm|1747044123|Motion detected"
    // line = "DM|Alice|1747044123|Hello"
  }
}
```

### Changing the Default Channel/DM Setting

The startup defaults are set in `platformio.ini`. Edit the
`[env:diy_rp2040_zero_companion_radio_usb_fwd]` section:

```ini
-D WITH_SERIAL_FORWARD_CHANNEL='"Alarm"'   ; start with channel "Alarm" active
; remove the line above to start with channels disabled

-D WITH_SERIAL_FORWARD_DM=1               ; start with DM forwarding active
; remove the line above to start with DMs disabled
```

After changing the ini, rebuild and reflash the firmware.
The ESP32 can always override these settings at runtime via the commands above
without reflashing.

---

## Troubleshooting

**Radio not initializing:**
- Check SPI wiring (MISO/MOSI/SCK/CS)
- Make sure BUSY and RESET are connected correctly
- The TX LED (GPIO 7) blinks briefly on each transmission — useful for a quick sanity check

**Serial forward not receiving messages on ESP32:**
- Verify TX (GPIO 0) of RP2040 is connected to RX of ESP32
- Make sure both devices share a common GND
- Baud rate is 115200 on both sides
- Check that the correct channel name is set (names are case-sensitive)

**Build fails with "FIRMWARE_VERSION must be set":**
- You forgot to set the environment variable before running `build.py`
- See the build commands above
