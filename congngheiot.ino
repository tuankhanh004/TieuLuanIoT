#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D4       // Chân SDA của RFID (GPIO2)
#define RST_PIN D3      // Chân RST của RFID (GPIO0)
#define BUZZER_PIN D8   // Chân còi (GPIO15)
#define GAS_SENSOR A0   // Cảm biến khí gas (analog)

MFRC522 rfid(SS_PIN, RST_PIN);
int failedAttempts = 0;
const int maxFailedAttempts = 3;

// UID hợp lệ (thẻ đúng)
byte validUID[] = {0x2C, 0x2F, 0x16, 0x05}; // <--- FIX: thiếu dấu ;

bool isSameUID(byte *a, byte *b, byte len) {
  for (byte i = 0; i < len; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("🔐 Hệ thống kiểm soát RFID + Gas đã khởi động");
}

void loop() {
  // Đọc cảm biến khí gas
  int gasValue = analogRead(GAS_SENSOR);
  Serial.print("Nồng độ khí gas: ");
  Serial.println(gasValue);
  delay(1000);

  // Nếu nồng độ vượt ngưỡng cảnh báo
  if (gasValue > 400 ) {
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("⚠️ Cảnh báo: Phát hiện khí gas!");
    delay(500); // Delay tránh hú liên tục
    return;
  } else {
    digitalWrite(BUZZER_PIN, LOW);
  }

  // Kiểm tra nếu có thẻ mới
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  byte readUID[4];
  for (byte i = 0; i < 4; i++) {
    readUID[i] = rfid.uid.uidByte[i];
  }

  // So sánh UID
  if (isSameUID(readUID, validUID, 4)) {
    Serial.println("✅ Truy cập hợp lệ!");
    failedAttempts = 0;
    beepSuccess();
  } else {
    Serial.println("❌ Truy cập bị từ chối!");
    failedAttempts++;
    beepError();

    if (failedAttempts >= maxFailedAttempts) {
      beepAlert();
    }
  }

  // Ngắt kết nối thẻ
  rfid.PICC_HaltA();
  delay(500);
}

// Còi báo thành công
void beepSuccess() {
  tone(BUZZER_PIN, 1500, 200);
  delay(200);
}

// Còi báo sai
void beepError() {
  for (int i = 0; i < 2; i++) {
    tone(BUZZER_PIN, 1000, 200);
    delay(300);
  }
}

// Còi cảnh báo sai quá số lần
void beepAlert() {
  for (int i = 0; i < 5; i++) {
    tone(BUZZER_PIN, 800, 400);
    delay(500);
  }
}
