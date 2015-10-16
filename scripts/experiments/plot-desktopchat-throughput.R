library(ggplot2)
library(plyr)

nostaleData <- read.table("desktopchat-throughput/nostale-results.txt", header = TRUE)
nostaleAbortRates <- read.table("desktopchat-throughput/nostale-abortrate.txt", header = TRUE)
staleData <- read.table("desktopchat-throughput/stale-results.txt", header = TRUE)
staleAbortRates <- read.table("desktopchat-throughput/stale-abortrate.txt", header = TRUE)

nostaleData$staleness = "no stale"
nostaleAbortRates$staleness = "no stale"
staleData$staleness = "stale"
staleAbortRates$staleness = "stale"

throughputs = rbind(nostaleData, staleData)
abortRates = rbind(nostaleAbortRates, staleAbortRates)

pdf("desktop-chat-throughput-plots.pdf")

ggplot(throughputs, aes(x=clients, y=throughput, color=staleness)) + geom_line()
ggplot(abortRates, aes(x=clients, y=abortrate, color=staleness)) + geom_line()

dev.off()
