
# Configure the files containing the data to be analyzed.
f1 <- "msq-accel-20110624-12:41:31-rtdata-single"
f2 <- "msq-accel-20110624-12:42:06-rtdata-single"
#
# To run this script start R and source this file
# user$ R
# > source("accel-analyze.R")
# ... plot output ...
#

d1 <- read.csv(file=f1, head=TRUE, sep=",")
d2 <- read.csv(file=f2, head=TRUE, sep=",")

# names(d1)

# find the ranges
x_max <- min(c(max(d1$rpm), max(d2$rpm)));
x_min <- max(c(min(d1$rpm), min(d2$rpm)));
# reduce the range by 10% on each end
x_max <- x_max - (x_max * .10)
x_min <- x_min + (x_min * .10)

# filter out the data
filt1 <- d1$rpm > x_min & d1$rpm < x_max
filt2 <- d2$rpm > x_min & d2$rpm < x_max
x1 <- d1$rpm[filt1]
x2 <- d2$rpm[filt2]
t1 <- d1$localtime[filt1]
t2 <- d2$localtime[filt2]

# rescale the times so that they both start at zero
t1 <- t1 - min(t1)
t2 <- t2 - min(t2)
t_min <- 0
t_max <- max(c(max(t1), max(t2)))

# least squares fit
nlsfit1 <- nls(x1 ~ a*t1 + b)
nlsfit2 <- nls(x2 ~ a*t2 + b)
# calculate the values
fx1 <- coef(nlsfit1)[1]*t1 + coef(nlsfit1)[2]
fx2 <- coef(nlsfit2)[1]*t2 + coef(nlsfit2)[2]

# The function with the steepest slope would be the more optimal
# solution since it maximizes acceleration.
m1 <- coef(nlsfit1)[1]
m2 <- coef(nlsfit2)[1]

# plot the results
title <- "comparision of two accelerations"

plot(t1, x1, col="red", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main=title, xlab="time(s)", ylab="rpm")
par(new=T)
plot(t1, fx1, col="red", type="l", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main="", xlab="", ylab="")
par(new=T)
plot(t2, x2, col="blue", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main="", xlab="", ylab="")
par(new=T)
plot(t2, fx2, col="blue", type="l", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main="", xlab="", ylab="")
par(new=F)

