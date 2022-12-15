//
// Created by bismarck on 12/8/22.
//

#ifndef MSG_SERIALIZE_LISTENER_H
#define MSG_SERIALIZE_LISTENER_H

#include <vector>
#include <string>
#include <cstring>

#include "callback_manager.h"
#include "check.h"

namespace ms {
    template<typename Head, typename Tail>
    class Listener {
    private:
        CallbackManager callbackManager;
        std::string txBuffer{100};
        std::vector<std::function<bool(const Head&)>> headCheckers;
        std::vector<std::function<bool(const Tail&, const uint8_t*, int)>> tailCheckers;
        std::function<size_t(const Head&)> getLength;
        std::function<int(const Head&)> getId;
        int maxSize = 1024;

    public:
        Listener() = default;

        explicit Listener(std::function<size_t(const Head&)> _getLength, std::function<int(const Head&)> _getId) :
                getLength(_getLength), getId(_getId) {}

        Listener(Listener&& other)  noexcept {
            callbackManager = other.callbackManager;
            txBuffer.swap(other.txBuffer);
            headCheckers.swap(other.headCheckers);
            tailCheckers.swap(other.tailCheckers);
            getLength = other.getLength;
        }

        Listener& operator=(const Listener& other) {
            callbackManager = other.callbackManager;
            txBuffer = other.txBuffer;
            headCheckers = other.headCheckers;
            tailCheckers = other.tailCheckers;
            getLength = other.getLength;
            return *this;
        }

        void setMaxSize(int _maxSize) {
            maxSize = _maxSize;
        }

        void registerChecker(std::function<bool(const Head&)> checker) {
            headCheckers.push_back(checker);
        }

        void registerChecker(std::function<bool(const Tail&, const uint8_t*, int)> checker) {
            tailCheckers.push_back(checker);
        }

        void setGetLength(std::function<size_t(const Head&)> _getLength) {
            getLength = _getLength;
        }

        void setGetId(std::function<int(const Head&)> _getId) {
            getId = _getId;
        }

        template<typename T>
        bool registerCallback(int id, std::function<void(const T&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback](const uint8_t* data) {
                T t;
                memcpy(&t, data, sizeof(T));
                userCallback(t);
            });
        }

        bool append(const uint8_t c) {
            return append(*(char*)&c);
        }

        bool append(const char c) {
            if (txBuffer.size() >= maxSize) {
                return false;
            }
            txBuffer.push_back(c);
            int eraseSize = 0;
            if ((int)txBuffer.size() < sizeof(Head)+sizeof(Tail)) {
                return false;
            }
            bool okHeadFound = false;
            for (int i = 0; (long)i < (long)txBuffer.size()-sizeof(Head)-sizeof(Tail); i++) {
                uint8_t *p = (uint8_t*)txBuffer.data() + i;
                int size = (int)txBuffer.size() - i;

                Head head;
                memcpy(&head, p, sizeof(Head));
                bool headOk = true;
                for (auto checker : headCheckers) {
                    if (!checker(head)) {
                        headOk = false;
                        break;
                    }
                }
                if (!headOk && !okHeadFound) {
                    eraseSize = i+1;
                    continue;
                }

                int length = getLength(head);
                if (size < length + sizeof(Tail) + sizeof(Head)) {
                    okHeadFound = true;
                    continue;
                }

                Tail tail;
                memcpy(&tail, p + sizeof(Head) + length, sizeof(Tail));
                bool tailOk = true;
                for (auto checker : tailCheckers) {
                    if (!checker(tail, p, sizeof(Head) + length)) {
                        tailOk = false;
                        break;
                    }
                }
                if (!tailOk && !okHeadFound) {
                    eraseSize = i+1;
                    continue;
                }
                int id = getId(head);
                callbackManager[id](p + sizeof(Head));
                txBuffer.erase(0, i+size);
                return true;
            }
            txBuffer.erase(0, eraseSize);
            return false;
        }
    };
}

#endif //MSG_SERIALIZE_LISTENER_H
