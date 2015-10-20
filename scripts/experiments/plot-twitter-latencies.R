library(ggplot2)
library(plyr)

original <- read.table("twitter-latency/original-data.txt")
diamond <- read.table("twitter-latency/diamond-data.txt")

original$system = "original"
diamond$system = "Diamond"

data <- rbind(original, diamond)
data <- rename(data, c("V1" = "latency"))

pdf("android-twitter-latency-plots.pdf")

ggplot(data, aes(x=latency, linetype=factor(system), color=factor(system))) + stat_ecdf() + coord_fixed(ratio = 30, xlim = c(0, 70)) +
    labs(x = "Latency (ms)", y = "CDF") + theme_bw() + theme(legend.title = element_blank())
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf() + coord_fixed(ratio = 100, xlim = c(0, 300))
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf()

ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot() + coord_cartesian(ylim = c(0, 50))
ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot()

dev.off()
