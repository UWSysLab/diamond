// -*- mode: c++; c-file-style: "k&r"; c-basic-offset: 4 -*-
/***********************************************************************
 *
 * store/benchmark/retwisClient.cc:
 *   Retwis benchmarking client for a distributed transactional store.
 *
 **********************************************************************/

#include "client/diamondclient.h"
#include <algorithm>

using namespace std;

// Function to pick a random key according to some distribution.
// Niel: this client uses Zipf distributions over three separate sets of keys,
// so I changed rand_key() to use passed-in state to cut down on duplicate
// code. rand_key() should always be called with the same values of ready and
// zipf. The three functions below behave the same way as the old rand_key()
// for the three zipf distributions.
int rand_key(const int nKeys, const double alpha, bool &ready, double *zipf);
int rand_read_key();
int rand_write_key();
int rand_increment_key();

// State for the three zipf distributions
double alpha = -1;
bool readyRead = false;
double *zipfRead;
bool readyWrite = false;
double *zipfWrite;
bool readyIncrement = false;
double *zipfIncrement;

vector<string> readKeys;
vector<string> writeKeys;
vector<string> incrementKeys;
int nTotalKeys = 100;
int nReadKeys = 10;
int nWriteKeys = 70;
int nIncrementKeys = 20;

const std::string PUT_VALUE("1");
const int INCR_VALUE = 1;

int
main(int argc, char **argv)
{
    const char *configPath = NULL;
    const char *keysPath = NULL;
    int duration = 10;
    int nShards = 1;
    int closestReplica = -1; // Closest replica id.
    int skew = 0; // difference between real clock and TrueTime
    int error = 0; // error bars
    bool docc = false;

    diamond::DiamondClient *client;
    enum {
        MODE_UNKNOWN,
        MODE_LINEARIZABLE,
        MODE_LINEARIZABLE_DOCC,
        MODE_SNAPSHOT,
        MODE_SNAPSHOT_DOCC,
        MODE_EVENTUAL
    } mode = MODE_UNKNOWN;

    int opt;
    while ((opt = getopt(argc, argv, "c:d:N:k:f:m:e:s:z:r:")) != -1) {
        switch (opt) {
        case 'c': // Configuration path
        { 
            configPath = optarg;
            break;
        }

        case 'f': // Generated keys path
        { 
            keysPath = optarg;
            break;
        }

        case 'N': // Number of shards.
        { 
            char *strtolPtr;
            nShards = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (nShards <= 0)) {
                fprintf(stderr, "option -N requires a numeric arg\n");
            }
            break;
        }

        case 'd': // Duration in seconds to run.
        { 
            char *strtolPtr;
            duration = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (duration <= 0)) {
                fprintf(stderr, "option -d requires a numeric arg\n");
            }
            break;
        }

        case 'k': // Number of keys to operate on.
        {
            char *strtolPtr;
            nTotalKeys = strtoul(optarg, &strtolPtr, 10);
            nReadKeys = nTotalKeys / 10;
            nIncrementKeys = 2 * (nTotalKeys / 10);
            nWriteKeys = nTotalKeys - nReadKeys - nIncrementKeys;
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (nTotalKeys <= 0)) {
                fprintf(stderr, "option -k requires a numeric arg\n");
            }
            break;
        }

        case 's': // Simulated clock skew.
        {
            char *strtolPtr;
            skew = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (skew < 0))
            {
                fprintf(stderr,
                        "option -s requires a numeric arg\n");
            }
            break;
        }

        case 'e': // Simulated clock error.
        {
            char *strtolPtr;
            error = strtoul(optarg, &strtolPtr, 10);
            if ((*optarg == '\0') || (*strtolPtr != '\0') || (error < 0))
            {
                fprintf(stderr,
                        "option -e requires a numeric arg\n");
            }
            break;
        }

        case 'z': // Zipf coefficient for key selection.
        {
            char *strtolPtr;
            alpha = strtod(optarg, &strtolPtr);
            if ((*optarg == '\0') || (*strtolPtr != '\0'))
            {
                fprintf(stderr,
                        "option -z requires a numeric arg\n");
            }
            break;
        }

        case 'r': // Preferred closest replica.
        {
            char *strtolPtr;
            closestReplica = strtod(optarg, &strtolPtr);
            if ((*optarg == '\0') || (*strtolPtr != '\0'))
            {
                fprintf(stderr,
                        "option -r requires a numeric arg\n");
            }
            break;
        }

        case 'm': // Mode to run in [occ/lock/...]
        {
            if (strcasecmp(optarg, "linearizable") == 0) {
                mode = MODE_LINEARIZABLE;
            } else if (strcasecmp(optarg, "linearizabledocc") == 0) {
                mode = MODE_LINEARIZABLE_DOCC;
            } else if (strcasecmp(optarg, "snapshot") == 0) {
                mode = MODE_SNAPSHOT;
            } else if (strcasecmp(optarg, "snapshotdocc") == 0) {
                mode = MODE_SNAPSHOT_DOCC;
            } else if (strcasecmp(optarg, "eventual") == 0) {
                mode = MODE_EVENTUAL;
            } else {
                fprintf(stderr, "unknown mode '%s'\n", optarg);
                exit(0);
            }
            break;
        }

        default:
            fprintf(stderr, "Unknown argument %s\n", argv[optind]);
            break;
        }
    }

    client = new diamond::DiamondClient(configPath);

    if (mode == MODE_LINEARIZABLE) {
        client->SetIsolationLevel(LINEARIZABLE);
    } else if (mode == MODE_LINEARIZABLE_DOCC) {
        client->SetIsolationLevel(LINEARIZABLE);
        docc = true;
    } else if (mode == MODE_SNAPSHOT) {
        client->SetIsolationLevel(SNAPSHOT_ISOLATION);
    } else if (mode == MODE_SNAPSHOT_DOCC) {
        client->SetIsolationLevel(SNAPSHOT_ISOLATION);
        docc = true;
    } else if (mode == MODE_EVENTUAL) {
        client->SetIsolationLevel(EVENTUAL);
    } else {
        fprintf(stderr, "option -m is required\n");
        exit(0);
    }

    // Read in the keys from a file.
    string key, value;
    ifstream in;
    in.open(keysPath);
    if (!in) {
        fprintf(stderr, "Could not read keys from: %s\n", keysPath);
        exit(0);
    }
    for (int i = 0; i < nTotalKeys; i++) {
        getline(in, key);
        if (i < nReadKeys) {
            readKeys.push_back(key);
        }
        else if (i < nReadKeys + nWriteKeys) {
            writeKeys.push_back(key);
        }
        else {
            incrementKeys.push_back(key);
        }
    }
    in.close();
    printf("DEBUG %lu %lu %lu\n", readKeys.size(), writeKeys.size(), incrementKeys.size());

    zipfRead = new double[nReadKeys];
    zipfWrite = new double[nWriteKeys];
    zipfIncrement = new double[nIncrementKeys];

    struct timeval t0, t1, t2;
    int nTransactions = 0; // Number of transactions attempted.
    int ttype; // Transaction type.
    int ret;
    vector<int> readKeyIdx;
    vector<int> writeKeyIdx;
    vector<int> incrementKeyIdx;

    gettimeofday(&t0, NULL);
    //srand(t0.tv_sec + t0.tv_usec);
    srand(1); // DEBUG TODO TODO REMOVE ME

    while (1) {
        readKeyIdx.clear();
        writeKeyIdx.clear();
        incrementKeyIdx.clear();
            
        // Begin a transaction.
        gettimeofday(&t1, NULL);

        // Decide which type of retwis transaction it is going to be.
        ttype = rand() % 100;

        if (ttype < 1) {
            // 1% - Add user transaction. 1,3
            client->Begin();
            incrementKeyIdx.push_back(rand_increment_key());
            writeKeyIdx.push_back(rand_write_key());
            writeKeyIdx.push_back(rand_write_key());
            sort(writeKeyIdx.begin(), writeKeyIdx.end());
            sort(incrementKeyIdx.begin(), incrementKeyIdx.end());

            if (docc) {
                client->Increment(incrementKeys[incrementKeyIdx[0]], INCR_VALUE); // 1 increment
                client->Put(writeKeys[writeKeyIdx[0]], PUT_VALUE); // 2 writes
                client->Put(writeKeys[writeKeyIdx[1]], PUT_VALUE);
            }
            else {
                if ((ret = client->Get(incrementKeys[incrementKeyIdx[0]], value))) { // 1 "increment"
                    Panic("Aborting due to %s %d", incrementKeys[incrementKeyIdx[0]].c_str(), ret);
                }
                client->Put(incrementKeys[incrementKeyIdx[0]], PUT_VALUE);

                for (int i = 0; i < 2; i++) { // 2 writes
                    client->Put(writeKeys[writeKeyIdx[i]], PUT_VALUE);
                }
            }
            ttype = 1;
        } else if (ttype < 6) {
            // 5% - Follow/Unfollow transaction. 2,2
            client->Begin();
            incrementKeyIdx.push_back(rand_increment_key());
            incrementKeyIdx.push_back(rand_increment_key());
            sort(incrementKeyIdx.begin(), incrementKeyIdx.end());

            if (docc) {
                client->Increment(incrementKeys[incrementKeyIdx[0]], INCR_VALUE); // 2 increments
                client->Increment(incrementKeys[incrementKeyIdx[1]], INCR_VALUE);
            }
            else {
                vector<string> multiGetKeys;
                map<string, string> multiGetValues;
                for (int i = 0; i < 2; i++) {
                    multiGetKeys.push_back(incrementKeys[incrementKeyIdx[i]]);
                }
                if ((ret = client->MultiGet(multiGetKeys, multiGetValues))) { // 2 "increments"
                    Panic("Aborting due to multiget %d", ret);
                }
                for (int i = 0; i < 2; i++) {
                    client->Put(incrementKeys[incrementKeyIdx[i]], PUT_VALUE);
                }
            }
            ttype = 2;
        } else if (ttype < 30) {
            // 24% - Post tweet transaction. 3,5
            client->Begin();
            readKeyIdx.push_back(rand_read_key());
            incrementKeyIdx.push_back(rand_increment_key());
            incrementKeyIdx.push_back(rand_increment_key());
            incrementKeyIdx.push_back(rand_increment_key());
            incrementKeyIdx.push_back(rand_increment_key());
            incrementKeyIdx.push_back(rand_increment_key());
            writeKeyIdx.push_back(rand_write_key());
            sort(readKeyIdx.begin(), readKeyIdx.end());
            sort(writeKeyIdx.begin(), writeKeyIdx.end());
            sort(incrementKeyIdx.begin(), incrementKeyIdx.end());

            if (docc) {
                if ((ret = client->Get(readKeys[readKeyIdx[0]], value))) { // 1 read (UNWRITEABLE KEY)
                    Panic("Aborting due to %s %d", readKeys[readKeyIdx[0]].c_str(), ret);
                }

                for (int i = 0; i < 5; i++) { // 5 increments
                    client->Increment(incrementKeys[incrementKeyIdx[i]], INCR_VALUE);
                }
                client->Put(writeKeys[writeKeyIdx[0]], PUT_VALUE); // 1 blind write
            }
            else {
                if ((ret = client->Get(readKeys[readKeyIdx[0]], value))) { // 1 read (UNWRITEABLE KEY)
                    Panic("Aborting due to %s %d", readKeys[readKeyIdx[0]].c_str(), ret);
                }

                vector<string> multiGetKeys;
                map<string, string> multiGetValues;
                for (int i = 0; i < 5; i++) {
                    readKeys.push_back(incrementKeys[incrementKeyIdx[i]]);
                }
                if ((ret = client->MultiGet(multiGetKeys, multiGetValues))) { // 5 "increments"
                    Panic("Aborting due to multiget %d", ret);
                }
                for (int i = 0; i < 5; i++) {
                    client->Put(incrementKeys[incrementKeyIdx[i]], PUT_VALUE);
                }

                client->Put(writeKeys[writeKeyIdx[5]], PUT_VALUE); // 1 blind write
            }
            ttype = 3;
        } else if (ttype < 80) {
            // 50% - Get followers/timeline transaction. rand(1,10),0
            client->BeginRO();
            int nGets = 1 + rand() % 10;
            for (int i = 0; i < nGets; i++) {
                readKeyIdx.push_back(rand_read_key());
            }

            sort(readKeyIdx.begin(), readKeyIdx.end());

            vector<string> multiGetKeys;
            map<string, string> multiGetValues;
            for (int i = 0; i < nGets; i++) {
                multiGetKeys.push_back(readKeys[readKeyIdx[i]]);
            }
            if ((ret = client->MultiGet(multiGetKeys, multiGetValues))) {
                Panic("Aborting due to multiget %d", ret);
            }
            ttype = 4;
        } else {
            // 20% - Like transaction. 1,1
            client->Begin();
            incrementKeyIdx.push_back(rand_increment_key());
            sort(incrementKeyIdx.begin(), incrementKeyIdx.end());

            if (docc) {
                client->Increment(incrementKeys[incrementKeyIdx[0]], INCR_VALUE); // 1 increment
            }
            else {
                printf("Get on key %s", incrementKeys[incrementKeyIdx[0]].c_str());
                if ((ret = client->Get(incrementKeys[incrementKeyIdx[0]], value))) { // 1 read
                    Panic("Aborting due to %s %d", incrementKeys[incrementKeyIdx[0]].c_str(), ret);
                }
                client->Put(incrementKeys[incrementKeyIdx[0]], PUT_VALUE); // 1 write
            }
            ttype = 5;
        }

        bool status = client->Commit();
        gettimeofday(&t2, NULL);
        
        long latency = (t2.tv_sec - t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);

        fprintf(stdout, "%d %ld.%06ld %ld.%06ld %ld %d %d", ++nTransactions, t1.tv_sec,
                t1.tv_usec, t2.tv_sec, t2.tv_usec, latency, status?1:0, ttype);
        //int retries = (client->Stats())[0];

        //fprintf(stderr, "%d %ld.%06ld %ld.%06ld %ld %d %d %d", ++nTransactions, t1.tv_sec,
        //        t1.tv_usec, t2.tv_sec, t2.tv_usec, latency, status?1:0, ttype, retries);
        fprintf(stdout, "\n");

        if ( ((t2.tv_sec-t0.tv_sec)*1000000 + (t2.tv_usec-t0.tv_usec)) > duration*1000000) 
            break;
    }

    fprintf(stdout, "# Client exiting..\n");
    return 0;
}

int rand_key(const int nKeys, const double alpha, bool &ready, double *zipf)
{
    printf("DEBUG ready? %d\n", ready);
    if (alpha < 0) {
        // Uniform selection of keys.
        return (rand() % nKeys);
    } else {
        // Zipf-like selection of keys.
        if (!ready) {
            double c = 0.0;
            for (int i = 1; i <= nKeys; i++) {
                c = c + (1.0 / pow((double) i, alpha));
            }
            c = 1.0 / c;

            double sum = 0.0;
            for (int i = 1; i <= nKeys; i++) {
                sum += (c / pow((double) i, alpha));
                zipf[i-1] = sum;
            }
            ready = true;
        }

        double random = 0.0;
        while (random == 0.0 || random == 1.0) {
            random = (1.0 + rand())/RAND_MAX;
        }

        // binary search to find key;
        int l = 0, r = nKeys, mid;
        while (l < r) {
            mid = (l + r) / 2;
            if (random > zipf[mid]) {
                l = mid + 1;
            } else if (random < zipf[mid]) {
                r = mid - 1;
            } else {
                break;
            }
        }
        return mid;
    } 
}

int rand_read_key()
{
    return rand_key(nReadKeys, alpha, readyRead, zipfRead);
}

int rand_write_key()
{
    return rand_key(nWriteKeys, alpha, readyWrite, zipfWrite);
}

int rand_increment_key()
{
    return rand_key(nIncrementKeys, alpha, readyIncrement, zipfIncrement);
}
