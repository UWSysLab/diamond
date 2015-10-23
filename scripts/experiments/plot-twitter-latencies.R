library(ggplot2)
library(plyr)
library(grid)

original <- read.table("twitter-latency/original.txt")
diamond <- read.table("twitter-latency/diamond.txt")
prefetch <- read.table("twitter-latency/prefetch.txt")
prefetchStale <- read.table("twitter-latency/prefetchstale.txt")

original$system = "Original  "
diamond$system = "Diamond  "
prefetch$system = "Diamond + P  "
prefetchStale$system = "Diamond + PS  "

data <- rbind(original, diamond, prefetch, prefetchStale)
data <- rename(data, c("V1" = "latency"))
data$system <- factor(data$system, levels = c("Original  ", "Diamond  ", "Diamond + P  ", "Diamond + PS  "))

pdf("android-twitter-latency-plots.pdf")

ggplot(data, aes(x=latency, linetype=factor(system), color=factor(system))) + stat_ecdf(size=1.4) + coord_fixed(ratio = 400, xlim = c(-20, 700)) +
    labs(x = "Latency (ms)", y = "CDF") + theme_bw() + theme(legend.title = element_blank(), legend.key.height = unit(8, units = "mm"), legend.key.width = unit(12, units = "mm")) +
    theme(legend.position="top", axis.text = element_text(size=14), axis.title = element_text(size=18), legend.text = element_text(size=14)) +
    scale_linetype_manual(values=c("solid", "dotted", "longdash", "dotdash")) + scale_color_manual(values=c("darkred", "blue", "darkgreen", "black"))
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf() + coord_fixed(ratio = 100, xlim = c(0, 300))
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf()

ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot() + coord_cartesian(ylim = c(0, 50))
ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot()

dev.off()
