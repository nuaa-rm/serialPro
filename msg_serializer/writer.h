//
// Created by bismarck on 12/8/22.
//

#ifndef MSG_SERIALIZE_WRITER_H
#define MSG_SERIALIZE_WRITER_H

#include <vector>
#include <string>
#include <cstring>
#include <functional>

#include "check.h"

namespace ms {
    template<typename Head, typename Tail>
    class Writer {
    private:
        std::vector<std::function<void(Head&, int)>> headSetters;
        std::vector<std::function<void(Tail&, const uint8_t*, int)>> tailSetters;
    public:
        Writer() = default;

        Writer(Writer&& other) noexcept {
            headSetters.swap(other.headSetters);
            tailSetters.swap(other.tailSetters);
        }

        Writer& operator=(const Writer& other) {
            headSetters = other.headSetters;
            tailSetters = other.tailSetters;
            return *this;
        }

        void registerSetter(std::function<void(Head&, int)> setter) {
            headSetters.push_back(setter);
        }

        void registerSetter(std::function<void(Tail&, const uint8_t*, int)> setter) {
            tailSetters.push_back(setter);
        }

        template<typename T>
        std::string serialize(Head head, T t, Tail tail=Tail{}) {
            std::string s;
            s.resize(sizeof(Head) + sizeof(T) + sizeof(Tail));

            for (auto& setter : headSetters) {
                setter(head, sizeof(T));
            }
            memcpy((void*)s.data(), &head, sizeof(Head));

            memcpy((void*)(s.data()+sizeof(Head)), &t, sizeof(T));

            for (auto& setter : tailSetters) {
                setter(tail, (uint8_t*)s.data(), sizeof(Head)+sizeof(T));
            }
            memcpy((void*)(s.data()+sizeof(Head)+sizeof(T)), &tail, sizeof(Tail));

            return std::move(s);
        }

        std::string serialize(Head head, std::string& t, Tail tail=Tail{}) {
            std::string s;
            s.resize(sizeof(Head) + t.length() + sizeof(Tail));

            for (auto& setter : headSetters) {
                setter(head, t.length());
            }
            memcpy((void*)s.data(), &head, sizeof(Head));

            memcpy((void*)(s.data()+sizeof(Head)), t.c_str(), t.length());

            for (auto& setter : tailSetters) {
                setter(tail, (uint8_t*)s.data(), sizeof(Head)+t.length());
            }
            memcpy((void*)(s.data()+sizeof(Head)+t.length()), &tail, sizeof(Tail));

            return std::move(s);
        }
    };
}

#endif //MSG_SERIALIZE_WRITER_H
