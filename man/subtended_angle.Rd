% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/RcppExports.R
\name{subtended_angle}
\alias{subtended_angle}
\title{Subtended Angle {C++}}
\usage{
subtended_angle(x1, y1, x2, y2, rx, ry, sw, sh, ez, ex = as.numeric(c(0)),
  ey = as.numeric(c(0)))
}
\arguments{
\item{x1}{the horizontal coordinate of the first point on a screen (pixels)}

\item{y1}{the vertical coordinate of the first point on a screen (pixels)}

\item{x2}{the horizontal coordinate of the second point on a screen (pixels)}

\item{y2}{the vertical coordinate of the second point on a screen (pixels)}

\item{rx}{the horizontal resolution of the screen (pixels)}

\item{ry}{the vertical resolution of the screen (pixels)}

\item{sw}{the physical screen width (mm)}

\item{sh}{the physical screen height (mm)}

\item{ez}{the perpendicular distance from the viewer to the screen (mm)}

\item{ex}{the horizontal offset of the viewer from screen center (mm)}

\item{ey}{the vertical offset of the viewer from screen center (mm)}
}
\value{
degrees of visual angle
}
\description{
Takes two screen coordinates and returns the angle (in degrees)
subtended by those two points.
}
\examples{
subtended_angle(835, 525, 845, 525, 1680, 1050, 473.76, 296.1, 750)

}

