#ifndef BENCHMARK_COMMON_H
#define BENCHMARK_COMMON_H

#include <fstream>
#include <random>
#include <string>
#include <sys/time.h>

void initRand();
int randInt(int lowerBound, int upperBound);
void parseKeys(const std::string &keyFile, int nKeys, std::vector<std::string> &keys);
std::string getRandomKey();
uint64_t getMilliseconds(const struct timeval &time);
uint64_t currentTimeMillis();

#endif
