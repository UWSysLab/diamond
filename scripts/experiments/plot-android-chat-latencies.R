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

client <- c("AtomicWriter",
    "AtomicReader",
    "StaleWriter",
    "StaleReader",
    "TransactionWriter",
    "TransactionReader",
    "BaselineWriter",
    "BaselineReader")
avgLatency <- c(mean(atomicWriter$V1),
    mean(atomicReader$V1),
    mean(transactionStaleWriter$V1),
    mean(transactionStaleReader$V1),
    mean(transactionNostaleWriter$V1),
    mean(transactionNostaleReader$V1),
    mean(baselineWriter$V1),
    mean(baselineReader$V1))
barGraphData <- data.frame(client, avgLatency)

readerClient <- c(
"AtomicReader",
"StaleReader",
"TransactionReader",
"BaselineReader"
)
readerAvgLatency <- c(
mean(atomicReader$V1),
mean(transactionStaleReader$V1),
mean(transactionNostaleReader$V1),
mean(baselineReader$V1)
)
readerBarGraphData <- data.frame(readerClient, readerAvgLatency)

writerClient <- c(
"AtomicWriter",
"StaleWriter",
"TransactionWriter",
"BaselineWriter"
)
writerAvgLatency <- c(
mean(atomicWriter$V1),
mean(transactionStaleWriter$V1),
mean(transactionNostaleWriter$V1),
mean(baselineWriter$V1)
)
writerBarGraphData <- data.frame(writerClient, writerAvgLatency)

pdf("android-chat-latency-plots.pdf")

ggplot(barGraphData, aes(x=client, y=avgLatency, fill=client)) +
    geom_bar(stat="identity") +
    coord_cartesian() +
    labs(x = "Action", y = "Latency (ms)") +
    theme_bw() +
    theme(legend.title = element_blank())

ggplot(readerBarGraphData, aes(x=readerClient, y=readerAvgLatency, fill=readerClient)) +
    geom_bar(stat="identity") +
    coord_cartesian() +
    labs(x = "Reader type", y = "Latency (ms)") +
    theme_bw() +
    theme(legend.title = element_blank()) +
    guides(fill=FALSE)

ggplot(writerBarGraphData, aes(x=writerClient, y=writerAvgLatency, fill=writerClient)) +
    geom_bar(stat="identity") +
    coord_cartesian() +
    labs(x = "Writer type", y = "Latency (ms)") +
    theme_bw() +
    theme(legend.title = element_blank()) +
    guides(fill=FALSE)

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
