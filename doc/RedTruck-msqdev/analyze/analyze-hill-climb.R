
#
# To execute this script:
#
#  shell$ R
#  > source("analyze-hill-climb.R")
#

#plot(x, y)
#cor(x, y)

#f1 <- "plotdata-20110608.23:14:59"
#f1 <- "plotdata-20110609.14:31:27"
#f1 <- "plotdata-veTable1-20110609.15:50:52"
#f1 <- "plotdata-veTable1-20110609.16:16:33"

#f1 <- "plotdata-veTable1-20110613.14:55:42"
#f1 <- "plotdata-veTable1-20110613.14:56:09"
#
#f1 <-"plotdata-veTable1-20110613.15:07:53"  # 5.3
#f1 <-"plotdata-veTable1-20110613.15:08:40"  # 0.4  *

# error shift
#f1 <-"plotdata-veTable1-20110613.15:09:07"  # 2.6  *
#f1 <-"plotdata-veTable1-20110613.15:10:14"  # 3.9
#f1 <-"plotdata-veTable1-20110613.15:10:53"  # 4.7
#f1 <-"plotdata-veTable1-20110613.15:11:31"  # 1.0
#f1 <-"plotdata-veTable1-20110613.15:12:08"  # 3.4
#f1 <-"plotdata-veTable1-20110613.15:12:47"  # 2.6
#f1 <-"plotdata-veTable1-20110613.15:13:20"  # 0.4
#f1 <-"plotdata-veTable1-20110613.15:14:39" # 2.4, ****
#f1 <-"plotdata-veTable1-20110613.15:15:14"  # 4.7, *
#f1 <-"plotdata-veTable1-20110613.15:15:46"  # 1.4
#f1 <-"plotdata-veTable1-20110613.15:16:38"  # 1.8
#f1 <-"plotdata-veTable1-20110613.15:17:10"  # 2.8  ***
#f1 <-"plotdata-veTable1-20110613.15:17:54"

# throttle stop
f1 <- "20110613-veTable1-throttle_stop/plotdata-veTable1-20110615.20:26:57"


#f1 <-"plotdata-veTable1-20110610.13:26:53"
 # tps_range: 19.9, 22.2
 # max=(64.66, 3416.46)

#title <- "rpm vs advance"
title <- "rpm vs veCurr1"
#xlab <- "advance"
xlab <- "veCurr1"
ylab <- "rpm"

d1 <- read.csv(file=f1, head=TRUE, sep=",")


tps_range = c(min(d1$tps), max(d1$tps))
Dtps = max(d1$tps) - min(d1$tps)

x <- d1$veCurr1
y <- d1$rpm

#xlim <- c(min(d1$advance), max(d1$advance))
xlim <- c(min(x), max(x))
ylim <- c(min(y), max(y) + 20) # also add some room for the max text above the plot

nlsfit <- nls(y ~ a*x^2 + b*x + c, data=d1, start=list(a=5,b=5,c=5))
#summary(nlsfit)
coef(nlsfit)

a <- coef(nlsfit)[1]
b <- coef(nlsfit)[2]
c <- coef(nlsfit)[3]

# To find the maximum the function is differentiated (not show)
# and then solved for when it equals zero.
x_at_max <- -b/(2 * a)
y_at_max <- a * x_at_max^2 + b * x_at_max + c

# convert the function to points
#x1 <- seq(min(d1$advance), max(d1$advance), by=0.001)
x1 <- seq(min(x), max(x), by=0.001)
y1 <- a*x1^2 + b*x1 + c

# Leave these commented out to display the plot.
# Uncomment them to produce output to a file.
# Be sure to uncomment dev.off for file output.
#pdf(paste(f1, ".pdf", sep=""))
#png(paste(f1, ".png", sep=""))  # png works well with LaTeX pdf output
#postscript(paste(f1, ".eps", sep=""))
#jpeg(paste(f1, ".jpg", sep=""))

#plot(d1$advance, d1$rpm, xlim=xlim, ylim=ylim, main=title, xlab=xlab, ylab=ylab,
plot(x, y, xlim=xlim, ylim=ylim, main=title, xlab=xlab, ylab=ylab,
		col="red", pch=4)
par(new=T)
plot(x1, y1, type="l", xlim=xlim, ylim=ylim, main="", xlab="", ylab="",
		col="blue")
#text(x_at_max, y_at_max + 40, labels=paste("max=(", round(x_at_max, 2), ", ", round(y_at_max, 2), ")", sep=""))
par(new=F)

#summary(d1)
#d1$advance
#names(d1)  # column names

#fit <- lm(x ~ y)
#summary(fit)
#dev.off()
