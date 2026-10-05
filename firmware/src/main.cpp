// Signal Lost: exploratory firmware reference, not a tested physical build.
// Target: Adafruit QT Py ESP32-S3 (4 MB flash, 2 MB PSRAM), USB powered.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <driver/i2s.h>
#include <math.h>
#include <cstring>

namespace {
constexpr uint8_t kEncoderA = A0;
constexpr uint8_t kEncoderB = A1;
constexpr uint8_t kTouch = A2;
constexpr uint8_t kEncoderButton = A3;
constexpr uint8_t kI2sBclk = TX;
constexpr uint8_t kI2sLrc = RX;
constexpr uint8_t kI2sData = SCK;
constexpr uint8_t kLedSignal = MOSI;
constexpr uint8_t kLedTuning = MISO;
constexpr uint32_t kSampleRate = 16000;
constexpr uint8_t kStationCount = 4;
constexpr uint8_t kSecretStation = 4;
constexpr uint8_t kScreenAddress = 0x3C;

Adafruit_SSD1306 screen(128, 32, &Wire1, -1);
bool screenReady = false;
volatile uint8_t station = 0;
volatile bool playing = true;
volatile bool secretUnlocked = false;

const char* const kNames[] = {
    "NIGHT WEATHER", "DISTRESS", "NUMBERS", "DEEP SPACE", "UNKNOWN"};
const char* const kFrequency[] = {
    "88.1", "91.7", "104.3", "117.9", "???.?"};

uint8_t encoderPrevious = 0;
int8_t encoderAccumulator = 0;
bool buttonPrevious = HIGH;
uint32_t buttonChangedAt = 0;
bool buttonStable = HIGH;
uint32_t touchBaseline = 0;
uint8_t touchSamples = 0;
bool touchLatched = false;
uint32_t lastDrawAt = 0;
uint32_t lastDebugAt = 0;

// An LCG provides reproducible static. This is decorative audio, not real radio reception.
uint32_t noiseState = 0x48616C66;
float phaseA = 0.0f;
float phaseB = 0.0f;
uint32_t audioSamples = 0;

int16_t nextAudioSample() {
  const uint8_t current = station;
  const float t = static_cast<float>(audioSamples++) / kSampleRate;
  noiseState = noiseState * 1664525u + 1013904223u;
  const float noise = static_cast<int32_t>(noiseState) / 2147483648.0f;
  float value = 0.0f;
  float frequencyA = 0.0f;
  float frequencyB = 0.0f;

  switch (current) {
    case 0: { // Night weather: low drone with a soft static bed.
      frequencyA = 110.0f;
      value = 0.30f * sinf(phaseA) + 0.10f * noise;
      break;
    }
    case 1: { // Distress: three short pulses, three long, three short.
      constexpr char sos[] = "101010111011101110101010000000";
      const uint32_t slot = (audioSamples / (kSampleRate / 10)) % (sizeof(sos) - 1);
      frequencyA = 720.0f;
      value = sos[slot] == '1' ? 0.40f * sinf(phaseA) : 0.04f * noise;
      break;
    }
    case 2: { // Numbers station: regular two-tone coded pulses.
      const uint32_t slot = (audioSamples / (kSampleRate / 8)) % 16;
      frequencyA = (slot & 1u) ? 880.0f : 660.0f;
      value = (slot % 4u == 3u) ? 0.02f * noise : 0.28f * sinf(phaseA);
      break;
    }
    case 3: { // Deep space: slow beat between two nearby tones.
      frequencyA = 165.0f;
      frequencyB = 168.0f;
      value = 0.22f * sinf(phaseA) + 0.22f * sinf(phaseB) + 0.05f * noise;
      break;
    }
    default: { // Hidden station: alternating signal and silence.
      frequencyA = 444.0f + 60.0f * sinf(t * 0.55f);
      const bool gate = ((audioSamples / (kSampleRate / 3)) % 6u) < 4u;
      value = gate ? 0.33f * sinf(phaseA) : 0.0f;
      break;
    }
  }

  phaseA += 2.0f * PI * frequencyA / kSampleRate;
  phaseB += 2.0f * PI * frequencyB / kSampleRate;
  if (phaseA >= 2.0f * PI) phaseA -= 2.0f * PI;
  if (phaseB >= 2.0f * PI) phaseB -= 2.0f * PI;
  // Software volume cap protects the small speaker during this draft.
  const float scaled = playing ? value * 0.45f : 0.0f;
  return static_cast<int16_t>(constrain(scaled, -1.0f, 1.0f) * 32767.0f);
}

void audioTask(void*) {
  int16_t samples[256];
  for (;;) {
    for (auto& sample : samples) sample = nextAudioSample();
    size_t written = 0;
    i2s_write(I2S_NUM_0, samples, sizeof(samples), &written, portMAX_DELAY);
  }
}

bool beginAudio() {
  i2s_config_t config{};
  config.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX);
  config.sample_rate = kSampleRate;
  config.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  config.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  config.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  config.intr_alloc_flags = 0;
  config.dma_buf_count = 6;
  config.dma_buf_len = 256;
  config.use_apll = false;
  config.tx_desc_auto_clear = true;
  config.fixed_mclk = 0;

  i2s_pin_config_t pins{};
  pins.bck_io_num = kI2sBclk;
  pins.ws_io_num = kI2sLrc;
  pins.data_out_num = kI2sData;
  pins.data_in_num = I2S_PIN_NO_CHANGE;
  if (i2s_driver_install(I2S_NUM_0, &config, 0, nullptr) != ESP_OK) return false;
  if (i2s_set_pin(I2S_NUM_0, &pins) != ESP_OK) return false;
  xTaskCreatePinnedToCore(audioTask, "radio-audio", 4096, nullptr, 2, nullptr, 0);
  return true;
}

void scanEncoder() {
  static constexpr int8_t transitions[16] = {
      0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
  const uint8_t now = (digitalRead(kEncoderA) << 1) | digitalRead(kEncoderB);
  const uint8_t index = (encoderPrevious << 2) | now;
  encoderPrevious = now;
  encoderAccumulator += transitions[index];
  if (encoderAccumulator >= 4 || encoderAccumulator <= -4) {
    const int direction = encoderAccumulator > 0 ? 1 : -1;
    encoderAccumulator = 0;
    station = (station + direction + kStationCount) % kStationCount;
    Serial.printf("Station %u: %s\n", station, kNames[station]);
  }
}

void scanButton() {
  const bool now = digitalRead(kEncoderButton);
  if (now != buttonPrevious) buttonChangedAt = millis();
  buttonPrevious = now;
  if (millis() - buttonChangedAt > 30 && now != buttonStable) {
    buttonStable = now;
    if (now == LOW) playing = !playing;
  }
}

void scanTouch() {
  const uint32_t raw = touchRead(kTouch);
  if (touchBaseline == 0) touchBaseline = raw;
  const uint32_t difference = raw > touchBaseline ? raw - touchBaseline : touchBaseline - raw;
  const uint32_t threshold = touchBaseline / 7 > 20 ? touchBaseline / 7 : 20;
  if (difference > threshold) {
    if (touchSamples < 8) ++touchSamples;
  } else {
    touchSamples = 0;
    touchLatched = false;
    touchBaseline = (touchBaseline * 63u + raw) / 64u;
  }
  if (touchSamples >= 4 && !touchLatched) {
    touchLatched = true;
    secretUnlocked = true;
    station = kSecretStation;
    playing = true;
    Serial.printf("Secret touch: raw=%lu baseline=%lu\n",
                  static_cast<unsigned long>(raw),
                  static_cast<unsigned long>(touchBaseline));
  }
  if (millis() - lastDebugAt >= 1000) {
    lastDebugAt = millis();
    Serial.printf("Touch raw=%lu baseline=%lu threshold=%lu\n",
                  static_cast<unsigned long>(raw),
                  static_cast<unsigned long>(touchBaseline),
                  static_cast<unsigned long>(threshold));
  }
}

void drawDisplay() {
  if (!screenReady || millis() - lastDrawAt < 80) return;
  lastDrawAt = millis();
  screen.clearDisplay();
  screen.setTextSize(1);
  screen.setTextColor(SSD1306_WHITE);
  screen.setCursor(0, 0);
  screen.print("SIGNAL LOST   FM ");
  screen.print(kFrequency[station]);
  screen.setTextSize(strlen(kNames[station]) > 10 ? 1 : 2);
  screen.setCursor(0, 12);
  screen.print(kNames[station]);
  screen.display();
}

void updateLeds() {
  const uint32_t now = millis();
  digitalWrite(kLedTuning, ((now / 120) % 2u) == 0u);
  digitalWrite(kLedSignal, playing && (((now / 300) % 3u) != 0u));
}
} // namespace

void setup() {
  Serial.begin(115200);
  pinMode(kEncoderA, INPUT_PULLUP);
  pinMode(kEncoderB, INPUT_PULLUP);
  pinMode(kEncoderButton, INPUT_PULLUP);
  pinMode(kLedSignal, OUTPUT);
  pinMode(kLedTuning, OUTPUT);
  encoderPrevious = (digitalRead(kEncoderA) << 1) | digitalRead(kEncoderB);

  Wire1.begin(SDA1, SCL1);
  screenReady = screen.begin(SSD1306_SWITCHCAPVCC, kScreenAddress);
  if (!screenReady) Serial.println("OLED not found; continuing without display");
  if (!beginAudio()) Serial.println("I2S setup failed");
  Serial.println("Signal Lost exploratory firmware started");
}

void loop() {
  scanEncoder();
  scanButton();
  scanTouch();
  drawDisplay();
  updateLeds();
  delay(2);
}
