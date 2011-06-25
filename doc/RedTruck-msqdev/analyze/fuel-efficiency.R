
# What exactly must be maximized in order to maximize fuel efficiency?
# Running leaner consumes less fuel but it also produces less power.
# Running rich beyond the point of maximum power would obviously
# have poor fuel efficency because it is consuming more fuel and
# producing less power.
# But where is the maximum fuel efficency, is it some ratio of rpm vs fuelload?

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
f1 <- "plotdata-veTable1-20110609.15:50:52"
#f1 <- "plotdata-veTable1-20110609.16:16:33"

#title <- "rpm vs advance"
title <- "fuel efficiency?"
#xlab <- "advance"
xlab <- "veCurr1"
ylab <- "rpm/fuelload"

d1 <- read.csv(file=f1, head=TRUE, sep=",")

x <- d1$veCurr1
y1 <- d1$rpm
y2 <- d1$fuelload
y <- y1/y2
#y <- y2

#xlim <- c(min(d1$advance), max(d1$advance))
xlim <- c(min(x), max(x))
#ylim <- c(min(y), max(y) + 20) # also add some room for the max text above the plot
ylim <- c(min(y), max(y)) # also add some room for the max text above the plot

nlsfit <- nls(y ~ a*x^2 + b*x + c, data=d1, start=list(a=1,b=1,c=1))
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


#plot(d1$advance, d1$rpm, xlim=xlim, ylim=ylim, main=title, xlab=xlab, ylab=ylab,
plot(x, y, xlim=xlim, ylim=ylim, main=title, xlab=xlab, ylab=ylab,
		col="red", pch=4)
par(new=T)
plot(x1, y1, type="l", xlim=xlim, ylim=ylim, main="", xlab="", ylab="",
		col="blue")
#text(x_at_max, y_at_max + 40, labels=paste("max=(", round(x_at_max, 2), ", ", round(y_at_max, 2), ")", sep=""))
par(new=F)

