
// calibracion validada, recuperacion de sensores, envio sin saltos
#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

// Normalizar angulos a [-180,180).
float wrap180(float x) {
  x = fmodf(x + 180.0f, 360.0f);
  if (x < 0) x += 360.0f;
  return x - 180.0f;
}

//  CONFIGURACION  
const char* WIFI_SSID = "TU_RED_WIFI";
const char* WIFI_PASS = "TU_PASSWORD";

// IP de la PC con Blender (misma red que la ESP32)
IPAddress IP_DESTINO(192, 168, 1, 100);

// Puerto UDP (del script de Blender)
const uint16_t PUERTO_DESTINO = 5005;

#define USAR_BROADCAST_LAN false

// Si no conecta al router en 15 s, crea el AP "ESP32-Brazo" (clave 12345678)
// y envia por broadcast a 192.168.4.255
#define USAR_AP_RESPALDO true

// SENSORES 
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

#define MPU_ANTEBRAZO_ADDR 0x68
#define MPU_BRAZO_ADDR     0x69

#define SERIAL_BAUD 115200
#define SAMPLE_PERIOD_MS 10        
#define COMPLEMENTARY_ALPHA 0.98f

#define MAX_CONSECUTIVE_FAILURES 5  
#define RECOVERY_PERIOD_MS 1000      

// Calibracion
// se calibra. 120 LSB 
#define CALIB_SAMPLES 300
#define CALIB_MAX_RANGE 120
#define CALIB_MAX_ATTEMPTS 3

// Registros MPU6050
static const uint8_t REG_PWR_MGMT_1   = 0x6B;
static const uint8_t REG_SMPLRT_DIV   = 0x19;
static const uint8_t REG_CONFIG       = 0x1A;
static const uint8_t REG_GYRO_CONFIG  = 0x1B;
static const uint8_t REG_ACCEL_CONFIG = 0x1C;
static const uint8_t REG_ACCEL_XOUT_H = 0x3B;
static const uint8_t REG_WHO_AM_I     = 0x75;

WiFiUDP udp;

class MPU6050Motion {
private:
  TwoWire* bus;
  uint8_t address;
  const char* sensorName;

  float gyroOffsetX = 0.0f;
  float gyroOffsetY = 0.0f;
  float gyroOffsetZ = 0.0f;

  float roll = 0.0f;
  float pitch = 0.0f;
  float zeroRoll = 0.0f;
  float zeroPitch = 0.0f;

  uint32_t lastUpdateUs = 0;
  bool initialized = false;
  uint8_t consecutiveFailures = 0;

  bool writeRegister(uint8_t reg, uint8_t value) {
    bus->beginTransmission(address);
    bus->write(reg);
    bus->write(value);
    return bus->endTransmission() == 0;
  }

  bool readRegisters(uint8_t startReg, uint8_t* buffer, uint8_t length) {
    bus->beginTransmission(address);
    bus->write(startReg);
    if (bus->endTransmission(false) != 0) return false;

    uint8_t received = bus->requestFrom(address, length, (uint8_t)true);
    if (received != length) {
      while (bus->available()) bus->read();
      return false;
    }
    for (uint8_t i = 0; i < length; i++) buffer[i] = bus->read();
    return true;
  }

  static int16_t combineBytes(uint8_t highByte, uint8_t lowByte) {
    return (int16_t)((highByte << 8) | lowByte);
  }

  bool readRaw(int16_t& ax, int16_t& ay, int16_t& az,
               int16_t& gx, int16_t& gy, int16_t& gz) {
    uint8_t data[14];
    if (!readRegisters(REG_ACCEL_XOUT_H, data, 14)) return false;
    ax = combineBytes(data[0], data[1]);
    ay = combineBytes(data[2], data[3]);
    az = combineBytes(data[4], data[5]);
    gx = combineBytes(data[8], data[9]);
    gy = combineBytes(data[10], data[11]);
    gz = combineBytes(data[12], data[13]);
    return true;
  }

  void anglesFromAccel(float fax, float fay, float faz) {
    roll  = atan2f(fay, faz) * 180.0f / PI;
    pitch = atan2f(-fax, sqrtf(fay * fay + faz * faz)) * 180.0f / PI;
  }

  bool resetAnglesFromAccel() {
    int16_t ax, ay, az, gx, gy, gz;
    if (!readRaw(ax, ay, az, gx, gy, gz)) return false;
    anglesFromAccel(ax / 16384.0f, ay / 16384.0f, az / 16384.0f);
    return true;
  }

public:
  MPU6050Motion(TwoWire& wireBus, uint8_t i2cAddress, const char* name)
    : bus(&wireBus), address(i2cAddress), sensorName(name) {}

  bool begin(bool verbose = true) {
    initialized = false;
    uint8_t who = 0;
    if (!readRegisters(REG_WHO_AM_I, &who, 1)) {
      if (verbose) {
        Serial.print(F("# ERROR I2C: "));
        Serial.println(sensorName);
      }
      return false;
    }
    // comparten los registros usados aqui, asi que se aceptan con aviso
    bool idOk = (who == 0x68);
    bool idClone = (who == 0x70 || who == 0x71 || who == 0x72 || who == 0x73);
    if (!idOk && !idClone) {
      if (verbose) {
        Serial.print(F("# ID inesperado en "));
        Serial.print(sensorName);
        Serial.print(F(": 0x"));
        if (who < 0x10) Serial.print('0');
        Serial.println(who, HEX);
      }
      return false;
    }
    if (idClone && verbose) {
      Serial.print(F("# Aviso: "));
      Serial.print(sensorName);
      Serial.println(F(" no es un MPU6050 original (clon/MPU6500); se intenta usar igual"));
    }

    if (!writeRegister(REG_PWR_MGMT_1, 0x00)) return false;
    delay(100);
    if (!writeRegister(REG_CONFIG, 0x03))        return false;  
    if (!writeRegister(REG_SMPLRT_DIV, 0x09))    return false;
    if (!writeRegister(REG_GYRO_CONFIG, 0x00))   return false;  
    if (!writeRegister(REG_ACCEL_CONFIG, 0x00))  return false;  

    initialized = true;
    consecutiveFailures = 0;
    if (!resetAnglesFromAccel() && verbose) {
      Serial.print(F("# Advertencia: primera lectura de "));
      Serial.print(sensorName);
      Serial.println(F(" fallo; angulos iniciales en 0"));
    }
    lastUpdateUs = micros();

    if (verbose) {
      Serial.print(F("# Detectado: "));
      Serial.print(sensorName);
      Serial.print(F(" en 0x"));
      if (address < 0x10) Serial.print('0');
      Serial.println(address, HEX);
    }
    return true;
  }

  // Calibra el giroscopio promediando muestras valida que el sensor estuviera
  // quieto y repite hasta CALIB_MAX_ATTEMPTS veces
  // El sensor sigue operativo, devuelve true si la calibracion fue limpia
  bool calibrateGyroscope() {
    if (!initialized) return false;
    Serial.print(F("# Calibrando giroscopio: "));
    Serial.println(sensorName);
    Serial.println(F("# Mantener el sensor completamente quieto..."));

    bool clean = false;
    for (uint8_t attempt = 1; attempt <= CALIB_MAX_ATTEMPTS; attempt++) {
      int32_t sumGx = 0, sumGy = 0, sumGz = 0;
      int32_t sumAx = 0, sumAy = 0, sumAz = 0;
      int16_t minG[3] = { 32767, 32767, 32767 };
      int16_t maxG[3] = { -32768, -32768, -32768 };
      uint16_t valid = 0;

      for (uint16_t i = 0; i < CALIB_SAMPLES; i++) {
        int16_t ax, ay, az, gx, gy, gz;
        if (readRaw(ax, ay, az, gx, gy, gz)) {
          sumGx += gx; sumGy += gy; sumGz += gz;
          sumAx += ax; sumAy += ay; sumAz += az;
          int16_t g[3] = { gx, gy, gz };
          for (uint8_t k = 0; k < 3; k++) {
            if (g[k] < minG[k]) minG[k] = g[k];
            if (g[k] > maxG[k]) maxG[k] = g[k];
          }
          valid++;
        }
        delay(10);
      }

      if (valid < CALIB_SAMPLES * 0.8f) {
        Serial.print(F("# Calibracion: demasiadas lecturas fallidas en "));
        Serial.println(sensorName);
        continue;  // reintentar
      }

      // Los offsets se guardan siempre, si el intento fue sucio(malo) se sobrescriben en el siguiente
      gyroOffsetX = (float)sumGx / valid;
      gyroOffsetY = (float)sumGy / valid;
      gyroOffsetZ = (float)sumGz / valid;
      anglesFromAccel(sumAx / (float)valid / 16384.0f,
                      sumAy / (float)valid / 16384.0f,
                      sumAz / (float)valid / 16384.0f);

      int16_t range = 0;
      for (uint8_t k = 0; k < 3; k++) {
        int16_t r = maxG[k] - minG[k];
        if (r > range) range = r;
      }
      if (range <= CALIB_MAX_RANGE) {
        clean = true;
        break;
      }
      Serial.print(F("# Se detecto movimiento durante la calibracion (intento "));
      Serial.print(attempt);
      Serial.print(F("/"));
      Serial.print(CALIB_MAX_ATTEMPTS);
      Serial.println(F(")"));
    }

    Serial.print(clean ? F("# Calibracion lista: ") : F("# Calibracion con aviso (posible movimiento): "));
    Serial.println(sensorName);
    lastUpdateUs = micros();
    return clean;
  }

  void syncTime() { lastUpdateUs = micros(); }

  bool update() {
    if (!initialized) return false;

    int16_t rawAx, rawAy, rawAz, rawGx, rawGy, rawGz;
    if (!readRaw(rawAx, rawAy, rawAz, rawGx, rawGy, rawGz)) {
      if (consecutiveFailures < 255) consecutiveFailures++;
      return false;
    }

    uint32_t nowUs = micros();
    float dt = (nowUs - lastUpdateUs) / 1000000.0f;
    lastUpdateUs = nowUs;
    if (dt <= 0.0f || dt > 0.1f) {
      if (consecutiveFailures < 255) consecutiveFailures++;
      return false;
    }
    consecutiveFailures = 0;

    float ax = rawAx / 16384.0f;
    float ay = rawAy / 16384.0f;
    float az = rawAz / 16384.0f;

    float gx = (rawGx - gyroOffsetX) / 131.0f;
    float gy = (rawGy - gyroOffsetY) / 131.0f;

    float accelRoll  = atan2f(ay, az) * 180.0f / PI;
    float accelPitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / PI;

    float rollError = wrap180(accelRoll - roll);

    // constante de tiempo (alpha = .98 a 10 ms)
    float alpha = powf(COMPLEMENTARY_ALPHA, dt / 0.01f);

    roll = wrap180(roll + alpha * gx * dt + (1.0f - alpha) * rollError);

    pitch = alpha * (pitch + gy * dt) + (1.0f - alpha) * accelPitch;
    pitch = wrap180(pitch);

    return true;
  }

  void setZero() {
    zeroRoll  = roll;
    zeroPitch = pitch;
  }

  bool isInitialized() const { return initialized; }

  bool isHealthy() const {
    return initialized && consecutiveFailures < MAX_CONSECUTIVE_FAILURES;
  }

  float getRoll()  const { return wrap180(roll - zeroRoll); }
  float getPitch() const { return wrap180(pitch - zeroPitch); }

  const char* getName() const { return sensorName; }
};

// OBJETOS
MPU6050Motion antebrazo(Wire, MPU_ANTEBRAZO_ADDR, "ANTEBRAZO_MANO");
MPU6050Motion brazoSuperior(Wire, MPU_BRAZO_ADDR, "BRAZO_SUPERIOR");

uint32_t lastSampleMs = 0;
bool wifiOK = false;
bool modoAP = false;
bool udpReady = false;
bool debugSerial = false;

//  WiFi 
bool conectarWiFi() {
  Serial.print(F("# Conectando a "));
  Serial.print(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);   
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  uint32_t inicio = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - inicio < 15000) {
    delay(250);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("# WiFi conectado. IP de la ESP32: "));
    Serial.println(WiFi.localIP());
    if (USAR_BROADCAST_LAN) IP_DESTINO = WiFi.broadcastIP();
    Serial.print(F("# Enviando a "));
    Serial.print(IP_DESTINO);
    Serial.print(F(":"));
    Serial.println(PUERTO_DESTINO);
    return true;
  }

  Serial.println(F("# No se pudo conectar al router."));
#if USAR_AP_RESPALDO
  Serial.println(F("# Creando punto de acceso 'ESP32-Brazo' (clave: 12345678)"));
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  modoAP = WiFi.softAP("ESP32-Brazo", "12345678");
  if (!modoAP) return false;
  IP_DESTINO = IPAddress(192, 168, 4, 255);
  Serial.print(F("# IP de la ESP32 (AP): "));
  Serial.println(WiFi.softAPIP());
  Serial.print(F("# La PC debe unirse a 'ESP32-Brazo'; destino fijado a "));
  Serial.println(IP_DESTINO);
  return true;
#else
  return false;
#endif
}

void asegurarWiFi() {
  if (modoAP) {
    wifiOK = WiFi.softAPgetStationNum() > 0;
    return;
  }
  if (WiFi.status() == WL_CONNECTED) {
    wifiOK = true;
    return;
  }
  wifiOK = false;
  static uint32_t ultimoIntentoMs = 0;
  if (millis() - ultimoIntentoMs > 10000) {
    ultimoIntentoMs = millis();
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}

//  Salida UDP 
void enviarUDP() {
  //   sw1 = roll del BRAZO SUPERIOR
  //   ab1 = pitch del BRAZO SUPERIOR
  //   sw2 = roll del ANTEBRAZO  (Blender calcula el codo: wrap180(sw2 - sw1))
  char buffer[64];
  snprintf(buffer, sizeof(buffer), "%.2f,%.2f,%.2f",
           brazoSuperior.getRoll(),
           brazoSuperior.getPitch(),
           antebrazo.getRoll());

  if (udp.beginPacket(IP_DESTINO, PUERTO_DESTINO)) {
    udp.write((const uint8_t*)buffer, strlen(buffer));
    udp.endPacket();
  }
}

void zeroBothSensors() {
  antebrazo.setZero();
  brazoSuperior.setZero();
  Serial.println(F("# CERO establecido en la postura actual"));
}

void recalibrarTodo() {
  if (antebrazo.isInitialized())     antebrazo.calibrateGyroscope();
  if (brazoSuperior.isInitialized()) brazoSuperior.calibrateGyroscope();
  zeroBothSensors();
  antebrazo.syncTime();
  brazoSuperior.syncTime();
}

// si un sensor se cae (cable suelto,x y razon) o no estaba al arrancar,
// se intenta reiniciar cada RECOVERY_PERIOD_MS sin calibrar (quedan offsets previos o 0)
void mantenerSensor(MPU6050Motion& s, uint32_t& ultimoIntentoMs) {
  if (s.isHealthy()) return;
  uint32_t ahora = millis();
  if (ahora - ultimoIntentoMs < RECOVERY_PERIOD_MS) return;
  ultimoIntentoMs = ahora;
  if (s.begin(false)) {
    Serial.print(F("# Sensor recuperado: "));
    Serial.println(s.getName());
    s.setZero();
  }
}

void manejarSerial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == 'z' || c == 'Z') {
      zeroBothSensors();
    } else if (c == 'c' || c == 'C') {
      recalibrarTodo();
    } else if (c == 'd' || c == 'D') {
      debugSerial = !debugSerial;
      Serial.print(F("# Debug "));
      Serial.println(debugSerial ? F("ON") : F("OFF"));
    }
  }
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  uint32_t t0 = millis();
  while (!Serial && (millis() - t0) < 3000) { /* espera CDC USB */ }
  delay(50);

  Serial.println();
  Serial.println(F("# === MONITOR DE MOVIMIENTO: 2x MPU6050 -> WiFi UDP (Blender) ==="));
  Serial.print(F("# ESP32-S3 | I2C SDA="));
  Serial.print(I2C_SDA_PIN);
  Serial.print(F(" SCL="));
  Serial.println(I2C_SCL_PIN);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(400000);
  Wire.setTimeOut(20);  

  bool okAnte = antebrazo.begin();
  bool okBrazo = brazoSuperior.begin();
  if (!okAnte || !okBrazo) {
    Serial.println(F("# Revisar alimentacion, SDA/SCL y direccion AD0 (se reintentara solo)."));
  }

  if (okAnte)  antebrazo.calibrateGyroscope();
  if (okBrazo) brazoSuperior.calibrateGyroscope();
  if (okAnte)  antebrazo.setZero();
  if (okBrazo) brazoSuperior.setZero();

  // la calibracion no sufre interrupciones de red
  wifiOK = conectarWiFi();
  udpReady = udp.begin(5006);
  if (!udpReady) Serial.println(F("# ERROR al iniciar UDP"));

  // Tras el setup lento (calibracion + WiFi) se reinicia la referencia de tiempo
  antebrazo.syncTime();
  brazoSuperior.syncTime();

  Serial.println(F("# Listo. Comandos: z=cero, c=recalibrar, d=debug"));
  Serial.print(F("# Paquete UDP: brazo_roll,brazo_pitch,antebrazo_roll -> puerto "));
  Serial.println(PUERTO_DESTINO);
}

void loop() {
  static uint32_t recAnte = 0, recBrazo = 0, ultimoDebugMs = 0;

  manejarSerial();
  asegurarWiFi();
  mantenerSensor(antebrazo, recAnte);
  mantenerSensor(brazoSuperior, recBrazo);

  uint32_t nowMs = millis();
  if (nowMs - lastSampleMs < SAMPLE_PERIOD_MS) return;
  lastSampleMs += SAMPLE_PERIOD_MS;                      
  if (nowMs - lastSampleMs >= SAMPLE_PERIOD_MS) lastSampleMs = nowMs;  

  antebrazo.update();
  brazoSuperior.update();

  // Se envia mientras ambos sensores esten funcioando, un fallo aislado de lectura
  // no corta el flujo (el angulo conserva su ultimo valor y Blender no da saltos)
  if (wifiOK && udpReady && antebrazo.isHealthy() && brazoSuperior.isHealthy()) {
    enviarUDP();
  }

  if (debugSerial && nowMs - ultimoDebugMs >= 100) {
    ultimoDebugMs = nowMs;
    Serial.print(F("# brazo_roll="));
    Serial.print(brazoSuperior.getRoll(), 2);
    Serial.print(F(" brazo_pitch="));
    Serial.print(brazoSuperior.getPitch(), 2);
    Serial.print(F(" ante_roll="));
    Serial.print(antebrazo.getRoll(), 2);
    Serial.print(F(" wifi="));
    Serial.println(wifiOK ? 1 : 0);
  }
}
