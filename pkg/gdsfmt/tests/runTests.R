# ===========================================================
#     _/_/_/   _/_/_/  _/_/_/_/    _/_/_/_/  _/_/_/   _/_/_/
#      _/    _/       _/             _/    _/    _/   _/   _/
#     _/    _/       _/_/_/_/       _/    _/    _/   _/_/_/
#    _/    _/       _/             _/    _/    _/   _/
# _/_/_/   _/_/_/  _/_/_/_/_/     _/     _/_/_/   _/_/
# ===========================================================
#
# runTests.R: the R interface of CoreArray library
#
# Copyright (C) 2013 - 2014		Xiuwen Zheng
#


# load R packages
if (require(RUnit))
{
	library(gdsfmt)

	# define a test suite
	myTestSuite <- defineTestSuite("gdsfmt examples",
		system.file("unitTests", package = "gdsfmt"))

	# run the test suite
	testResult <- runTestSuite(myTestSuite)

	# print detailed text protocol to standard out:
	printTextProtocol(testResult)
}

# quit
q("no")
