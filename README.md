# IoT-Cat-Detection-ESP32-ESp8266
This repository contains the Arduino code for an ESP32-CAM and ESP8266-based automated pet feeder system. The ESP32-CAM is used to detect cats using an AI model, while the ESP8266 manages the feeding mechanism, including monitoring the feeding container's status and controlling the feeder's motor. Both modules communicate with a Telegram bot to receive commands and send updates.

## Features
- **AI Cat Detection**: The ESP32-CAM uses an AI model trained with a personal cat image dataset, developed with FOMO (Faster Objects, More Objects) from EdgeImpulse, to detect the presence of a cat.
- **Telegram Integration**: Both ESP32-CAM and ESP8266 can receive commands from and send updates to a Telegram bot.
- **ESP32-CAM**: Captures and sends a photo along with the AI classification result.
- **ESP8266**: Manages the feeding system, returns feeding timer, updates status timer, checks module status, and monitors feeding container fullness.
- **The ESP8266 controls**:
- **Ultrasonic Sensor (HC-SR04)**: Monitors the fullness of the feeding container.
- **Stepper Motor (28BYJ-48 5V)**: Rotates the feeder compartment to dispense food.

## Components
### Hardware
ESP32-CAM: Used for AI-based cat detection and capturing images.
ESP8266: Manages the feeding mechanism and communicates with the Telegram bot.
Ultrasonic Sensor (HC-SR04): Checks the feeding container's fullness.
Stepper Motor (28BYJ-48 5V): Controls the feeder compartment to dispense food.
Telegram Bot: Interfaces with both ESP32-CAM and ESP8266 for remote control and monitoring.
### Software
Arduino IDE: Used for programming the ESP32-CAM and ESP8266.
EdgeImpulse FOMO Model: Trained on a personal cat image dataset to detect cats.
Telegram Bot API: Handles commands and responses between the ESP modules and the Telegram bot.

# Setup Instructions
## ESP32-CAM Setup:
1. Clone this repository and open the ESP32-CAM folder in the Arduino IDE.
2. Configure your Wi-Fi credentials and Telegram bot token in the code.
3. Upload the AI model trained with FOMO from EdgeImpulse to the ESP32-CAM.
4. Upload the code to the ESP32-CAM.

## ESP8266 Setup:
1. Open the ESP8266 folder in the Arduino IDE.
2. Configure your Wi-Fi credentials and Telegram bot token in the code.
3. Set up the Ultrasonic Sensor (HC-SR04) and Stepper Motor (28BYJ-48 5V) connections.
4. Upload the code to the ESP8266.

## Telegram Bot Configuration:
1. Create a Telegram bot using BotFather and obtain the bot token.
2. Integrate the bot token into both the ESP32-CAM and ESP8266 code.

## Power On and Test:
1. Power on the ESP32-CAM and ESP8266 modules.
2. Test the system by sending commands to the Telegram bot and verify responses.

# Usage
## Commands for ESP32-CAM:
/capture: Captures a photo and returns the AI classification result.

## Commands for ESP8266:
1. /feeder_status: Returns the feeding timer, update status timer, and module status.
2. /check_container: Checks and returns the feeding container's fullness status.
3. /dispense_food: Activates the stepper motor to rotate the feeder compartment.

# Future Enhancements
- [ ] Add more AI models to detect multiple pets.
- [ ] Implement a more advanced feeding schedule.

# License
This project is licensed under the APACHE 2.0 License - see the LICENSE file for details.

# Acknowledgments
EdgeImpulse for providing the FOMO model framework.
Telegram for the bot API.
Arduino for the development environment.
