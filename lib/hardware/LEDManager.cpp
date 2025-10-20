// hardware/LEDManager.cpp
#include "./LEDManager.h"
#include <string>
#include <tuple>
#include <unordered_map>

namespace CloudMouse {

LEDManager::LEDManager()
  : strip(NUM_LEDS, DATA_PIN, NEO_GRB + NEO_KHZ800) {
  // Constructor
}

void LEDManager::init() {
  Serial.println("💡 Initializing LEDManager...");

  // Initialize hardware
  strip.begin();
  strip.setBrightness(currentBrightness);
  resetAllLEDs();
  strip.show();

  // Create event queue
  ledEventQueue = xQueueCreate(LED_QUEUE_SIZE, sizeof(LEDEvent));
  if (!ledEventQueue) {
    Serial.println("❌ Failed to create LED event queue!");
    return;
  }

  // Load color from preferences
  setMainColor();

  Serial.println("✅ LEDManager initialized");
}

void LEDManager::startAnimationTask() {
  if (animationTaskHandle) {
    Serial.println("💡 LED Animation task already running");
    return;
  }

  xTaskCreatePinnedToCore(
    animationTaskFunction,
    "LED_Animation",
    8192,
    this,
    2,  // Priority 2 (lower than UI task)
    &animationTaskHandle,
    0);

  if (animationTaskHandle) {
    Serial.println("✅ LED Animation task started on Core 0");
  } else {
    Serial.println("❌ Failed to start LED Animation task!");
  }
}

void LEDManager::stopAnimationTask() {
  if (animationTaskHandle) {
    vTaskDelete(animationTaskHandle);
    animationTaskHandle = nullptr;
    Serial.println("💡 LED Animation task stopped");
  }
}

// 🆕 Static task function
void LEDManager::animationTaskFunction(void* parameter) {
  LEDManager* ledManager = static_cast<LEDManager*>(parameter);
  ledManager->animationLoop();
}

// 🆕 Main animation loop (runs on dedicated task)
void LEDManager::animationLoop() {
  TickType_t lastWake = xTaskGetTickCount();
  uint32_t loopCounter = 0;

  Serial.println("💡 LED Animation loop started");

  while (true) {
    loopCounter++;

    if (loopCounter % 1000 == 0) {
      Serial.printf("💡 LED Task alive - loops: %d, free stack: %d\n",
                    loopCounter, uxTaskGetStackHighWaterMark(NULL));
    }

    // Process incoming events
    processLEDEvents();

    // Update animations
    updateAnimations();

    // 🎯 Precise timing: 100Hz (10ms) for smooth animations
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(10));
  }
}

// 🆕 Process events from other tasks
void LEDManager::processLEDEvents() {
  LEDEvent event;

  // Process all pending events (non-blocking)
  while (xQueueReceive(ledEventQueue, &event, 0) == pdPASS) {
    switch (event.type) {
      case LEDEventType::SET_LOADING:
        loading = event.state;
        if (loading) {
          lastEncMovementTime = millis();
          pulsating = false;
          fading = false;
          setAllLEDs(244, 70, 17);  // Orange loading color
        } else {
          setAllLEDs(baseRed, baseGreen, baseBlue);
          fadeToBrightness(200, 150);
        }
        break;

      case LEDEventType::FLASH_COLOR:
        pulsating = false;
        fading = false;
        flash = true;
        flashDuration = event.duration;
        lastFlashStarted = millis();
        currentBrightness = event.brightness;

        strip.setBrightness(event.brightness);
        setAllLEDs(event.r, event.g, event.b);
        strip.show();
        break;

      case LEDEventType::ACTIVATE:
        if (!loading) {
          lastEncMovementTime = millis();
        }
        pulsating = false;
        fading = false;
        currentBrightness = 255;

        strip.setBrightness(255);
        setAllLEDs(red, green, blue);
        strip.show();
        break;

      case LEDEventType::SET_COLOR:
        {
          // event.duration contains color name hash or use r,g,b directly
          setAllLEDs(event.r, event.g, event.b);
          baseRed = event.r;
          baseGreen = event.g;
          baseBlue = event.b;
          red = event.r;
          green = event.g;
          blue = event.b;
          strip.show();
          break;
        }

      default:
        break;
    }
  }
}

// 🆕 Main animation update function
void LEDManager::updateAnimations() {
  unsigned long currentMillis = millis();

  // Priority order: Fade → Flash → Init → Loading → Pulsating → Idle

  if (fading) {
    updateFadeAnimation();
    return;
  }

  if (flash) {
    updateFlashAnimation();
    return;
  }

  if (!initAnimationCompleted) {
    updateInitAnimation();
    return;
  }

  if (loading) {
    updateLoadingAnimation();
    return;
  }

  if (pulsating) {
    updatePulsatingAnimation();
    return;
  }

  // Auto-return to pulsating after idle time
  if (currentMillis - lastEncMovementTime >= IDLE_DELAY_SECONDS * 1000) {
    if (currentBrightness > 10) {
      fadeToBrightness(10, 1000);
    } else {
      pulsating = true;
    }
  }
}

void LEDManager::updateInitAnimation() {
  unsigned long currentMillis = millis();

  if (!inited && currentMillis - previousMillis >= ANIMATION_INTERVAL) {
    previousMillis = currentMillis;

    resetAllLEDs();
    strip.setPixelColor(cursorLED, strip.Color(red, green, blue));
    strip.show();

    if (clockwise) {
      cursorLED++;
    } else {
      cursorLED--;
    }

    if (cursorLED >= NUM_LEDS) {
      clockwise = false;
      cursorLED = NUM_LEDS - 1;
    }

    if (cursorLED <= 0 && !clockwise) {
      // Init animation complete
      strip.setBrightness(0);
      strip.show();
      vTaskDelay(pdMS_TO_TICKS(500));

      fadeToBrightness(255, 150);
      inited = true;
      cursorLED = 0;
      clockwise = true;
    }
  }

  if (inited && currentBrightness != 0) {
    fadeToBrightness(0, 3000);
  }

  if (currentMillis > 4000 && !initAnimationCompleted) {
    initAnimationCompleted = true;
    pulsating = true;

    Serial.println("💡 LED Init animation completed!");
    // Potresti anche inviare un evento qui se vuoi
  }
}

void LEDManager::updateLoadingAnimation() {
  static unsigned long lastLoadingUpdate = 0;
  static bool loadingUp = true;

  if (getBrightness() != 250) {
    fadeToBrightness(250, 50);
  }

  if (getBrightness() != 1) {
    fadeToBrightness(1, 50);
  }
}

void LEDManager::updatePulsatingAnimation() {
  static unsigned long lastPulseUpdate = 0;
  static bool pulseUp = true;

  if (millis() - lastPulseUpdate > 100) {  // 10Hz pulsating
    lastPulseUpdate = millis();

    if (pulseUp) {
      if (currentBrightness < 100) {
        fadeToBrightness(100, 1500);
      } else {
        pulseUp = false;
      }
    } else {
      if (currentBrightness > 10) {
        fadeToBrightness(10, 2000);
      } else {
        pulseUp = true;
      }
    }
  }
}

void LEDManager::updateFlashAnimation() {
  if (millis() - lastFlashStarted >= flashDuration) {
    flash = false;
    flashDuration = 0;
    setAllLEDs(baseRed, baseGreen, baseBlue);
    strip.show();
  }
}

void LEDManager::updateFadeAnimation() {
  unsigned long elapsedTime = millis() - fadeStartMillis;

  if (elapsedTime <= fadeDuration) {
    int brightness = map(elapsedTime, 0, fadeDuration, startBrightness, targetBrightness);
    strip.setBrightness(brightness);
    setAllLEDs(red, green, blue);
    strip.show();
  } else {
    // Fade completed
    fading = false;
    currentBrightness = targetBrightness;
    strip.setBrightness(currentBrightness);
    setAllLEDs(red, green, blue);
    strip.show();
  }
}

// 🆕 Thread-safe public interface methods
void LEDManager::setLoadingState(bool on) {
  LEDEvent event;
  event.type = LEDEventType::SET_LOADING;
  event.state = on;
  sendLEDEvent(event);
}

void LEDManager::flashColor(uint8_t r, uint8_t g, uint8_t b, int brightness, int duration) {
  LEDEvent event;
  event.type = LEDEventType::FLASH_COLOR;
  event.r = r;
  event.g = g;
  event.b = b;
  event.brightness = brightness;
  event.duration = duration;
  sendLEDEvent(event);
}

void LEDManager::activate() {
  LEDEvent event;
  event.type = LEDEventType::ACTIVATE;
  sendLEDEvent(event);
}

void LEDManager::updateLastEncoderMovementTime() {
  activate();  // Same effect
}

void LEDManager::setMainColor(const String& colorName) {
  PreferencesManager prefs;
  String actualColorName = colorName.isEmpty() ? prefs.get("conf.ledColor") : colorName;

  // 🆕 Simplified color mapping (more ESP32-friendly)
  uint8_t r = 0, g = 181, b = 214;  // Default azure

  if (actualColorName == "azure") {
    r = 0;
    g = 181;
    b = 214;
  } else if (actualColorName == "green") {
    r = 30;
    g = 254;
    b = 30;
  } else if (actualColorName == "red") {
    r = 255;
    g = 0;
    b = 0;
  } else if (actualColorName == "orange") {
    r = 254;
    g = 94;
    b = 0;
  } else if (actualColorName == "yellow") {
    r = 128;
    g = 128;
    b = 0;
  } else if (actualColorName == "blue") {
    r = 18;
    g = 0;
    b = 213;
  } else if (actualColorName == "violet") {
    r = 110;
    g = 0;
    b = 255;
  } else if (actualColorName == "purple") {
    r = 211;
    g = 0;
    b = 164;
  } else {
    // Default to azure if unknown
    Serial.printf("⚠️ Unknown LED color: %s, using azure\n", actualColorName.c_str());
  }

  LEDEvent event;
  event.type = LEDEventType::SET_COLOR;
  event.r = r;
  event.g = g;
  event.b = b;
  sendLEDEvent(event);

  Serial.printf("💡 LED Color set to: %s (%d,%d,%d)\n", actualColorName.c_str(), r, g, b);
}


// 🆕 Helper methods
bool LEDManager::sendLEDEvent(const LEDEvent& event) {
  if (!ledEventQueue) return false;

  if (xQueueSend(ledEventQueue, &event, pdMS_TO_TICKS(10)) != pdPASS) {
    Serial.println("⚠️ LED event queue full!");
    return false;
  }
  return true;
}

void LEDManager::resetAllLEDs() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
}

void LEDManager::setAllLEDs(uint8_t r, uint8_t g, uint8_t b) {
  red = r;
  green = g;
  blue = b;

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
}

void LEDManager::fadeToBrightness(int brightness, int duration) {
  startBrightness = currentBrightness;
  targetBrightness = brightness;
  if (!fading) {
    fadeStartMillis = millis();
  }
  fadeDuration = duration;
  fading = true;
}

void LEDManager::restartAnimationTask() {
  if (animationTaskHandle) {
    vTaskDelete(animationTaskHandle);
    animationTaskHandle = nullptr;
    Serial.println("💡 LED Animation task deleted");
  }

  delay(100);  // Breve pausa per cleanup
  startAnimationTask();
}

}  // namespace CloudMouse