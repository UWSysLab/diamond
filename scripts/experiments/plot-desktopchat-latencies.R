library(ggplot2)
library(plyr)

atomicWriter <- read.table("desktopchat-latency/atomic-writer-latencies.txt")
atomicReader <- read.table("desktopchat-latency/atomic-reader-latencies.txt")
transactionStaleWriter <- read.table("desktopchat-latency/transaction-stale-writer-latencies.txt")
transactionStaleReader <- read.table("desktopchat-latency/transaction-stale-reader-latencies.txt")
transactionNostaleWriter <- read.table("desktopchat-latency/transaction-nostale-writer-latencies.txt")
transactionNostaleReader <- read.table("desktopchat-latency/transaction-nostale-reader-latencies.txt")
baselineWriter <- read.table("desktopchat-latency/baseline-writer-latencies.txt")
baselineReader <- read.table("desktopchat-latency/baseline-reader-latencies.txt")

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

pdf("desktop-chat-latency-plots.pdf")

ggplot(data, aes(x=latency, color=factor(client), linetype=factor(client))) + stat_ecdf() + coord_fixed(ratio=0.33, xlim=c(0, 0.7)) + labs(x = "Latency (ms)", y = "CDF") +
    theme_bw() + theme(legend.title = element_blank())
#ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf()

ggplot(readers, aes(x=latency, color=factor(client), linetype=factor(client))) + stat_ecdf() + coord_fixed(ratio=0.33, xlim=c(0, 0.7)) + labs(x = "Latency (ms)", y = "CDF") +
    theme_bw() + theme(legend.title = element_blank())
#ggplot(readers, aes(x=latency, color=factor(client))) + stat_ecdf()

ggplot(writers, aes(x=latency, color=factor(client), linetype=factor(client))) + stat_ecdf() + coord_fixed(ratio=0.33, xlim=c(0, 0.7)) + labs(x = "Latency (ms)", y = "CDF") +
    theme_bw() + theme(legend.title = element_blank())
#ggplot(writers, aes(x=latency, color=factor(client))) + stat_ecdf()

#ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot() + coord_cartesian(ylim=c(0, 0.5))
#ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot()

dev.off()
