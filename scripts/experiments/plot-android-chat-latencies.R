library(ggplot2)
library(plyr)

atomicWriter <- read.table("android-chat-latency/atomic-writes.txt")
atomicReader <- read.table("android-chat-latency/atomic-reads.txt")
transactionStaleWriter <- read.table("android-chat-latency/stale-writes.txt")
transactionStaleReader <- read.table("android-chat-latency/stale-reads.txt")
transactionNostaleWriter <- read.table("android-chat-latency/transaction-writes.txt")
transactionNostaleReader <- read.table("android-chat-latency/transaction-reads.txt")
baselineWriter <- read.table("android-chat-latency/baseline-writes.txt")
baselineReader <- read.table("android-chat-latency/baseline-reads.txt")

atomicWriter$client = "AtomicWriter";
atomicReader$client = "AtomicReader";
transactionStaleWriter$client = "StaleTransactionWriter";
transactionStaleReader$client = "StaleTransactionReader";
transactionNostaleWriter$client = "TransactionWriter";
transactionNostaleReader$client = "TransactionReader";
baselineWriter$client = "BaselineWriter";
baselineReader$client = "BaselineReader";

data <- rbind(atomicWriter, atomicReader, transactionStaleWriter, transactionStaleReader, transactionNostaleWriter, transactionNostaleReader, baselineWriter, baselineReader)
data <- rename(data, c("V1" = "latency"))

atomicWriter$client = "Atomic";
atomicReader$client = "Atomic";
transactionStaleWriter$client = "Transaction (stale)";
transactionStaleReader$client = "Transaction (stale)";
transactionNostaleWriter$client = "Transaction";
transactionNostaleReader$client = "Transaction";
baselineWriter$client = "Baseline";
baselineReader$client = "Baseline";

writers <- rbind(atomicWriter, transactionStaleWriter, transactionNostaleWriter, baselineWriter)
writers <- rename(writers, c("V1" = "latency"))

readers <- rbind(atomicReader, transactionStaleReader, transactionNostaleReader, baselineReader)
readers <- rename(readers, c("V1" = "latency"))

pdf("android-chat-latency-plots.pdf")

ggplot(data, aes(x=latency, color=factor(client), linetype=factor(client))) +
    stat_ecdf() +
    coord_fixed(ratio=40, xlim=c(-10, 75)) +
    labs(x = "Latency (ms)", y = "CDF") +
    theme_bw() +
    theme(legend.title = element_blank())

ggplot(readers, aes(x=latency, color=factor(client), linetype=factor(client))) +
    stat_ecdf() +
    coord_fixed(ratio=40, xlim=c(-10, 75)) +
    labs(x = "Latency (ms)", y = "CDF") +
    theme_bw() +
    theme(legend.title = element_blank())

ggplot(writers, aes(x=latency, color=factor(client), linetype=factor(client))) +
    stat_ecdf() +
    coord_fixed(ratio=40, xlim=c(-10, 75)) +
    labs(x = "Latency (ms)", y = "CDF") +
    theme_bw() +
    theme(legend.title = element_blank())

dev.off()
