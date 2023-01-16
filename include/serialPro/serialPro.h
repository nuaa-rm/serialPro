//
// Created by bismarck on 12/15/22.
//

#ifndef SERIALPRO_SERIALPRO_H
#define SERIALPRO_SERIALPRO_H

#include <atomic>
#include <string>
#include <thread>

#include "msg_serialize.h"
#include "serialib.h"

namespace sp {
    template<typename Head, typename Tail>
    class serialPro {
    private:
        ms::Listener<Head, Tail> listener;
        ms::Writer<Head, Tail> writer;
        serialib serial;
        std::thread readThread;
        std::atomic<bool> running{false};

        void readLoop() {
            while (running) {
                char c;
                serial.readChar(&c, 1);
                listener.append(c);
            }
        }

    public:
        serialPro() = default;

        serialPro(const std::string& port, int baud) {
            open(port, baud);
            running = true;
            readThread = std::thread(&serialPro::readLoop, this);
        }

        serialPro(serialPro&& other) noexcept {
            listener = std::move(other.listener);
            writer = std::move(other.writer);
            serial = std::move(other.serial);
            readThread = std::move(other.readThread);
        }

        serialPro& operator=(serialPro&& other) noexcept {
            if (this != &other) {
                listener = std::move(other.listener);
                writer = std::move(other.writer);
                serial = std::move(other.serial);
                readThread = std::move(other.readThread);
            }
            return *this;
        }

        serialPro(serialPro& _) = delete;
        serialPro& operator=(serialPro& _) = delete;

        ~serialPro() {
            close();
            running = false;
            if (readThread.joinable()) {
                readThread.join();
            }
        }

        char open(const std::string& port, int baud) {
            return serial.openDevice(port.c_str(), baud);
        }

        void close() {
            serial.closeDevice();
        }

        void setListenerMaxSize(int size) {
            listener.setMaxSize(size);
        }

        void setGetLength(std::function<size_t(const Head&)> _getLength) {
            listener.setGetLength(_getLength);
        }

        void setGetId(std::function<int(const Head&)> _getId) {
            listener.setGetId(_getId);
        }

        template<typename T>
        void registerSetter(T setter) {
            writer.registerSetter(setter);
        }

        template<typename T>
        void registerCallback(int id, T callback) {
            listener.registerCallback(id, callback);
        }

        template<typename T>
        void registerChecker(T checker) {
            listener.registerChecker(checker);
        }

        template<typename T>
        bool write(Head head, T t, Tail tail=Tail{}) {
            std::string s = writer.serialize(head, t, tail);
            return serial.writeString(s.c_str());
        }
    };
}


#endif //SERIALPRO_SERIALPRO_H
