library(ggplot2)
library(plyr)

original <- read.table("twitter-latency/original.txt")
diamond <- read.table("twitter-latency/diamond.txt")
prefetch <- read.table("twitter-latency/prefetch.txt")

original$system = "original"
diamond$system = "Diamond"
prefetch$system = "Diamond (prefetching)"

data <- rbind(original, diamond, prefetch)
data <- rename(data, c("V1" = "latency"))

pdf("android-twitter-latency-plots.pdf")

ggplot(data, aes(x=latency, linetype=factor(system), color=factor(system))) + stat_ecdf() + coord_fixed(ratio = 400, xlim = c(-20, 1000)) +
    labs(x = "Latency (ms)", y = "CDF") + theme_bw() + theme(legend.title = element_blank())
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf() + coord_fixed(ratio = 100, xlim = c(0, 300))
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf()

ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot() + coord_cartesian(ylim = c(0, 50))
ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot()

dev.off()
