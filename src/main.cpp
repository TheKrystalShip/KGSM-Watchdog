#include "queue.h"
#include "csv.h"

#include <iostream>
#include <string>
#include <array>
#include <format>

#include <chrono>
#include <thread>
#include <vector>
#include <filesystem>

#define JOURNALCTL_COMMAND "/usr/bin/journalctl -fqn0 -o cat -u "
#define INIT_STATUS -1

typedef TKS::Parsers::CSV::Row csv_row;
typedef TKS::Concurrency::ConcurrentQueue<std::string> cc_string_q;

int initReaderThread(cc_string_q &m_queue, csv_row const &row);
int initPublisherThread(cc_string_q &m_queue);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cout << "ERROR: No argument for file passed, exiting\n";
        return EXIT_FAILURE;
    }

    std::string fileName{argv[1]};

    if (!std::filesystem::exists(fileName))
    {
        std::cout << "ERROR: file " << fileName << " not found, exiting\n";
        return EXIT_FAILURE;
    }

    cc_string_q m_queue;
    std::ifstream file(fileName);

    // Reader threads
    std::vector<std::thread> threads;
    for (auto row : TKS::Parsers::CSV::Range(file))
    {
        threads.push_back(std::thread(
            [&m_queue, row]()
            { initReaderThread(m_queue, row); }));
    }

    // Publisher thread
    std::thread publisherThread = std::thread(
        [&m_queue]()
        { initPublisherThread(m_queue); });

    std::cout << "Threads created, listening for output\n";

    // Prevent closing
    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(1));

    return EXIT_SUCCESS;
}

int initReaderThread(cc_string_q &m_queue, csv_row const &row)
{
    const std::string_view serviceName{row[0]};

    std::string command = JOURNALCTL_COMMAND;
    command.append(serviceName);

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);

    if (!pipe)
        throw std::runtime_error("popen() failed!");

    const std::string_view onlineString{row[1]};
    const std::string_view offlineString{row[2]};

    std::array<char, 128> buffer;
    int status{INIT_STATUS};
    int previousStatus = {status};

    while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr)
    {
        std::string output(buffer.begin(), buffer.end());

        if (output.find(onlineString) != std::string::npos)
            status = 1;

        else if (output.find(offlineString) != std::string::npos)
            status = 0;

        if (status == previousStatus)
            continue;

        std::string message = "\'{";
        message.append(std::format(R"("service":"{}","status":{})",
                                   std::string(serviceName),
                                   std::to_string(status)));
        message.append("}\'");

        m_queue.push(message);

        previousStatus = status;
    }

    std::cout << "Reader thread exited\n";

    return EXIT_SUCCESS;
}

int initPublisherThread(cc_string_q &m_queue)
{
    char *rabbitMqUri = std::getenv("STATUS_WATCHDOG_RABBITMQ_URI");
    char *rabbitMqRoutingKey = std::getenv("STATUS_WATCHDOG_RABBITMQ_ROUTING_KEY");

    if (rabbitMqUri == NULL || rabbitMqRoutingKey == NULL)
    {
        std::cout << "Error: Could not read RabbitMQ environmental variables\n";
        return EXIT_FAILURE;
    }

    while (true)
    {
        // m_queue.pop() blocks until the queue is not empty
        std::string message = m_queue.pop();
        std::cout << message << std::endl;

        std::string fullCommand = std::format(
            "{} --uri={} --exchange={} --routing-key={} --body=",
            "/usr/local/bin/amqp-publish", // amqp-publish
            rabbitMqUri,                   // --uri=
            "\"\"",                        // --exchange=
            rabbitMqRoutingKey             // --routing-key=
        );

        // Append body after to avoid losing content
        fullCommand.append(message);

        system(fullCommand.c_str());
    }

    std::cout << "Exited publisher thread\n";

    return EXIT_SUCCESS;
}
