\name{ls.gdsn}
\alias{ls.gdsn}
\title{Return the names of child nodes}
\description{
	Get a list of names for its child nodes.
}

\usage{
ls.gdsn(node)
}
\arguments{
	\item{node}{an object of class \code{\link{gdsn.class}}, a GDS node}
}
\value{
	A vector of characters.
}

\references{\url{http://github.com/zhengxwen/gdsfmt}}
\author{Xiuwen Zheng}
\seealso{
	\code{\link{cnt.gdsn}}, \code{\link{objdesp.gdsn}},
	\code{\link{ls.gdsn}}, \code{\link{index.gdsn}}
}

\examples{
# cteate a GDS file
f <- createfn.gds("test.gds")

# add a list to "test.gds"
node <- add.gdsn(f, name="list", val=list(x=c(1,2), y=c("T","B","C"), z=TRUE))

ls.gdsn(node)

# close the GDS file
closefn.gds(f)


# delete the temporary file
unlink("test.gds", force=TRUE)
}

\keyword{GDS}
\keyword{utilities}
