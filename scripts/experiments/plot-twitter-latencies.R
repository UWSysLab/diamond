library(ggplot2)
library(plyr)

args <- commandArgs(TRUE)
if (length(args) != 2) {
    print("Usage: Rscript plot-twitter-latencies.R original_latencies diamond_latencies")
    q()
}

original <- read.table(args[1])
diamond <- read.table(args[2])

original$system = "original"
diamond$system = "diamond"

data <- rbind(original, diamond)
data <- rename(data, c("V1" = "latency"))

pdf("R-plots.pdf")

ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf()
ggplot(data, aes(x=latency, color=factor(system))) + stat_ecdf() + coord_cartesian(xlim = c(0, 50))

ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot()
ggplot(data, aes(x=factor(system), y=latency)) + geom_boxplot() + coord_cartesian(ylim = c(0, 50))

dev.off()
