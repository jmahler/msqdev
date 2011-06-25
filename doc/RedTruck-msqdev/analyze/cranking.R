
#
# Calculations used to modify the cranking settings.
#

# temp, degF
temp <- c(10, 30, 50, 70, 90, 110, 130, 150, 170, 180)

# stock firmware, 3.1.x
crpulse <- c(326, 301, 276, 251, 226, 201, 176, 151, 126, 101)
prpulse <- c(6, 5.6, 5.2, 4.8, 4.4, 4.0, 3.6, 3.2, 2.6, 2.0)
ase <- c(45, 43, 41, 39, 37, 35, 33, 31, 28, 25)
asetaper <- c(350, 330, 310, 290, 270, 250, 230, 210, 180, 150)

#
# The description here [http://www.megamanual.com/v22manual/mtune.htm] is
# wrong according to [http://www.msextra.com/forums/viewtopic.php?f=131&t=39576].
#
# [http://www.msextra.com/doc/ms2extra/MS2-Extra_Tuning_Manual.html#start_idle]

#req_fuel <- 15.5
#t1 <- 170
#t2 <- -40
#crpulse1 <- req_fuel * .23
#crpulse2 <- 4 * crpulse1

# stock firmware
#t1 <- 9.9
#t2 <- 180
#crpulse1 <- 326
#crpulse2 <- 101

# #1 (start)
# [http://www.msextra.com/doc/ms2extra/MS2-Extra_Tuning_Manual.html#start_idle]
#
# date: Fri, 24 Jun 2011 16:07:30 -0700
# crank time: not great
# host start: helps to depress the throttle
#
#t1 <- -40  # degF
#t2 <- 159
#crpulse1 <- 320
#crpulse2 <- 101

# #2
# leaner hot setting
#
# date: ?
# crank time: ?
# hot start: ?
#
t1 <- -40
t2 <- 159
crpulse1 <- 320
crpulse2 <- 90   # leaner setting


A <- matrix(c(t1, t2, 1, 1), nrow=2)
b <- matrix(c(crpulse1, crpulse2), nrow=2)

coef <- solve(A, b)

crfn <- function(x) {
	coef[1] * x + coef[2]
}

# plug these values in to the cranking pulse map
crpulse <- crfn(temp)

