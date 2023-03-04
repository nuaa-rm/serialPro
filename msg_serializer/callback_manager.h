//
// Created by bismarck on 12/8/22.
//

#ifndef MSG_SERIALIZE_CALLBACK_MANAGER_H
#define MSG_SERIALIZE_CALLBACK_MANAGER_H

#include <iostream>
#include <map>
#include <functional>


namespace ms {
    class CallbackManager{
    private:
        std::map<int, std::function<int(const uint8_t*)>> callbacks;
    public:
        bool registerCallback(int id, std::function<int(const uint8_t*)> callback) {
            auto it = callbacks.find(id);
            if (it != callbacks.end()) {
                return false;
            }
            callbacks[id] = std::move(callback);
            return true;
        }

        std::function<int(const uint8_t*)> operator[](int id) {
            auto it = callbacks.find(id);
            if (it != callbacks.end()) {
                return it->second;
            } else {
                std::cout << "Command " << id << " not Registered" << std::endl;
                return [](const uint8_t*){return 0;};
            }
        }
    };
}

#endif //MSG_SERIALIZE_CALLBACK_MANAGER_H
