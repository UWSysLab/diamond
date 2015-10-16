library(ggplot2)
library(plyr)

atomicWriter <- read.table("desktopchat-latency/atomic-writer-latencies.txt")
atomicReader <- read.table("desktopchat-latency/atomic-reader-latencies.txt")
transactionWriter <- read.table("desktopchat-latency/transaction-writer-latencies.txt")
transactionReader <- read.table("desktopchat-latency/transaction-reader-latencies.txt")

atomicWriter$client = "AtomicWriter";
atomicReader$client = "AtomicReader";
transactionWriter$client = "TransactionWriter";
transactionReader$client = "TransactionReader";

data <- rbind(atomicWriter, atomicReader, transactionWriter, transactionReader)
data <- rename(data, c("V1" = "latency"))

pdf("desktopchat-plots.pdf")

ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf()
ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf() + coord_cartesian(xlim=c(0, 0.5))

ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot()
ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot() + coord_cartesian(ylim=c(0, 0.5))

dev.off()
