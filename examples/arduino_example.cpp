#include <Arduino.h>
#include "paa5102e1Array.hpp"
#include <SPI.h>
#include <type_traits>

#define LED2LD_TH 0x500
#define LED2LED_TH 0x700

#define SS_SENSOR_1 5
#define LDP_ENL_SENSOR_1 4


#define NUM_SENSORS 1

template <typename T>
class AverageCircularBuffer {
    static_assert(std::is_arithmetic<T>::value, "T must be an arithmetic type");

private:
    std::unique_ptr<T[]> buffer;
    size_t head;
    size_t count;
    size_t maxSize;
    T sum;

public:
    explicit AverageCircularBuffer(size_t maxSize)
        : head(0), count(0), maxSize(maxSize), sum(0.0) {
          buffer = std::make_unique<T[]>(maxSize);
    }

    ~AverageCircularBuffer() {
    }

    void push(T value) {
        if (count == maxSize) {
            sum -= buffer[head];
        } else {
            count++;
        }
        sum += value;
        buffer[head] = value;
        head = (head + 1) % maxSize;
    }

    T getAverage() const {
        return count == 0 ? 0.0 : sum / count;
    }

    T getSum() const {
        return sum;
    }
};

std::unique_ptr<PAA5102E1Array<NUM_SENSORS>> sensors;

int16_t pos_x = 0, pos_y = 0;
AverageCircularBuffer<uint16_t> avg_fiqs[NUM_SENSORS] = {
  AverageCircularBuffer<uint16_t>(8),
//  AverageCircularBuffer<uint16_t>(8)
};
size_t loops = 0;

void error_loop(void * error) {
  auto error_ = (PAA5102E1Error *) error;
  Serial.println("Error: \n" + error_->toString());
  while (true) {
    delay(1000);
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(MOSI, OUTPUT);
  pinMode(MISO, INPUT);
  pinMode(SCK, OUTPUT);

  delay(5000);
  Serial.println("Started");
  
  PAA5102E1PinSetting sensor_pins[NUM_SENSORS] = {
    {SS_SENSOR_1, LDP_ENL_SENSOR_1},
  //  {SS_SENSOR_2, LDP_ENL_SENSOR_2}
  };
  const auto spi_settings = SPISettings(1000000, MSBFIRST, SPI_MODE3);
  sensors = std::make_unique<PAA5102E1Array<NUM_SENSORS>>(spi_settings, sensor_pins);
  
#if BOARD == BOARD_OPENCR
        // define the SPI only one time, as it leads to some voltage drop on initialization
        SPI.begin();
        SPI.beginTransaction(spi_settings);
#endif

  auto res = sensors->init(1);
  if (res.hasError) {
    error_loop(&res);
  }

  Serial.println("DONE");

}

void loop() {
  if (++loops == 8) {
    loops = 0;

    for (int i = 0; i < NUM_SENSORS; ++i) {
      avg_fiqs[i].push(sensors->getSensor(i).value->readImageQuality());

      Light_Source currentMode = sensors->getSensor(i).value->getLightSource();
      uint16_t avg = avg_fiqs[i].getSum();

      if (currentMode == Light_Source::LASER && avg < LED2LD_TH) {
        sensors->getSensor(i).value->setLightSource(Light_Source::LED);
      } else if (currentMode == Light_Source::LED && avg < LED2LED_TH) {
        sensors->getSensor(i).value->setLightSource(Light_Source::LASER);
      }
    }
  }

  for (int i = 0; i < NUM_SENSORS; ++i) {
    Light_Source mode = sensors->getSensor(i).value->getLightSource();
    String modeStr = (mode == Light_Source::LED) ? "LED" : "Laser";

    int16_t dx = 0, dy = 0;
    if (sensors->getSensor(i).value->hasMoved()) {
      dx = sensors->getSensor(i).value->readDeltaX();
      dy = sensors->getSensor(i).value->readDeltaY();
    }

    Serial.print("Sensor");
    Serial.print(i);
    Serial.print("\tOP_Mode: ");
    Serial.print(modeStr);
    Serial.print(", DeltaXY: (");
    Serial.print(dx);
    Serial.print(", ");
    Serial.print(dy);
    Serial.println(")");
  }

  Serial.println();
  delay(200);
}

