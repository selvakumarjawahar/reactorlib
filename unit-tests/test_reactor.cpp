#include "event_handler.h"
#include "reactor.h"
#include "utils.h"
#include <catch2/catch.hpp>

#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

const std::string Sensor_FIFO { "/tmp/sensor_fifo" };
using namespace lib_reactor;

class SensorHandler : public EpollHandlerInterface {
public:
    explicit SensorHandler(EpollReactor& reactor)
        : reactor_ref { reactor }
    {
        read_fifo_fd = open(Sensor_FIFO.c_str(), O_RDONLY | O_NONBLOCK);
        if (read_fifo_fd < 0) {
            std::string err_str { strerror(errno) };
            throw err_str;
        }
    }

    void HandleEvent(uint32_t event) noexcept override
    {
        if (!(event & EPOLLIN)) {
            std::cout << "wrong event = " << event << '\n';
            return;
        }
        const auto result = read(read_fifo_fd, &recvd_val, sizeof(char));
        if (result < 0) {
            std::cerr << "Error in Reading: " << strerror(errno) << '\n';
            return;
        }
        /*
         * In real application, here we will do the handling of the read data.
         * In testing we want to end the event loop, so we will remove this 
         * handler from reactor.
         */
        reactor_ref.RemoveHandler(this);
    }

    int GetHandle() const noexcept override
    {
        return read_fifo_fd;
    }

    char GetRecvdVal() const noexcept
    {
        return recvd_val;
    }

    ~SensorHandler()
    {
        close(read_fifo_fd);
    }

private:
    EpollReactor& reactor_ref;
    int read_fifo_fd = -1;
    char recvd_val = 0;
};

void simulate_sensor(char send_val)
{
    int write_fifo_fd = open(Sensor_FIFO.c_str(), O_WRONLY);
    if (write_fifo_fd < 0) {
        std::cerr << "Error in opening the write fd: " << strerror(errno) << '\n';
        return;
    }
    const char val = send_val;
    auto result = write(write_fifo_fd, &val, sizeof(char));
    if (result < 0) {
        std::cerr << "Error in writing data: " << strerror(errno) << '\n';
        return;
    }
    close(write_fifo_fd);
}

TEST_CASE("SmokeTest", "Simple Use case")
{
    if (mkfifo(Sensor_FIFO.c_str(), 0666) < 0) {
        CHECK(false);
        return;
    }
    EpollReactor reactor;
    SensorHandler sensor_handler { reactor };
    CHECK(reactor.RegisterHandler(&sensor_handler) == ErrorCode::Ok);
    const char to_send = '5';
    std::thread simulator { simulate_sensor, to_send };
    reactor.HandleEvents();
    simulator.join();
    CHECK(sensor_handler.GetRecvdVal() == to_send);
}
