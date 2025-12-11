# ESP32 SIMCom L501 SDK Project

## Overview
This project provides an SDK for the SIMCom L501 module, designed to work with the ESP32 using the Arduino framework. The SDK facilitates UART communication with the SIMCom L501, allowing users to send AT commands and receive responses.

## Project Structure
```
esp32-simcom-sdk-project
├── sdk
│   ├── include
│   │   └── simcom_l501.h
│   └── src
│       └── simcom_l501.cpp
├── sample-project
│   ├── src
│   │   └── main.cpp
│   └── platformio.ini
├── README.md
```

## SDK Details
- **Header File (`sdk/include/simcom_l501.h`)**: Contains function prototypes for initializing the SIMCom L501 module, sending commands, and receiving responses.
- **Source File (`sdk/src/simcom_l501.cpp`)**: Implements the functions declared in the header file, handling UART communication with the SIMCom L501 module.

## Sample Project
- **Entry Point (`sample-project/src/main.cpp`)**: Initializes the ESP32, sets up UART communication, and demonstrates the usage of the SDK to interact with the SIMCom L501 module.
- **PlatformIO Configuration (`sample-project/platformio.ini`)**: Contains configuration settings for PlatformIO, specifying the board type, framework, and required libraries.

## Setup Instructions
1. **Clone the Repository**: Clone this project to your local machine.
2. **Install PlatformIO**: Ensure you have PlatformIO installed in your development environment.
3. **Open the Project**: Open the `esp32-simcom-sdk-project` folder in PlatformIO.
4. **Build the Project**: Use the PlatformIO build command to compile the SDK and sample project.
5. **Upload to ESP32**: Connect your ESP32 board and upload the sample project.

## Usage Example
After setting up the project, you can modify the `main.cpp` file in the `sample-project/src` directory to send specific AT commands to the SIMCom L501 module and handle responses as needed.

## License
This project is licensed under the MIT License. See the LICENSE file for more details.