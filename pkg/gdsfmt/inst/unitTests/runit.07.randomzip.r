#############################################################
#
# DESCRIPTION: test data dimension
#

library(RUnit)
library(gdsfmt)


#############################################################
#
# test functions
#

test.random_access_1 <- function()
{
	########  High Compression Rate (ratio: 6.3%)  ########

	####  cteate a GDS file  ####
	f <- createfn.gds("tmp.gds")

	set.seed(1000)
	v <- as.integer(rnorm(1000000) >= 0)

	n0 <- add.gdsn(f, "I0", val=v, compress="")
	readmode.gdsn(n0)

	n1 <- add.gdsn(f, "I1", val=v, compress="ZIP")
	readmode.gdsn(n1)

	n2 <- add.gdsn(f, "I2", val=v, compress="ZIP_RA:16K")
	readmode.gdsn(n2)

	closefn.gds(f)


	####  open the GDS file  ####
	f <- openfn.gds("tmp.gds")

	n0 <- index.gdsn(f, "I0")
	n1 <- index.gdsn(f, "I1")
	n2 <- index.gdsn(f, "I2")

	z0 <- read.gdsn(n0)
	z1 <- read.gdsn(n1)
	z2 <- read.gdsn(n2)
	checkEquals(z0, z1, "random access, all together [z0==z1]")
	checkEquals(z0, z2, "random access, all together [z0==z2]")

	set.seed(1000)
	for (k in 1:10)
	{
		idx <- sample.int(length(v), 100)

		v0 <- rep.int(0, length(idx))
		for (i in 1:length(idx))
			v0[i] <- read.gdsn(n0, start=idx[i], count=1)

		v1 <- rep.int(0, length(idx))
		for (i in 1:length(idx))
			v1[i] <- read.gdsn(n1, start=idx[i], count=1)

		v2 <- rep.int(0, length(idx))
		for (i in 1:length(idx))
			v2[i] <- read.gdsn(n2, start=idx[i], count=1)

		vv <- v[idx]
		checkEquals(v0, vv, sprintf("random access (%d), all together [v0]", k))
		checkEquals(v1, vv, sprintf("random access (%d), all together [v1]", k))
		checkEquals(v2, vv, sprintf("random access (%d), all together [v2]", k))
	}

	# close the file
	closefn.gds(f)

	# delete the GDS file
	unlink("tmp.gds")
}


test.random_access_2 <- function()
{
	########  Low Compression Rate (ratio: 85.5%)  ########

	####  cteate a GDS file  ####
	f <- createfn.gds("tmp.gds")

	set.seed(1000)
	v <- as.integer(rnorm(1000000)*2^20)

	n0 <- add.gdsn(f, "I0", val=v, compress="")
	readmode.gdsn(n0)

	n1 <- add.gdsn(f, "I1", val=v, compress="ZIP")
	readmode.gdsn(n1)

	n2 <- add.gdsn(f, "I2", val=v, compress="ZIP_RA:16K")
	readmode.gdsn(n2)

	closefn.gds(f)


	####  open the GDS file  ####
	f <- openfn.gds("tmp.gds")

	n0 <- index.gdsn(f, "I0")
	n1 <- index.gdsn(f, "I1")
	n2 <- index.gdsn(f, "I2")

	z0 <- read.gdsn(n0)
	z1 <- read.gdsn(n1)
	z2 <- read.gdsn(n2)
	checkEquals(z0, z1, "random access, all together [z0==z1]")
	checkEquals(z0, z2, "random access, all together [z0==z2]")

	set.seed(1000)
	for (k in 1:10)
	{
		idx <- sample.int(length(v), 100)

		v0 <- rep.int(0, length(idx))
		for (i in 1:length(idx))
			v0[i] <- read.gdsn(n0, start=idx[i], count=1)

		v1 <- rep.int(0, length(idx))
		for (i in 1:length(idx))
			v1[i] <- read.gdsn(n1, start=idx[i], count=1)

		v2 <- rep.int(0, length(idx))
		for (i in 1:length(idx))
			v2[i] <- read.gdsn(n2, start=idx[i], count=1)

		vv <- v[idx]
		checkEquals(v0, vv, sprintf("random access (%d), all together [v0]", k))
		checkEquals(v1, vv, sprintf("random access (%d), all together [v1]", k))
		checkEquals(v2, vv, sprintf("random access (%d), all together [v2]", k))
	}

	# close the file
	closefn.gds(f)

	# delete the GDS file
	unlink("tmp.gds")
}
