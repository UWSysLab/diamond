library(ggplot2)
library(plyr)
library(grid)

nostaleData <- read.table("desktopchat-throughput/nostale-results.txt", header = TRUE)
nostaleAbortRates <- read.table("desktopchat-throughput/nostale-abortrate.txt", header = TRUE)
staleData <- read.table("desktopchat-throughput/stale-results.txt", header = TRUE)
staleAbortRates <- read.table("desktopchat-throughput/stale-abortrate.txt", header = TRUE)
baselineData <- read.table("desktopchat-throughput/baseline-results.txt", header = TRUE)

nostaleData$staleness = "Diamond  "
nostaleAbortRates$staleness = "Diamond  "
staleData$staleness = "Diamond + S  "
staleAbortRates$staleness = "Diamond + S  "
baselineData$staleness = "baseline  "

throughputs = rbind(nostaleData, baselineData, staleData)
abortRates = rbind(nostaleAbortRates, staleAbortRates)

pdf("desktop-chat-throughput-plots.pdf")

ggplot(throughputs, aes(x=clients, y=throughput, color=staleness, linetype=staleness)) + geom_line(size=1.8) + coord_fixed(ratio=0.0002) +
    labs(x = "Number of clients", y = "Throughput (actions/sec)") + theme_bw() + theme(legend.title = element_blank(), legend.key.height = unit(8, units = "mm"), legend.key.width = unit(12, units = "mm")) +
    theme(legend.position="top", axis.text = element_text(size=14), axis.title = element_text(size=18), legend.text = element_text(size=14)) +
    scale_linetype_manual(values=c("solid", "dotted", "longdash")) + scale_color_manual(values=c("darkred", "blue", "darkgreen"))
ggplot(abortRates, aes(x=clients, y=abortrate, color=staleness, linetype=staleness)) + geom_line() + coord_fixed(ratio=10) +
    labs(x = "Number of clients", y = "Abort rate") + theme_bw() + theme(legend.title = element_blank())

dev.off()
