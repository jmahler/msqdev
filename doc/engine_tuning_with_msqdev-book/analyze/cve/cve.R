
# This R script is to be used with the convergence
# ve data output by 'msq-ve_tuner'.

require(graphics)
require(ggplot2)

# choose one of the files
# one point, almost oscillates
#f1 <- "cve-12.4-01.dat"
# one point, oscillates around stable point
#f1 <- "cve-13.0-cd_1-02.dat"
# not enough data
#f1 <- "cve-13.0-cd_3-03.dat"
# one point, oscillates
#f1 <- "cve-13.5-cd_2-04.dat"
# some up, some down
#f1 <- "cve-13.5-cd_1-05.dat"
# 5 points, some up, some down
#f1 <- "cve-13.5-cd_2-06.dat"
# 4 points, some adjusting up, others down
#f1 <- "cve-13.0-cd_3-08.dat"
# one point, 2_3, little data
#f1 <- "cve-12.5-cd_3-09.dat"
# lots of points, hard to discern
f1 <- "cve-12.5-cd_3-10.dat"
#
# The points, such as 2_3, correspond
# to the map in the following way.
# They start from a zero offset.
# The first digit is the x (rpm) direction.
# The second digit is the y (fuelload) direction.
# 0_0 is at absolute bottom left
# 2_3 would be, starting from 0_0 at the 3rd
#  position to the right and the 4th position up.
#

df1 <- read.csv(file=f1, head=TRUE, sep=",")

p1 <- ggplot(df1, aes(x=time, y=veCurr1, group=group, color=group))

pX <- p1 +
	geom_point() +
	geom_line() +
	xlab("time (s)") +
	opts(title="veCurr1 adjustments over time\nmsq-ve_tuner -cd 3 -a 12.5")

pdf("cve-many_points-cd3.pdf", width=6, height=6)
print(pX);
dev.off()
