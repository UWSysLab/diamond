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

systems <- c(
"Original",
"Diamond",
"Diamond + P",
"Diamond + PS"
)
avgLatencies <- c(
mean(original$V1),
mean(diamond$V1),
mean(prefetch$V1),
mean(prefetchStale$V1)
)
barGraphData <- data.frame(systems, avgLatencies)
barGraphData$systems <- factor(barGraphData$systems, levels = c("Original", "Diamond", "Diamond + P", "Diamond + PS"))

ggplot(barGraphData, aes(x=systems, y=avgLatencies, fill=factor(systems))) +
    geom_bar(stat="identity") +
    coord_cartesian() +
    labs(y = "Latency (ms)") +
    theme_bw() +
    theme(legend.position = "none") +
    theme(axis.title.x = element_blank()) +
    theme(axis.text = element_text(size=14), axis.title = element_text(size=18)) +
    theme(aspect.ratio = 0.66)
    
    
    #theme(legend.title = element_blank(), legend.key.height = unit(8, units = "mm"), legend.key.width = unit(12, units = "mm")) +
    #theme(legend.position="top", axis.text = element_text(size=14), axis.title = element_text(size=18), legend.text = element_text(size=14)) +
    #scale_linetype_manual(values=c("solid", "dotted", "longdash", "dotdash")) +
    #scale_color_manual(values=c("darkred", "blue", "darkgreen", "black"))

dev.off()
