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
int rand_key();
int rand_unwriteable_key();

bool ready = false;
double alpha = -1;
double *zipf;
bool readyUnwriteable = false; // state for zipf distribution for unwriteable keys
double *zipfUnwriteable;

vector<string> keys;
vector<string> unwriteableKeys;
int nKeys = 100;
int nUnwriteableKeys = 10;
int nWriteableKeys = 90;

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
        MODE_DOCC,
        MODE_SNAPSHOT,
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
            nKeys = strtoul(optarg, &strtolPtr, 10);
            nUnwriteableKeys = nKeys / 10;
            nWriteableKeys = nKeys - nUnwriteableKeys;
            if ((*optarg == '\0') || (*strtolPtr != '\0') ||
                (nKeys <= 0)) {
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
            } else if (strcasecmp(optarg, "docc") == 0) {
                mode = MODE_DOCC;
            } else if (strcasecmp(optarg, "snapshot") == 0) {
                mode = MODE_SNAPSHOT;
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
    } else if (mode == MODE_DOCC) {
        client->SetIsolationLevel(LINEARIZABLE);
        docc = true;
    } else if (mode == MODE_SNAPSHOT) {
        client->SetIsolationLevel(SNAPSHOT_ISOLATION);
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
    for (int i = 0; i < nKeys; i++) {
        getline(in, key);
        if (i < nUnwriteableKeys) {
            unwriteableKeys.push_back(key);
        }
        else {
            keys.push_back(key);
        }
    }
    in.close();

    struct timeval t0, t1, t2;
    int nTransactions = 0; // Number of transactions attempted.
    int ttype; // Transaction type.
    int ret;
    vector<int> keyIdx;

    gettimeofday(&t0, NULL);
    srand(t0.tv_sec + t0.tv_usec);

    while (1) {
        keyIdx.clear();
            
        // Begin a transaction.
        gettimeofday(&t1, NULL);

        // Decide which type of retwis transaction it is going to be.
        ttype = rand() % 100;

        if (ttype < 1) {
            // 1% - Add user transaction. 1,3
            client->Begin();
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            sort(keyIdx.begin(), keyIdx.end());
            if (docc) {
                client->Increment(keys[keyIdx[0]], INCR_VALUE); // 1 increment
                client->Put(keys[keyIdx[1]], PUT_VALUE); // 2 writes
                client->Put(keys[keyIdx[2]], PUT_VALUE);
            }
            else {
                if ((ret = client->Get(keys[keyIdx[0]], value))) {
                    Panic("Aborting due to %s %d", keys[keyIdx[0]].c_str(), ret);
                }

                for (int i = 0; i < 3; i++) {
                    client->Put(keys[keyIdx[i]], PUT_VALUE);
                }
            }
            ttype = 1;
        } else if (ttype < 6) {
            // 5% - Follow/Unfollow transaction. 2,2
            client->Begin();
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            sort(keyIdx.begin(), keyIdx.end());

            if (docc) {
                client->Increment(keys[keyIdx[0]], INCR_VALUE); // 2 increments
                client->Increment(keys[keyIdx[1]], INCR_VALUE);
            }
            else {
                vector<string> readKeys;
                map<string, string> readValues;
                for (int i = 0; i < 2; i++) {
                    readKeys.push_back(keys[keyIdx[i]]);
                }
                if ((ret = client->MultiGet(readKeys, readValues))) { // 2 reads
                    Panic("Aborting due to multiget %d", ret);
                }

                for (int i = 0; i < 2; i++) { // 2 writes
                    client->Put(keys[keyIdx[i]], PUT_VALUE);
                }
            }
            ttype = 2;
        } else if (ttype < 30) {
            // 24% - Post tweet transaction. 3,5
            client->Begin();
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            keyIdx.push_back(rand_key());
            sort(keyIdx.begin(), keyIdx.end());

            int unwriteableKeyIdx = rand_unwriteable_key();

            if (docc) {
                if ((ret = client->Get(unwriteableKeys[unwriteableKeyIdx], value))) { // 1 read (UNWRITEABLE KEY)
                    Panic("Aborting due to %s %d", unwriteableKeys[unwriteableKeyIdx].c_str(), ret);
                }

                for (int i = 0; i < 5; i++) { // 5 increments
                    client->Increment(keys[keyIdx[i]], INCR_VALUE);
                }
                client->Put(keys[keyIdx[5]], PUT_VALUE); // 1 blind write
            }
            else {
                if ((ret = client->Get(unwriteableKeys[unwriteableKeyIdx], value))) { // 1 read (UNWRITEABLE KEY)
                    Panic("Aborting due to %s %d", unwriteableKeys[unwriteableKeyIdx].c_str(), ret);
                }

                vector<string> readKeys;
                map<string, string> readValues;
                for (int i = 0; i < 5; i++) {
                    readKeys.push_back(keys[keyIdx[i]]);
                }
                if ((ret = client->MultiGet(readKeys, readValues))) { // 5 reads
                    Panic("Aborting due to multiget %d", ret);
                }
                for (int i = 0; i < 5; i++) { // 5 writes
                    client->Put(keys[keyIdx[i]], keys[keyIdx[i]]);
                }

                client->Put(keys[keyIdx[5]], PUT_VALUE); // 1 blind write
            }
            ttype = 3;
        } else if (ttype < 80) {
            // 50% - Get followers/timeline transaction. rand(1,10),0
            client->BeginRO();
            int nGets = 1 + rand() % 10;
            for (int i = 0; i < nGets; i++) {
                keyIdx.push_back(rand_key());
            }

            sort(keyIdx.begin(), keyIdx.end());

            vector<string> readKeys;
            map<string, string> readValues;
            for (int i = 0; i < nGets; i++) {
                readKeys.push_back(keys[keyIdx[i]]);
            }
            if ((ret = client->MultiGet(readKeys, readValues))) {
                Panic("Aborting due to multiget %d", ret);
            }
            ttype = 4;
        } else {
            // 20% - Like transaction. 1,1
            client->Begin();
            keyIdx.push_back(rand_key());
            sort(keyIdx.begin(), keyIdx.end());

            if (docc) {
                client->Increment(keys[keyIdx[0]], INCR_VALUE); // 1 increment
            }
            else {
                if ((ret = client->Get(keys[keyIdx[0]], value))) { // 1 read
                    Panic("Aborting due to %s %d", keys[keyIdx[0]].c_str(), ret);
                }
                client->Put(keys[keyIdx[0]], PUT_VALUE); // 1 write
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

int rand_key()
{
    if (alpha < 0) {
        // Uniform selection of keys.
        return (rand() % nWriteableKeys);
    } else {
        // Zipf-like selection of keys.
        if (!ready) {
            zipf = new double[nWriteableKeys];

            double c = 0.0;
            for (int i = 1; i <= nWriteableKeys; i++) {
                c = c + (1.0 / pow((double) i, alpha));
            }
            c = 1.0 / c;

            double sum = 0.0;
            for (int i = 1; i <= nWriteableKeys; i++) {
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
        int l = 0, r = nWriteableKeys, mid;
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

int rand_unwriteable_key()
{
    if (alpha < 0) {
        // Uniform selection of keys.
        return (rand() % nUnwriteableKeys);
    } else {
        // Zipf-like selection of keys.
        if (!readyUnwriteable) {
            zipfUnwriteable = new double[nUnwriteableKeys];

            double c = 0.0;
            for (int i = 1; i <= nUnwriteableKeys; i++) {
                c = c + (1.0 / pow((double) i, alpha));
            }
            c = 1.0 / c;

            double sum = 0.0;
            for (int i = 1; i <= nUnwriteableKeys; i++) {
                sum += (c / pow((double) i, alpha));
                zipfUnwriteable[i-1] = sum;
            }
            readyUnwriteable = true;
        }

        double random = 0.0;
        while (random == 0.0 || random == 1.0) {
            random = (1.0 + rand())/RAND_MAX;
        }

        // binary search to find key;
        int l = 0, r = nUnwriteableKeys, mid;
        while (l < r) {
            mid = (l + r) / 2;
            if (random > zipfUnwriteable[mid]) {
                l = mid + 1;
            } else if (random < zipfUnwriteable[mid]) {
                r = mid - 1;
            } else {
                break;
            }
        }
        return mid;
    }
}
