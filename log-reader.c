#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
// #include <sys/types.h>
// #include <signal.h>

#define BUFSIZE64 64
#define BUFSIZE32 32
#define RABBITMQ_ROUTING_KEY "service_status"

struct Service
{
    char *name;
    char *onlineString;
    char *offlineString;
};

void publish(const char *serviceName, const int newStatus);

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        printf("Error: Missing argument, required: \n");
        printf("\tSERVICE\tONLINE_PATTERN_STRING\tOFFLINE_PATTERN_STRING\n");
        return EXIT_FAILURE;
    }

    struct Service service = {
        .name = argv[1],
        .onlineString = argv[2],
        .offlineString = argv[3]};

    pid_t pid = 0;
    int pipefd[2];
    FILE *output;
    char line[512];

    pipe(pipefd); // create a pipe
    pid = fork(); // spawn a child process

    if (pid == 0)
    {
        // Child. Let's redirect its standard output to our pipe and replace process
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        execl("/usr/bin/journalctl", "/usr/bin/journalctl", "-fqn0", "-o", "cat", "-u", service.name, (char *)NULL);
    }

    // Only parent gets here. Listen to what the child says
    close(pipefd[1]);
    output = fdopen(pipefd[0], "r");

    printf("%s log reader started with PID %d\n", service.name, getpid());

    char *match = NULL;
    int status = 0;
    int previousStatus = status;

    // listen to what the process writes to stdout
    while (fgets(line, sizeof(line), output))
    {
        match = strstr(line, service.onlineString);
        // Check for "online" pattern match
        if (match != NULL)
        {
            status = 1;
        }

        match = strstr(line, service.offlineString);
        // Check for "offline" pattern match
        if (match != NULL)
        {
            status = 0;
        }

        // Avoid spamming same status
        if (status == previousStatus)
            continue;

        publish(service.name, status);
        previousStatus = status;
        match = NULL;

        // if you need to kill the tail application, just kill it:
        // if (something_goes_wrong)
        //     kill(pid, SIGKILL);
    }

    // Wait for the child process to terminate
    waitpid(pid, NULL, 0);

    return EXIT_SUCCESS;
}

void publish(const char *serviceName, const int newStatus)
{
    // Read it every time just in case the value changes while the program's running
    char *rabbitMqUri = getenv("STATUS_WATCHDOG_RABBITMQ_URI");

    if (rabbitMqUri == NULL)
    {
        printf("Error: Could not find RabbitMQ Uri environmental variable");
        return;
    }

    // Prepare the command params
    char uriBuffer[BUFSIZE64];
    snprintf(uriBuffer, sizeof(uriBuffer), "--uri=\"%s\"", rabbitMqUri);

    char exchangeBuffer[BUFSIZE32];
    snprintf(exchangeBuffer, sizeof(exchangeBuffer), "--exchange=%s", "\"\"");

    char routingKeyBuffer[BUFSIZE32];
    snprintf(routingKeyBuffer, sizeof(routingKeyBuffer), "--routing-key=\"%s\"", RABBITMQ_ROUTING_KEY);

    char jsonMessageBodyBuffer[BUFSIZE64]; // Adjusted size
    snprintf(jsonMessageBodyBuffer, sizeof(jsonMessageBodyBuffer), "{\"service\":\"%s\",\"status\":%d}", serviceName, newStatus);

    char bodyBuffer[BUFSIZE64 + 16];
    snprintf(bodyBuffer, sizeof(bodyBuffer), "--body=\"%s\"", jsonMessageBodyBuffer);

    // Used for checking necessary buffer sizes when programming
    // char *temp = "--body={service:projectzomboid,status:1}";

    printf("%s\n", jsonMessageBodyBuffer);

    // Concat everything together
    char command[256];
    snprintf(command, sizeof(command), "%s %s %s %s %s",
             "/usr/local/bin/amqp-publish",
             uriBuffer,
             exchangeBuffer,
             routingKeyBuffer,
             bodyBuffer);

    int responseCode = system(command);

    if (responseCode != EXIT_SUCCESS)
    {
        printf("\n>>>> Error %d: Failed to publish message to RabbitMQ\n", responseCode);
    }
}
