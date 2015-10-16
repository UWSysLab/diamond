library(ggplot2)
library(plyr)

data <- read.table("desktopchat-throughput/throughput-results.txt", header = TRUE)
abortRates <- read.table("desktopchat-throughput/abortrate-results.txt", header = TRUE)

pdf("desktop-chat-throughput-plots.pdf")

ggplot(data, aes(x=clients, y=throughput)) + geom_line()
ggplot(abortRates, aes(x=clients, y=abortrate)) + geom_line()

dev.off()
