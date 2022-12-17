#include <iostream>
#include <iomanip>
#include "msg_serialize.h"

message_data head {
    uint8_t header = 0xaa;
    uint8_t length = 0;
    uint8_t id = 0;
};

message_data tail {
    uint8_t crc8 = 0;
    uint8_t tailer = 0xbb;
};

message_data data {
    uint8_t a;
    uint16_t b;
};

void cb(const data& d) {
    std::cout << "recv: \n";
    std::cout << "a: " << std::hex << std::setw(2) << std::setfill('0') << (int)d.a << std::endl;
    std::cout << "b: " << std::hex << std::setw(4) << std::setfill('0') << d.b << std::endl;
}

int main() {
    ms::Writer<head, tail> writer;
    writer.registerSetter([](head& h, int s) {
        h.length = s;
    });
    writer.registerSetter([](tail& t, const uint8_t* data, int s) {
        t.crc8 = ms::crc8check(data, s);
    });

    ms::Listener<head, tail> listener(
        [](const head& h) {return h.length;},
        [](const head& t) {return t.id;}
    );
    listener.registerCallback<data>(0x10, cb);
    listener.registerChecker([](const head& h) {
        return h.header == 0xaa;
    });
    listener.registerChecker([](const tail& t, const uint8_t* data, int s) {
        return t.crc8 == ms::crc8check(data, s);
    });
    listener.registerChecker([](const tail& t, const uint8_t* _d, int _s) {
        return t.tailer == 0xbb;
    });

    auto s = writer.serialize(head{.id=0x10}, data{0x01, 0x0203});

    for (auto c : s) {
        std::cout.fill('0');
        std::cout << std::setw(2) << std::hex << (int)*(uint8_t *)&c << " ";
    }
    std::cout << std::endl;

    listener.append((uint8_t)0);
    listener.append((uint8_t)0xbb);
    listener.append((uint8_t)0xaa);
    for (auto c : s) {
        listener.append(c);
    }
}
