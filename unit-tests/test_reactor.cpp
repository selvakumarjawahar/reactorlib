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

/*
 * This is the concrete implementation of the EpollHandler Interface
 */
class SensorHandler : public EpollHandlerInterface {
public:
    /*
     * We will be setting up FIFO and will read from this fifo
     * Sensors will be sending data over this fifo
     */
    explicit SensorHandler(EpollReactor& reactor)
        : reactor_ref { reactor }
    {
        read_fifo_fd = open(Sensor_FIFO.c_str(), O_RDONLY | O_NONBLOCK);
        if (read_fifo_fd < 0) {
            std::string err_str { strerror(errno) };
            throw err_str;
        }
    }

    /*
     * This will be called by the reactor, when there is a event.
     * This is just a test implementation where we just read and 
     * save the value. As this is the test, we want to exit from 
     * the event loop, so once we receive a message, we remove the
     * registered handler.
     */
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

// Simple sensor simulator
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

/*
 * This is a smoke test. But this is how application main will
 * look like
 */
TEST_CASE("SmokeTest", "Simple Use case")
{
    // Setup FIFO.
    if (mkfifo(Sensor_FIFO.c_str(), 0666) < 0) {
        CHECK(false);
        return;
    }

    // Instantiate reactor
    EpollReactor reactor;

    // Instantiate sensor handler.
    SensorHandler sensor_handler { reactor };

    // Register sensor handler with reactor
    CHECK(reactor.RegisterHandler(&sensor_handler) == ErrorCode::Ok);
    const char to_send = '5';

    // Start the sensor simulator thread
    std::thread simulator { simulate_sensor, to_send };

    /* Start the reactor event loop. The loop will exit when no more
     * event handlers are in the list. Our dummy event handler, removes 
     * itself once it receives a message from sensor. So once handler
     * receives the message, the event loop will exit.
     */
    reactor.HandleEvents();

    simulator.join();

    // Check we received the correct message
    CHECK(sensor_handler.GetRecvdVal() == to_send);
}
