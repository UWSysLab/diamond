#include "benchmark_common.h"

std::mt19937 rng;

void initRand() {
    rng.seed(std::random_device()());
}

int randInt(int lowerBound, int upperBound) {
    std::uniform_int_distribution<std::mt19937::result_type> dist(lowerBound, upperBound);
    return dist(rng);
}

void parseKeys(const std::string &keyFile, int nKeys, std::vector<std::string> &keys) {
    std::string key;
    std::ifstream in;
    in.open(keyFile.c_str());
    if (!in) {
        fprintf(stderr, "Could not read keys from: %s\n", keyFile.c_str());
        exit(0);
    }

    for (unsigned int i = 0; i < nKeys; i++) {
        std::getline(in, key);
        keys.push_back(key);
    }
    in.close();
}

uint64_t getMilliseconds(const struct timeval &time) {
    return (time.tv_sec * 1000 + time.tv_usec / 1000);
}

uint64_t currentTimeMillis() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return getMilliseconds(time);
}

std::string getRandomKey() {
    int range = 10 + 26 + 26; // digits + uppercase + lowercase
    char keyChars[65];
    for (int i = 0; i < 64; i++) {
        int index = randInt(0, range - 1);
        int finalVal = -1;
        if (index < 10) {
            finalVal = index + 48;
        }
        else if (index >= 10 && index < 36) {
            finalVal = index - 10 + 65;
        }
        else { // index >= 36
            finalVal = index - 36 + 97;
        }

        keyChars[i] = finalVal;
    }
    keyChars[64] = '\0';
    return std::string(keyChars);
}
