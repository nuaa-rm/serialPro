//
// Created by bismarck on 12/17/22.
//

#include <iostream>
#include <iomanip>
#include <ctime>

#include "serialPro/robotComm.h"

clock_t begin1, begin2;

message_data dd {
    uint8_t a;
    uint16_t b;
};

void callback(const dd &d) {
    clock_t end = clock();
    std::cout << "Send time: " << (double) (begin2 - begin1) / CLOCKS_PER_SEC * 1000 << "ms" << std::endl;
    std::cout << "Recv time: " << (double) (end - begin2) / CLOCKS_PER_SEC * 1000 << "ms" << std::endl;
    std::cout << "recv: \n";
    std::cout << "a: " << std::hex << std::setw(2) << std::setfill('0') << (int) d.a << std::endl;
    std::cout << "b: " << std::hex << std::setw(4) << std::setfill('0') << d.b << std::endl;
}

int main() {
    robot::RobotSerial serial("/dev/ttyUSB0", 115200);
    serial.registerCallback(0x10, &callback);
    serial.registerCallback(0x09, [](const dd &d) {
        std::cout << "recv: \n";
        std::cout << "a: " << std::hex << std::setw(2) << std::setfill('0') << (int) d.a << std::endl;
        std::cout << "b: " << std::hex << std::setw(4) << std::setfill('0') << d.b << std::endl;
    });

    begin1 = clock();
    serial.write(0x10, dd{0x01, 0x0203});
    begin2 = clock();
    serial.spinOnce();
}
