#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <pthread.h>
#include <signal.h>

#define LISTEN_PORT	5001
#define BUFFER_SIZE 128

typedef struct {
        unsigned long total;
        unsigned long nonIdle;
        unsigned long idle;
} cpu_stats;

/* assuming for this that stats has enough storage space */
int getCpuStats(cpu_stats *stats)
{
        FILE *statFile;
        char buf[256];
        unsigned long user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

        statFile = fopen("/proc/stat", "rt");
        if (statFile < 0) {
                exit(-1);
        }
        bzero(buf, 256);
        /* Getting the first line here, but this code could be improved to
           look at the version of the OS to make sure of the response format */
        fgets(buf, 255, statFile);
        // TODO: check for errors.
        fclose(statFile);
        /* strtol on a split string would be safer, since there is an overflow problem
           using sscanf, buf for this sample should be sufficient */
        sscanf(buf, "cpu  %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
        stats->idle = idle + iowait;
        stats->nonIdle = user + nice + system + irq + softirq + steal;
        stats->total = stats->idle + stats->nonIdle;
        return 0; /* Could return non-zero if there are errors */
}

double getCpuLoad()
{       
        cpu_stats firstStats, secondStats;
        double totalDiff, idleDiff;
        
        getCpuStats(&firstStats);
        sleep(1); /* let some time pass */
        getCpuStats(&secondStats);
        /* TODO: make sure there are not any errors getting CPU stats */

        totalDiff = (double)(secondStats.total - firstStats.total);
        idleDiff = (double)(secondStats.idle - firstStats.idle);
        return (totalDiff - idleDiff) / totalDiff * 100.0f;
}

/* Returns used memory in KB */
unsigned long getMemoryUsage()
{
        FILE *memFile;
        char buf[256];
        unsigned long memTotal, memFree, buffers, cached;

        memFile = fopen("/proc/meminfo", "rt");
        if (memFile < 0) {
                return -1; /* TODO: need better error handling */
        }

        /* Expecting MemTotal & MemFree on first and second line,
           Buffers and Cached on lines fourth and fifth lines,
           but this code could be improved to look at the version of
           the OS to make sure of the response format */

        /* Generally, not a great idea to use sscanf, but for simplicity
                using it here.  Normally would prefer to find the value and
                use strtol */
        fgets(buf, 255, memFile); // MemTotal
        sscanf(buf, "MemTotal: %lu", &memTotal);
        fgets(buf, 255, memFile); // MemFree
        sscanf(buf, "MemFree: %lu", &memFree);
        fgets(buf, 255, memFile); // MemAvailable - ignore this line
        fgets(buf, 255, memFile); // Buffers
        sscanf(buf, "Buffers: %lu", &buffers);
        fgets(buf, 255, memFile); // Cached
        sscanf(buf, "Cached: %lu", &cached);
        // TODO: check for errors.
        fclose(memFile);
        return memTotal - memFree - buffers - cached;
}

void *handleClientCommands(void *arg)
{
	int socketFd = *(int *)arg;
	char incoming[BUFFER_SIZE];
	char outgoing[BUFFER_SIZE];
	char keepRunning = 1;
	int bytesRead;

	while(keepRunning) {
		bytesRead = read(socketFd, incoming, BUFFER_SIZE);
		if (bytesRead < 0)
		{
			perror("Error reading from socket");
			keepRunning = 0;
			continue;
		}

		if (!strncmp("cpu\n", incoming, 4)) {
			snprintf(outgoing, BUFFER_SIZE-1, "CPU Load: %.1f%%", getCpuLoad());
		} else if (!strncmp("mem\n", incoming, 4)) {
			snprintf(outgoing, BUFFER_SIZE-1, "Memory Usage: %lu kB", getMemoryUsage());
		} else {
			snprintf(outgoing, BUFFER_SIZE-1, "Unknown command. Use 'mem' or 'cpu'");
		}

		if (write(socketFd, outgoing, strlen(outgoing)+1) < 0) 
		{
			perror("Error writing to socket");
			keepRunning = 0;
			continue;
		}
	}
	close(socketFd);
	pthread_exit(NULL);
}

int main()
{
	int optval;
	struct sockaddr_in myAddress;
	struct sockaddr_in clientAddress;
	socklen_t clientAddrLen;
	int clientSocket;
	int sd;

	/* start daemon */
	switch (fork ())
	{
	case -1:
		perror ("Cannot fork");
		exit(-1);
		break;
	case 0: //child process
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		if (setsid () == -1) //get new session
		{
			exit(-1);
		}
		break;
	default: // Parent can now exit
		exit(0);
	}

 	signal(SIGPIPE, SIG_IGN);

	/* open a socket and wait for a client */
	sd = socket (AF_INET, SOCK_STREAM, 0);

	bzero ((char *) &myAddress, sizeof (myAddress));
	myAddress.sin_family = AF_INET;
	myAddress.sin_port   = htons(LISTEN_PORT);

	if (bind (sd, (struct sockaddr *) &myAddress, sizeof (myAddress)) < 0)
	{
		perror("bind failed - daemon already running? Address already in use?");
		exit(-1);
	}

	listen (sd, 3); /* allow up to 3 requests to queue */

	 /* This daemon will need to handle some signals - since it cannot be unloaded cleanly */
	while (1)
	{
		pthread_t thread;
		clientAddrLen = sizeof (clientAddress);
		clientSocket = accept (sd, (struct sockaddr *)&clientAddress, &clientAddrLen);
		if (clientSocket < 0)
		{
			perror("accept failed");
			continue; /* Don't exit.  Just try again */
		}
		optval = 1;
		setsockopt (clientSocket, SOL_SOCKET, SO_KEEPALIVE,(char *) &optval, sizeof (optval));
		
		/* TODO: use a thread pool to ensure we don't spawn endless threads */
		if(pthread_create(&thread, NULL, handleClientCommands, &clientSocket) != 0) {
           perror("Failed to create thread");
		   continue; // Again - better error handling will be needed
		}
	}
}

