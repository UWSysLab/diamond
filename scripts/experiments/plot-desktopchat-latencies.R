library(ggplot2)
library(plyr)

atomicWriter <- read.table("desktopchat-latency/atomic-writer-latencies.txt")
atomicReader <- read.table("desktopchat-latency/atomic-reader-latencies.txt")
transactionWriter <- read.table("desktopchat-latency/transaction-writer-latencies.txt")
transactionReader <- read.table("desktopchat-latency/transaction-reader-latencies.txt")
baselineWriter <- read.table("desktopchat-latency/baseline-writer-latencies.txt")
baselineReader <- read.table("desktopchat-latency/baseline-reader-latencies.txt")

atomicWriter$client = "AtomicWriter";
atomicReader$client = "AtomicReader";
transactionWriter$client = "TransactionWriter";
transactionReader$client = "TransactionReader";
baselineWriter$client = "BaselineWriter";
baselineReader$client = "BaselineReader";

data <- rbind(atomicWriter, atomicReader, transactionWriter, transactionReader, baselineWriter, baselineReader)
data <- rename(data, c("V1" = "latency"))

pdf("desktop-chat-latency-plots.pdf")

ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf() + coord_cartesian(xlim=c(0, 0.5))
ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf()

ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot() + coord_cartesian(ylim=c(0, 0.5))
ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot()

dev.off()
