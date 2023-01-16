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
        std::string rxBuffer{100};
        std::vector<std::function<bool(const Head&)>> headCheckers;
        std::vector<std::function<bool(const Tail&, const uint8_t*, int)>> tailCheckers;
        std::function<size_t(const Head&)> getLength;
        std::function<int(const Head&)> getId;
        int maxSize = 1024;

        template<class T>
        struct lambda_type;

        template<class C, class Ret, class... Args>
        struct lambda_type<Ret(C::*)(Args...) const>
        {
            using type = std::function<void(Args...)>;
        };

        bool scan() {
            int eraseSize = 0;
            if ((int)rxBuffer.size() < sizeof(Head) + sizeof(Tail)) {
                return false;
            }
            bool okHeadFound = false;
            for (int i = 0; (long)i < (long)rxBuffer.size() - sizeof(Head) - sizeof(Tail); i++) {
                uint8_t *p = (uint8_t*)rxBuffer.data() + i;
                int size = (int)rxBuffer.size() - i;

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
                callbackManager[id](p);
                rxBuffer.erase(0, i + size);
                return true;
            }
            rxBuffer.erase(0, eraseSize);
            return false;
        }

        template<typename T>
        bool _registerCallback(int id, std::function<void(const T&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback](const uint8_t* data) {
                T t;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                userCallback(t);
            });
        }

        template<typename T>
        bool _registerCallback(int id, void(*userCallback)(const T&)) {
            return callbackManager.registerCallback(id, [userCallback](const uint8_t* data) {
                T t;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                userCallback(t);
            });
        }

        template<typename T>
        bool registerCallback(int id, void(*userCallback)(const Head&, const T&)) {
            return callbackManager.registerCallback(id, [userCallback](const uint8_t* data) {
                T t;
                Head head;
                Tail tail;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                memcpy(&t, data, sizeof(Head));
                memcpy(&t, data + sizeof(Head) + sizeof(T), sizeof(Tail));
                userCallback(head, t, tail);
            });
        }

        template<typename T>
        bool registerCallback(int id, std::function<void(const Head&, const T&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback](const uint8_t* data) {
                T t;
                Head head;
                Tail tail;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                memcpy(&t, data, sizeof(Head));
                memcpy(&t, data + sizeof(Head) + sizeof(T), sizeof(Tail));
                userCallback(head, t, tail);
            });
        }

        template<typename T>
        bool registerCallback(int id, void(*userCallback)(const Head&, const T&, const Tail&)) {
            return callbackManager.registerCallback(id, [userCallback](const uint8_t* data) {
                T t;
                Head head;
                Tail tail;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                memcpy(&t, data, sizeof(Head));
                memcpy(&t, data + sizeof(Head) + sizeof(T), sizeof(Tail));
                userCallback(head, t, tail);
            });
        }

        template<typename T>
        bool registerCallback(int id, std::function<void(const Head&, const T&, const Tail&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback](const uint8_t* data) {
                T t;
                Head head;
                Tail tail;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                memcpy(&t, data, sizeof(Head));
                memcpy(&t, data + sizeof(Head) + sizeof(T), sizeof(Tail));
                userCallback(head, t, tail);
            });
        }

    public:
        Listener() = default;

        explicit Listener(std::function<size_t(const Head&)> _getLength, std::function<int(const Head&)> _getId) :
                getLength(_getLength), getId(_getId) {}

        Listener(Listener&& other)  noexcept {
            callbackManager = other.callbackManager;
            rxBuffer.swap(other.rxBuffer);
            headCheckers.swap(other.headCheckers);
            tailCheckers.swap(other.tailCheckers);
            getLength = other.getLength;
        }

        Listener& operator=(const Listener& other) {
            callbackManager = other.callbackManager;
            rxBuffer = other.rxBuffer;
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

        bool append(const uint8_t c) {
            return append(*(char*)&c);
        }

        bool append(const char c) {
            if (rxBuffer.size() >= maxSize) {
                return false;
            }
            rxBuffer.push_back(c);
            return scan();
        }

        template<class T, typename fun_t = typename lambda_type<decltype(&T::operator())>::type>
        void registerCallback(int id, T callback) {
            _registerCallback(id, fun_t(std::forward<T>(callback)));
        }

        template<typename ... args>
        void registerCallback(int id, std::function<void(args...)> callback) {
            _registerCallback(id, callback);
        }

        template<typename T>
        typename std::enable_if<std::is_pointer<T>::value, void>::type
        registerCallback(int id, T callback) {
            _registerCallback(id, callback);
        }
    };
}

#endif //MSG_SERIALIZE_LISTENER_H
