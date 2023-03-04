//
// Created by bismarck on 12/15/22.
//

#ifndef SERIALPRO_SERIALPRO_H
#define SERIALPRO_SERIALPRO_H

#define MAX_READ_ONCE_CHAR 40

#include <atomic>
#include <string>
#include <thread>

#include "msg_serialize.h"
#include "serialib.h"

namespace sp {
    class SerialException : public std::exception {
    public:
        explicit SerialException(const char *what)
                : m_what(what) {}

        const char *what() const noexcept override {
            return m_what;
        }

    private:
        const char *m_what;
    };

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
                char buffer[MAX_READ_ONCE_CHAR];
                int len = serial.readBytes(buffer, MAX_READ_ONCE_CHAR, 1);
                if (len > 0) {
                    listener.push(buffer, len);
                } else if (len < 0) {
                    throw SerialException("Serial Port Read ERROR!");
                }
            }
        }

        void open(const std::string &port, int baud) {
            if (serial.openDevice(port.c_str(), baud) < 0) {
                throw SerialException("Cannot Open Serial Port!");
            }
        }

    public:
        serialPro() = default;

        serialPro(const std::string &port, int baud) {
            open(port, baud);
            running = true;
        }

        serialPro(serialPro &&other) noexcept {
            listener = std::move(other.listener);
            writer = std::move(other.writer);
            serial = std::move(other.serial);
            readThread = std::move(other.readThread);
            running.exchange(other.running);
        }

        serialPro &operator=(serialPro &&other) noexcept {
            if (this != &other) {
                listener = std::move(other.listener);
                writer = std::move(other.writer);
                serial = std::move(other.serial);
                readThread = std::move(other.readThread);
                running.exchange(other.running);
            }
            return *this;
        }

        serialPro(serialPro &_) = delete;

        serialPro &operator=(serialPro &_) = delete;

        ~serialPro() {
            close();
        }

        void spin(bool background) {
            if (background) {
                running = true;
                readThread = std::thread(&serialPro::readLoop, this);
            } else {
                readLoop();
            }
        }

        void spinOnce() {
            while (true) {
                int len = serial.available();
                if (len == 0) {
                    break;
                } else if (len > MAX_READ_ONCE_CHAR) {
                    len = MAX_READ_ONCE_CHAR;
                }
                char buffer[MAX_READ_ONCE_CHAR];
                len = serial.readBytes(buffer, len, 1);
                if (len > 0) {
                    listener.push(buffer, len);
                } else if (len < 0) {
                    throw SerialException("Serial Port Read ERROR!");
                }
            }
        }

        void close() {
            running = false;
            if (readThread.joinable()) {
                readThread.join();
            }
            serial.closeDevice();
        }

        template<typename T>
        bool write(Head head, T t, Tail tail = Tail{}) {
            std::string s = writer.serialize(head, t, tail);
            return serial.writeBytes(s.c_str(), s.size()) == 1;
        }

        template<typename T>
        void registerCallback(int id, T callback) {
            listener.registerCallback(id, callback);
        }

        void registerErrorHandle(std::function<void(int, const std::string &)> func) {
            listener.registerErrorHandle(func);
        }

    protected:
        void setListenerMaxSize(int size) {
            listener.setMaxSize(size);
        }

        void setGetLength(std::function<size_t(const Head &)> _getLength) {
            listener.setGetLength(_getLength);
        }

        void setGetId(std::function<int(const Head &)> _getId) {
            listener.setGetId(_getId);
        }

        template<typename T>
        void registerSetter(T setter) {
            writer.registerSetter(setter);
        }

        template<typename T>
        void registerChecker(T checker) {
            listener.registerChecker(checker);
        }
    };
}


#endif //SERIALPRO_SERIALPRO_H
