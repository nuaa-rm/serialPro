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
        std::string rxBuffer;
        std::vector<std::function<int(const Head&)>> headCheckers;
        std::vector<std::function<int(const Tail&, const uint8_t*, int)>> tailCheckers;
        std::function<size_t(const Head&)> getLength;
        std::function<int(const Head&)> getId;
        std::function<void(int, const std::string&)> errorHandle;
        int maxSize = 1024;

        template<typename T>
        struct fp_type;

        template<typename R, typename... Args>
        struct fp_type<R(*)(Args...)> {
            using ArgTypes = std::tuple<Args...>;
            static constexpr std::size_t ArgCount = sizeof...(Args);
            template<std::size_t N>
            using NthArg = std::tuple_element_t<N, ArgTypes>;
            using _data_type = typename std::remove_reference<NthArg<0>>::type;
            using data_type = typename std::remove_const<_data_type>::type;
        };

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
            bool frameFound = false;
            for (int i = 0; (long)i < (long)rxBuffer.size() - sizeof(Head) - sizeof(Tail); i++) {
                uint8_t *p = (uint8_t*)rxBuffer.data() + i;
                int size = (int)rxBuffer.size() - i;

                Head head;
                memcpy(&head, p, sizeof(Head));
                int headCheckerResult;
                for (auto checker : headCheckers) {
                    headCheckerResult = checker(head);
                    if (headCheckerResult) {
                        if (errorHandle) {
                            errorHandle(headCheckerResult, rxBuffer.substr(i));
                        }
                        break;
                    }
                }
                if (headCheckerResult) {
                    if (!okHeadFound) {
                        eraseSize = i+1;
                    }
                    continue;
                }

                int length = getLength(head);
                if (size < length + sizeof(Tail) + sizeof(Head)) {
                    okHeadFound = true;
                    if (errorHandle) {
                        errorHandle(-1, rxBuffer.substr(i));
                    }
                    continue;
                }

                Tail tail;
                memcpy(&tail, p + sizeof(Head) + length, sizeof(Tail));
                int tailCheckerResult;
                for (auto checker : tailCheckers) {
                    tailCheckerResult = checker(tail, p, sizeof(Head) + length);
                    if (tailCheckerResult) {
                        if (errorHandle) {
                            errorHandle(tailCheckerResult, rxBuffer.substr(i));
                        }
                        break;
                    }
                }
                if (tailCheckerResult) {
                    if (!okHeadFound) {
                        eraseSize = i+1;
                    }
                    continue;
                }
                int id = getId(head);
                int cRes = callbackManager[id](p);
                if (cRes && errorHandle) {
                    errorHandle(cRes-1, rxBuffer.substr(i));
                }
                eraseSize = i + size;
                frameFound = true;
            }
            rxBuffer.erase(0, eraseSize);
            return frameFound;
        }

        template<typename T>
        bool _registerCallback(int id, std::function<void(const T&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback, this](const uint8_t* data) {
                Head head;
                memcpy(&head, data, sizeof(Head));
                if (this->getLength(head) != sizeof(T)) {
                    return -1;
                }
                T t;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                userCallback(t);
                return 0;
            });
        }

        bool _registerCallback(int id, std::function<void(const std::string&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback, this](const uint8_t* data) {
                Head head;
                memcpy(&head, data, sizeof(Head));
                std::string t((const char*)data + sizeof(Head), this->getLength(head));
                userCallback(t);
                return 0;
            });
        }

        template<typename T>
        bool _registerCallback(int id, std::function<void(const T&, const Head&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback, this](const uint8_t* data) {
                Head head;
                memcpy(&head, data, sizeof(Head));
                if (this->getLength(head) != sizeof(T)) {
                    return -1;
                }
                T t;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                userCallback(t, head);
                return 0;
            });
        }

        bool _registerCallback(int id, std::function<void(const std::string&, const Head&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback, this](const uint8_t* data) {
                Head head;
                memcpy(&head, data, sizeof(Head));
                std::string t((const char*)data + sizeof(Head), this->getLength(head));
                userCallback(t, head);
                return 0;
            });
        }

        template<typename T>
        bool _registerCallback(int id, std::function<void(const T&, const Head&, const Tail&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback, this](const uint8_t* data) {
                Head head;
                memcpy(&head, data, sizeof(Head));
                if (this->getLength(head) != sizeof(T)) {
                    return -1;
                }
                T t;
                Tail tail;
                memcpy(&t, data + sizeof(Head), sizeof(T));
                memcpy(&tail, data + sizeof(Head) + sizeof(T), sizeof(Tail));
                userCallback(t, head, tail);
                return 0;
            });
        }

        bool _registerCallback(int id, std::function<void(const std::string&, const Head&, const Tail&)> userCallback) {
            return callbackManager.registerCallback(id, [userCallback, this](const uint8_t* data) {
                Head head;
                Tail tail;
                memcpy(&head, data, sizeof(Head));
                std::string t((const char*)data + sizeof(Head), this->getLength(head));
                memcpy(&tail, data + sizeof(Head) + t.length(), sizeof(Tail));
                userCallback(t, head, tail);
                return 0;
            });
        }

    public:
        Listener() = default;

        explicit Listener(std::function<size_t(const Head&)> _getLength, std::function<int(const Head&)> _getId) :
                getLength(_getLength), getId(_getId) {}

        Listener(Listener&& other) noexcept {
            callbackManager = other.callbackManager;
            rxBuffer.swap(other.rxBuffer);
            headCheckers.swap(other.headCheckers);
            tailCheckers.swap(other.tailCheckers);
            getLength = other.getLength;
            getId = other.getId;
        }

        Listener& operator=(const Listener& other) {
            callbackManager = other.callbackManager;
            rxBuffer = other.rxBuffer;
            headCheckers = other.headCheckers;
            tailCheckers = other.tailCheckers;
            getLength = other.getLength;
            getId = other.getId;
            return *this;
        }

        void setMaxSize(int _maxSize) {
            maxSize = _maxSize;
        }

        void registerChecker(std::function<int(const Head&)> checker) {
            headCheckers.push_back(checker);
        }

        void registerChecker(std::function<int(const Tail&, const uint8_t*, int)> checker) {
            tailCheckers.push_back(checker);
        }

        void setGetLength(std::function<size_t(const Head&)> _getLength) {
            getLength = _getLength;
        }

        void setGetId(std::function<int(const Head&)> _getId) {
            getId = _getId;
        }

        bool push(const uint8_t c) {
            return push(*(char *) &c);
        }

        bool push(const char c) {
            if (rxBuffer.size() >= maxSize) {
                return false;
            }
            rxBuffer.push_back(c);
            return scan();
        }

        bool push(const char* buffer, int len) {
            rxBuffer.append(buffer, len);
            return scan();
        }

        bool push(const std::string& str) {
            rxBuffer.append(str);
            return scan();
        }

        void registerErrorHandle(std::function<void(int, const std::string&)> func) {
            errorHandle = func;
        }

        template<class T, class func_t = typename lambda_type<decltype(&T::operator())>::type>
        void registerCallback(int id, T callback) {
            _registerCallback(id, func_t(std::forward<T>(callback)));
        }

        template<typename ... args>
        void registerCallback(int id, std::function<void(args...)> callback) {
            _registerCallback(id, callback);
        }

        template<typename T>
        typename std::enable_if<std::is_pointer<T>::value, void>::type
        registerCallback(int id, T callback) {
            using data_type = typename fp_type<T>::data_type;
            _registerCallback<data_type>(id, callback);
        }
    };
}

#endif //MSG_SERIALIZE_LISTENER_H
