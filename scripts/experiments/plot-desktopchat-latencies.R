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

atomicWriter$client = "AtomicW";
atomicReader$client = "AtomicR";
transactionStaleWriter$client = "TransStaleW";
transactionStaleReader$client = "TransStaleR";
transactionNostaleWriter$client = "TransNostaleW";
transactionNostaleReader$client = "TransNostaleR";
baselineWriter$client = "BaselineW";
baselineReader$client = "BaselineR";

data <- rbind(atomicWriter, atomicReader, transactionStaleWriter, transactionStaleReader, transactionNostaleWriter, transactionNostaleReader, baselineWriter, baselineReader)
data <- rename(data, c("V1" = "latency"))

writers <- rbind(atomicWriter, transactionStaleWriter, transactionNostaleWriter, baselineWriter)
writers <- rename(writers, c("V1" = "latency"))

readers <- rbind(atomicReader, transactionStaleReader, transactionNostaleReader, baselineReader)
readers <- rename(readers, c("V1" = "latency"))

pdf("desktop-chat-latency-plots.pdf")

ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf() + coord_cartesian(xlim=c(0, 0.7))
#ggplot(data, aes(x=latency, color=factor(client))) + stat_ecdf()

ggplot(readers, aes(x=latency, color=factor(client))) + stat_ecdf() + coord_cartesian(xlim=c(0, 0.7))
#ggplot(readers, aes(x=latency, color=factor(client))) + stat_ecdf()

ggplot(writers, aes(x=latency, color=factor(client))) + stat_ecdf() + coord_cartesian(xlim=c(0, 0.7))
#ggplot(writers, aes(x=latency, color=factor(client))) + stat_ecdf()

#ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot() + coord_cartesian(ylim=c(0, 0.5))
#ggplot(data, aes(x=factor(client), y=latency)) + geom_boxplot()

dev.off()
