
# to run this script:
# user$ R
# > source("accel-analyze.R")
# ... plot output ...

f1 <- "msq-accel-tmp-20110625-15:21:06-rtdata-single"

d1 <- read.csv(file=f1, head=TRUE, sep=",")

# Comment out png and devoff if you want the plot displayed
# instead of output to a file.
png(paste(f1, ".png", sep=""))

t1 <- d1$localtime
t1 <- t1 - t1[1]

plot(t1, d1$afr1,
			main="air fuel ratio during steady acceleration",
			xlab="time(s)",
			ylab="air fuel ratio")

dev.off()  # needed in order for png() to work
