// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * bench/terminal.cc:
 *   A terminal client for a distributed transactional store.
 *
 **********************************************************************/

#include "store/common/frontend/client.h"
#include "client/diamondclient.h"

using namespace std;

int
main(int argc, char **argv)
{
    const char *configPath = NULL;

    Client *client;

    int opt;
    while ((opt = getopt(argc, argv, "c:")) != -1) {
        switch (opt) {
        case 'c': // Configuration path
        { 
            configPath = optarg;
            break;
        }
        default:
            fprintf(stderr, "Unknown argument %s\n", argv[optind]);
            break;
        }
    }

    client = new diamond::DiamondClient(configPath);

    char c, cmd[2048], *tok;
    int clen, status;
    string key, value;

    while (1) {
        printf(">> ");
        fflush(stdout);

        clen = 0;
        while ((c = getchar()) != '\n')
            cmd[clen++] = c;
        cmd[clen] = '\0';

        if (clen == 0) continue;

        tok = strtok(cmd, " ,.-");

        if (strcasecmp(tok, "exit") == 0 || strcasecmp(tok, "q") == 0) {
            printf("Exiting..\n");
            break;
        } else if (strcasecmp(tok, "get") == 0) {
            tok = strtok(NULL, " ,.-");
            key = string(tok);

            status = client->Get(key, value);

            if (status == 0) {
                printf("%s -> %s\n", key.c_str(), value.c_str());
            } else {
                printf("Error in retrieving value\n");
            }
        } else if (strcasecmp(tok, "put") == 0) {
            tok = strtok(NULL, " ,.-");
            key = string(tok);
            tok = strtok(NULL, " ,.-");
            value = string(tok);
            client->Put(key, value);
        } else if (strcasecmp(tok, "begin") == 0) {
            client->Begin();
        } else if (strcasecmp(tok, "commit") == 0) {
            bool status = client->Commit();
            if (status) {
                printf("Commit succeeded..\n");
            } else {
                printf("Commit failed..\n");
            }
        } else {
            printf("Unknown command.. Try again!\n");
        }
        fflush(stdout);
    }

    exit(0);
    return 0;
}
