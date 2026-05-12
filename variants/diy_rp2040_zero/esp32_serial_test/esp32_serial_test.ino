/**
 * MeshCore SerialForward — ESP32 Test Sketch
 *
 * Wiring (RP2040 Zero <-> ESP32):
 *   RP2040 GPIO 0 (TX)  -->  ESP32 RX_PIN
 *   RP2040 GPIO 1 (RX)  <--  ESP32 TX_PIN
 *   GND                 ---  GND
 *
 * Open the Arduino Serial Monitor at 115200 baud.
 * Incoming MeshCore messages are printed automatically.
 * Type commands and press Enter to send them to the RP2040.
 */

// --- Pin configuration ---
#define MESH_SERIAL       Serial2
#define MESH_SERIAL_RX    16    // ESP32 pin connected to RP2040 TX (GPIO 0)
#define MESH_SERIAL_TX    17    // ESP32 pin connected to RP2040 RX (GPIO 1)
#define MESH_BAUD         115200

// -------------------------

static char rx_buf[256];
static uint8_t rx_pos = 0;
static char tx_buf[128];
static uint8_t tx_pos = 0;

void printMenu() {
  Serial.println();
  Serial.println("┌─────────────────────────────────────────┐");
  Serial.println("│  MeshCore SerialForward Test — ESP32    │");
  Serial.println("├─────────────────────────────────────────┤");
  Serial.println("│  Shortcuts:                             │");
  Serial.println("│   1  →  SET_CHANNEL|Alarm               │");
  Serial.println("│   2  →  SET_CHANNEL|* (all channels)    │");
  Serial.println("│   3  →  SET_CHANNEL|0 (disable)         │");
  Serial.println("│   4  →  SET_DM|1      (enable DMs)      │");
  Serial.println("│   5  →  SET_DM|0      (disable DMs)     │");
  Serial.println("│                                         │");
  Serial.println("│  Or type any command and press Enter:   │");
  Serial.println("│   SET_CHANNEL|MyChannel                 │");
  Serial.println("│   SET_DM|1                              │");
  Serial.println("└─────────────────────────────────────────┘");
  Serial.println();
}

void handleShortcut(char c) {
  const char* cmd = nullptr;
  switch (c) {
    case '1': cmd = "SET_CHANNEL|Alarm"; break;
    case '2': cmd = "SET_CHANNEL|*";     break;
    case '3': cmd = "SET_CHANNEL|0";     break;
    case '4': cmd = "SET_DM|1";          break;
    case '5': cmd = "SET_DM|0";          break;
    default:  return;
  }
  MESH_SERIAL.println(cmd);
  Serial.print("[SENT] ");
  Serial.println(cmd);
}

void printMessage(const char* line) {
  // Format:  DM|SenderName|timestamp|text
  //          CH|ChannelName|timestamp|text
  char buf[256];
  strncpy(buf, line, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  char* type      = strtok(buf, "|");
  char* source    = strtok(nullptr, "|");
  char* timestamp = strtok(nullptr, "|");
  char* text      = strtok(nullptr, "\0");

  if (!type || !source || !timestamp || !text) {
    Serial.print("[RAW] ");
    Serial.println(line);
    return;
  }

  Serial.print("[");
  Serial.print(type);
  Serial.print("] ");
  Serial.print(source);
  Serial.print("  |  ts:");
  Serial.print(timestamp);
  Serial.print("  |  ");
  Serial.println(text);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  MESH_SERIAL.begin(MESH_BAUD, SERIAL_8N1, MESH_SERIAL_RX, MESH_SERIAL_TX);

  printMenu();
  Serial.println("Waiting for messages from RP2040...");
  Serial.println();
}

void loop() {
  // --- Receive from RP2040 ---
  while (MESH_SERIAL.available()) {
    char c = (char)MESH_SERIAL.read();
    if (c == '\n') {
      rx_buf[rx_pos] = '\0';
      if (rx_pos > 0) printMessage(rx_buf);
      rx_pos = 0;
    } else if (c != '\r' && rx_pos < sizeof(rx_buf) - 1) {
      rx_buf[rx_pos++] = c;
    }
  }

  // --- Send from Serial Monitor to RP2040 ---
  while (Serial.available()) {
    char c = (char)Serial.read();

    if (c == '\n' || c == '\r') {
      if (tx_pos > 0) {
        tx_buf[tx_pos] = '\0';

        // Single-character shortcuts
        if (tx_pos == 1 && tx_buf[0] >= '1' && tx_buf[0] <= '5') {
          handleShortcut(tx_buf[0]);
        } else if (tx_buf[0] == '?' || tx_buf[0] == 'h') {
          printMenu();
        } else {
          MESH_SERIAL.println(tx_buf);
          Serial.print("[SENT] ");
          Serial.println(tx_buf);
        }
        tx_pos = 0;
      }
    } else if (tx_pos < sizeof(tx_buf) - 1) {
      tx_buf[tx_pos++] = c;
    }
  }
}
