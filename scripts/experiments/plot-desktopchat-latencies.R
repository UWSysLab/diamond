library(ggplot2)
library(plyr)

atomicWriter <- read.table("desktopchat-atomic-writer-latencies.txt")
atomicReader <- read.table("desktopchat-atomic-reader-latencies.txt")
transactionWriter <- read.table("desktopchat-transaction-writer-latencies.txt")
transactionReader <- read.table("desktopchat-transaction-reader-latencies.txt")

atomicWriter$client = "AtomicWriter";
atomicReader$client = "AtomicReader";
transactionWriter$client = "TransactionWriter";
transactionReader$client = "TransactionReader";

data <- rbind(atomicWriter, atomicReader, transactionWriter, transactionReader)
data <- rename(data, c("V1" = "latency"))

pdf("desktopchat-plots.pdf")

ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf()

ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot()

dev.off()
