//
// Created by bismarck on 12/17/22.
//

#ifndef SERIALPRO_ROBOTCOMM_H
#define SERIALPRO_ROBOTCOMM_H

#include "serialPro.h"

message_data head {
    uint8_t header = 0xaa;
    uint8_t length = 0;
    uint8_t id = 0;
};

message_data tail {
    uint8_t crc8 = 0;
};

class RobotComm : protected sp::serialPro<head, tail> {
public:
    // 构造函数
    RobotComm() = default;
    RobotComm(const std::string& port, int baud) : sp::serialPro<head, tail>(port, baud) {
        registerSetter([](head& h, int s) {
            h.length = s;
        });
        registerSetter([](tail& t, const uint8_t* data, int s) {
            t.crc8 = ms::crc8check(data, s);
        });

        registerChecker([](const head& h) {
            return h.header == 0xaa;
        });
        registerChecker([](const tail& t, const uint8_t* data, int s) {
            return t.crc8 == ms::crc8check(data, s);
        });
        setGetId([](const head& h) {
            return h.id;
        });
        setGetLength([](const head& h) {
            return h.length;
        });

        setListenerMaxSize(256);
    }

    RobotComm(const RobotComm& other) = delete;
    RobotComm& operator=(const RobotComm& other) = delete;
    RobotComm(RobotComm&& other) noexcept : sp::serialPro<head, tail>(std::move(other)) {}

    // 注册回调函数
    template<typename T>
    void registerCallback(int id, std::function<void(const T&)> callback) {
        sp::serialPro<head, tail>::registerCallback(id, callback);
    }

    // 发送数据
    template<typename T>
    bool write(uint8_t id, const T& t) {
        return sp::serialPro<head, tail>::write(head{.id=id}, t);
    }
};

#endif //SERIALPRO_ROBOTCOMM_H
