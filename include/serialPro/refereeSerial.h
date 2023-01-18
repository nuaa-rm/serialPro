//
// Created by bismarck on 23-1-18.
//

#ifndef SERIALPRO_REFEREE_SERIAL_H
#define SERIALPRO_REFEREE_SERIAL_H

#include "serialPro.h"

namespace referee {
    message_data head {
        uint8_t SOF = 0xA5;
        uint16_t length = 0;
        uint8_t seq = 0;
        uint8_t crc8 = 0;
        uint16_t cmd_id = 0;
    };

    message_data tail {
        uint16_t crc16 = 0;
    };

    enum class game_type_t: uint8_t {
        RMUC = 1,
        ICRA = 3,
        RMUS_3V3,
        RMUS_1V1
    };

    enum class game_progress_t: uint8_t {
        NOT_START,
        PREPARE,
        CHECK,
        COUNTDOWN,
        FIGHTING,
        SUMMARY
    };

    message_data game_status_ts {
        uint8_t game_type : 4;
        uint8_t game_progress : 4;
        uint16_t stage_remain_time;
        uint64_t sync_time_stamp;
    };

    message_data game_status_t {
        game_type_t game_type;
        game_progress_t game_progress;
        uint16_t stage_remain_time;
        uint64_t sync_time_stamp;
    };

    game_status_t gameStatusFromSerial(game_status_ts in) {
        return {
            static_cast<game_type_t>(in.game_type),
            static_cast<game_progress_t>(in.game_progress),
            in.stage_remain_time,
            in.sync_time_stamp
        };
    }

    message_data game_result_t {
        uint8_t winner;
    };

    message_data game_robot_HP_t {
        uint16_t red_1_robot_HP;
        uint16_t red_2_robot_HP;
        uint16_t red_3_robot_HP;
        uint16_t red_4_robot_HP;
        uint16_t red_5_robot_HP;
        uint16_t red_7_robot_HP;
        uint16_t red_outpost_HP;
        uint16_t red_base_HP;
        uint16_t blue_1_robot_HP;
        uint16_t blue_2_robot_HP;
        uint16_t blue_3_robot_HP;
        uint16_t blue_4_robot_HP;
        uint16_t blue_5_robot_HP;
        uint16_t blue_7_robot_HP;
        uint16_t blue_outpost_HP;
        uint16_t blue_base_HP;
    };

    enum class dart_belong_t: uint8_t {
        RED = 1,
        BLUE
    };

    message_data dart_status_t {
        dart_belong_t dart_belong;
        uint16_t stage_remaining_time;
    };

//    typedef uint32_t event_data_ts;
//    struct event_data_t {
//        bool HP_zone_1, HP_zone_2, HP_zone_3;
//    };

    class RefereeSerial : protected sp::serialPro<head, tail> {
    public:
        // 构造函数
        RefereeSerial() = default;
        RefereeSerial(const std::string& port, int baud) : sp::serialPro<head, tail>(port, baud) {
            registerSetter([](head& h, int s) {
                h.length = s;
            });
            registerSetter([](head& h, int _) {
                h.crc8 = ms::crc8check((uint8_t*)&h, sizeof(head)-2);
            });
            registerSetter([](tail& t, const uint8_t* data, int s) {
                t.crc16 = ms::crc16check(data, s);
            });

            registerChecker([](const head& h) {
                return h.SOF == 0xA5;
            });
            registerChecker([](const head& h) {
                return h.crc8 == ms::crc8check((uint8_t*)&h, sizeof(head)-2);
            });
            registerChecker([](const tail& t, const uint8_t* data, int s) {
                return t.crc16 == ms::crc16check(data, s);
            });

            setGetId([](const head& h) {
                return h.cmd_id;
            });
            setGetLength([](const head& h) {
                return h.length;
            });

            setListenerMaxSize(512);
        }

        RefereeSerial(const RefereeSerial& other) = delete;
        RefereeSerial& operator=(const RefereeSerial& other) = delete;
        RefereeSerial(RefereeSerial&& other) noexcept : sp::serialPro<head, tail>(std::move(other)) {}

        // 注册回调函数
        using sp::serialPro<head, tail>::registerCallback;

        // 发送数据
        template<typename T>
        bool write(uint8_t id, uint8_t seq, const T& t) {
            return sp::serialPro<head, tail>::write(head{.seq=seq, .cmd_id=id}, t);
        }
    };
}

#endif //SERIALPRO_REFEREE_SERIAL_H
