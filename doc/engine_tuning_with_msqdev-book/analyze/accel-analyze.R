
#
# To run this script start R and source this file
# user$ R
# > source("accel-analyze.R")
# ... plot output ...
#
# Configure the files containing the data to be analyzed.
# Choose a f1 and f2 then source this file to plot it.
# 

f1 <- "20110627-msq-accel/msq-accel-4g_52cm_45-65mph-20110627-20:17:43-rtdata-single"
f2 <- "20110627-msq-accel/msq-accel-4g_52cm_45-65mph-20110627-20:20:18-rtdata-single"

d1 <- read.csv(file=f1, head=TRUE, sep=",")
d2 <- read.csv(file=f2, head=TRUE, sep=",")

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
require(MASS)
#nlsfit1 <- nls(x1 ~ a*t1 + b)
fit1 <- rlm(formula = x1 ~ t1)
#nlsfit2 <- nls(x2 ~ a*t2 + b)
fit2 <- rlm(formula = x2 ~ t2)
# calculate the values
fx1 <- coef(fit1)[2]*t1 + coef(fit1)[1]
fx2 <- coef(fit2)[2]*t2 + coef(fit2)[1]

# The function with the steepest slope would be the more optimal
# solution since it maximizes acceleration.
m1 <- coef(fit1)[1]
m2 <- coef(fit2)[1]


# plot the results
title <- "Acceleration Tuning"

# Leave these commented out to display the plot.
# Uncomment them to produce output to a file.
# Be sure to uncomment dev.off for file output.
pdf(file=paste(f1, ".pdf", sep=""), height=5, width=5)
#png(paste(f1, ".png", sep=""))
#postscript(paste(f1, ".eps", sep=""))
#jpeg(paste(f1, ".jpg", sep=""))

# point type
ptype <- 20  # small dot

linew <- 2   # line width

par(family="serif")
plot(t1, x1, col="red", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main=title, xlab="time(s)", ylab="rpm", pch=ptype)
grid(nx=20, ny=20)

par(new=T)

plot(t2, x2, col="blue", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main="", xlab="", ylab="", pch=ptype)

par(new=T)

plot(t1, fx1, col="yellow", type="l", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main="", xlab="", ylab="", lwd=linew)

par(new=T)

plot(t2, fx2, col="green", type="l", ylim=c(x_min, x_max), xlim=c(t_min, t_max), main="", xlab="", ylab="", lwd=linew)

#par(new=T)
legend("bottomright", c("data 1", "data 2", "fit 1", "fit 2"), pch=c(ptype, ptype, NA_integer_, NA_integer_), lty=c(0,0,1,1), col=c("red", "blue", "yellow", "green"), bty="o", cex=0.8)

par(new=F)

dev.off()
