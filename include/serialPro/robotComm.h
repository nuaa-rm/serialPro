//
// Created by bismarck on 12/17/22.
//

#ifndef SERIALPRO_ROBOTCOMM_H
#define SERIALPRO_ROBOTCOMM_H

#include "serialPro.h"

namespace robot {
    message_data head {
        uint8_t header = 0xaa;
        uint8_t length = 0;
        uint8_t id = 0;
    };

    message_data tail {
        uint8_t crc8 = 0;
    };

    class RobotSerial : public sp::serialPro<head, tail> {
    public:
        enum error {
            lengthNotMatch = -2,        // 从下位机接受的消息长度与注册回调时传入的消息长度不匹配
            rxLessThanLength = -1,      // 当前缓冲区中的消息不完整，下次解析时重试
            ok = 0,
            sofError,                   // 帧头不匹配
            crcError                    // crc8校验结果错误
        };

        // 构造函数
        RobotSerial() = default;
        RobotSerial(const std::string& port, int baud) : sp::serialPro<head, tail>(port, baud) {
            registerSetter([](head& h, int s) {
                h.length = s + sizeof(head) + sizeof(tail);
            });
            registerSetter([](tail& t, const uint8_t* data, int s) {
                t.crc8 = ms::crc8check(data, s);
            });

            registerChecker([](const head& h) -> int {
                if (h.header == 0xaa) {
                    return ok;
                } else {
                    return sofError;
                }
            });
            registerChecker([](const tail& t, const uint8_t* data, int s) -> int {
                if (t.crc8 == ms::crc8check(data, s)) {
                    return ok;
                } else {
                    return crcError;
                }
            });
            setGetId([](const head& h) {
                return h.id;
            });
            setGetLength([](const head& h) {
                return h.length - sizeof(head) - sizeof(tail);
            });

            setListenerMaxSize(256);
        }

        RobotSerial(const RobotSerial& other) = delete;
        RobotSerial(RobotSerial&& other) noexcept : sp::serialPro<head, tail>(std::move(other)) {}

        using sp::serialPro<head, tail>::operator=;

        // 发送数据
        template<typename T>
        bool write(uint8_t id, const T& t) {
            return sp::serialPro<head, tail>::write(head{.id=id}, t);
        }
    };
}

#endif //SERIALPRO_ROBOTCOMM_H
