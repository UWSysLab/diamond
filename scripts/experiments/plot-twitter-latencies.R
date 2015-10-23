library(ggplot2)
library(plyr)

original <- read.table("twitter-latency/original.txt")
diamond <- read.table("twitter-latency/diamond.txt")
prefetch <- read.table("twitter-latency/prefetch.txt")
prefetchStale <- read.table("twitter-latency/prefetchstale.txt")

original$system = "original  "
diamond$system = "Diamond  "
prefetch$system = "Prefetch  "
prefetchStale$system = "Prefetch and stale  "

data <- rbind(original, diamond, prefetch, prefetchStale)
data <- rename(data, c("V1" = "latency"))
data$system <- factor(data$system, levels = c("original  ", "Diamond  ", "Prefetch  ", "Prefetch and stale  "))

pdf("android-twitter-latency-plots.pdf")

ggplot(data, aes(x=latency, linetype=factor(system), color=factor(system))) + stat_ecdf(size=0.75) + coord_fixed(ratio = 400, xlim = c(-20, 700)) +
    labs(x = "Latency (ms)", y = "CDF") + theme_bw() + theme(legend.title = element_blank()) +
    theme(legend.position="top", axis.text = element_text(size=14), axis.title = element_text(size=18), legend.text = element_text(size=14))
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf() + coord_fixed(ratio = 100, xlim = c(0, 300))
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf()

ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot() + coord_cartesian(ylim = c(0, 50))
ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot()

dev.off()
