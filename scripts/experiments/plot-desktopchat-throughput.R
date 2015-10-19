library(ggplot2)
library(plyr)

nostaleData <- read.table("desktopchat-throughput/nostale-results.txt", header = TRUE)
nostaleAbortRates <- read.table("desktopchat-throughput/nostale-abortrate.txt", header = TRUE)
staleData <- read.table("desktopchat-throughput/stale-results.txt", header = TRUE)
staleAbortRates <- read.table("desktopchat-throughput/stale-abortrate.txt", header = TRUE)
baselineData <- read.table("desktopchat-throughput/baseline-results.txt", header = TRUE)

nostaleData$staleness = "Diamond"
nostaleAbortRates$staleness = "Diamond"
staleData$staleness = "Diamond (stale)"
staleAbortRates$staleness = "Diamond (stale)"
baselineData$staleness = "baseline"

throughputs = rbind(nostaleData, staleData, baselineData)
abortRates = rbind(nostaleAbortRates, staleAbortRates)

pdf("desktop-chat-throughput-plots.pdf")

ggplot(throughputs, aes(x=clients, y=throughput, color=staleness)) + geom_line() + coord_fixed(ratio=0.0004) + labs(x = "Number of clients", y = "Throughput (actions/sec)", color = "")
ggplot(abortRates, aes(x=clients, y=abortrate, color=staleness)) + geom_line() + coord_fixed(ratio=10) + labs(x = "Number of clients", y = "Abort rate", color = "")

dev.off()
