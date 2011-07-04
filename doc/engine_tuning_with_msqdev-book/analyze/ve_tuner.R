
# This script is used to perform a least squares fit of the data
# related to veTable1 to find the function that predicts the
# air fuel ratio.

# To run this script start R and source this file
#
# bash$ R
# > source("ve_tuner.R")
# Loading required package: MASS
# 
# Call: rlm(formula = afr1 ~ veCurr1 + fuelload + rpm, maxit = 40)
# Residuals:
#     Min      1Q  Median      3Q     Max 
# -4.1269 -0.4980 -0.1448  0.5697  3.0852 
# 
# Coefficients:
#             Value    Std. Error t value 
# (Intercept)  17.9873   0.0925   194.3936
# veCurr1      -0.1399   0.0023   -61.2773
# fuelload      0.0525   0.0016    33.8201
# rpm           0.0004   0.0000    25.2164
# 
# Residual standard error: 0.7837 on 12986 degrees of freedom
# >

# Once the coefficients are found a table can be built using
# the command below (with different values).
#
# bash$ msq-afr_table -afr 16.5 -a -0.1399 -b 0.0525 -c 0.0004 -d 17.9873

# Choose the file with the recorded data from msq-ve_tuner
#f1 <- "20110626-ve_tuner/msq-ve_tuner-20110626-16:47:48" # ok
f1 <- "20110626-ve_tuner/msq-ve_tuner-20110626-16:59:22" # good
#f1 <- "20110626-ve_tuner/msq-ve_tuner-20110626-17:12:36"  # ok

d1 <- read.csv(file=f1, head=TRUE, sep=",")


# Filter the data to remove invalid extreme values.

# Remove invalid extreme air fuel ratios
filt0 <- d1$afr1 > 10 & d1$afr1 < 16

# remove invalid idle and deceleration values
filt0 <- filt0 & d1$tps > 1

# remove near idle rpms
filt0 <- filt0 & d1$rpm > 2600

veCurr1  <- d1$veCurr1[filt0]  	# x
afr1 	 <- d1$afr1[filt0]  	# y
fuelload <- d1$fuelload[filt0]  # z
rpm 	 <- d1$rpm[filt0]


# account for delay in afr1
#dX <- 0
#dX <- 10  # ok
dX <- 20  # ok
#dX <- 40
#dX <- 60  # too far
# shift forward
afr1 <- afr1[(dX + 1):length(afr1)]
# shift back
veCurr1 <- veCurr1[1:(length(veCurr1) - dX)]
fuelload <- fuelload[1:(length(fuelload) - dX)]
rpm <- rpm[1:(length(rpm) - dX)]

#lmfit1 <- lm(formula = afr1 ~ veCurr1 + fuelload + rpm)
require(MASS)  # rlm
lmfit1 <- rlm(formula = afr1 ~ veCurr1 + fuelload + rpm, maxit=40)

print(summary(lmfit1))

