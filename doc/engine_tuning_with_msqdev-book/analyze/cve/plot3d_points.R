

# manually specify the points to use
#
# This can be done by looking at the points
# from the cve.R plot and finding them in the table.

rpm_range <- c(501, 801, 1101, 1401, 2001, 2601, 3101, 3700, 4300, 4900, 5400, 6000, 6500, 7000, 7200, 7500)
fuelload_range <- c(30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 98, 100)

df1 <- data.frame (
	x        = c(2,    2,    5,    6,    6),
	y        = c(3,    4,    3,    4,    6),
	rpm      = c(1101, 1101, 2601, 3101, 3101),
	fuelload = c(45,   50,   45,   50,   60),
	ve       = c(52,   42,   69,   64,   63)
);

if (sum(search() %in% c("df1")) > 0) {
	detach(df1);
}
attach(df1);

### least squares fit ###

fit1 <- lm(formula = ve ~ rpm + fuelload)

# Get the coefficients
# ve = a*rpm + fuelload*b + c
c <- fit1$coefficients[1] # intercept
a <- fit1$coefficients[2] # rpm
b <- fit1$coefficients[3] # fuelload

# calculate all the values for the entire map
calc_rpm <- c()
calc_fuelload <- c()
calc_ve <- c()

n <- 1
for (i1 in 1:length(rpm_range)) {
	for (j1 in 1:length(fuelload_range)) {
		calc_rpm[n] <- rpm_range[i1];
		calc_fuelload[n] <- fuelload_range[j1];
		calc_ve[n] = a*rpm_range[i1] + b*fuelload_range[j1] + c
		n <- n + 1;
	}
}

calcs1 <- data.frame(
	rpm = calc_rpm,
	fuelload = calc_fuelload,
	ve = calc_ve
)

### PLOT ###

require(rgl)

# The arguments to plot3d may have to be re-arranged
# to get the axis orientation correct.
with(calcs1,
	plot3d(rpm, fuelload, ve))

