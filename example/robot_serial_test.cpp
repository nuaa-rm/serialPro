//
// Created by bismarck on 12/17/22.
//

#include <iostream>
#include <iomanip>

#include "serialPro/robotComm.h"

message_data dd {
    uint8_t a;
    uint16_t b;
};

void callback(const dd& d) {
    std::cout << "recv: \n";
    std::cout << "a: " << std::hex << std::setw(2) << std::setfill('0') << (int)d.a << std::endl;
    std::cout << "b: " << std::hex << std::setw(4) << std::setfill('0') << d.b << std::endl;
}

int main() {
    robot::RobotSerial serial("/dev/ttyUSB0", 115200);
    serial.registerCallback(0x10, &callback);
    serial.registerCallback(0x09, [](const dd& d) {
        std::cout << "recv: \n";
        std::cout << "a: " << std::hex << std::setw(2) << std::setfill('0') << (int)d.a << std::endl;
        std::cout << "b: " << std::hex << std::setw(4) << std::setfill('0') << d.b << std::endl;
    });

    serial.write(0x10, dd{0x01, 0x0203});

    sleep(1);
}
