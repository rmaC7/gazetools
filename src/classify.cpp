#include "gazetools.h"

//' Classify Raw Gaze Data {C++}
//'
//' Velocity based classification of raw gaze samples to discrete events such as saccades and fixations.
//' With this classifier, fixations are what ever samples are left after identifying all other events.
//' In other words, there is no concept of minium fixation duration.
//'
//' @param v a vector of the instantaneous velocities for a set raw gaze samples
//' @param e a vector indicating blinks or otherwise bad data in the velocity vector
//' @param samplerate the number of samples taken in one second of time
//' @param vt saccade onset velocity threshold
//' @param sigma when greater than 0, the saccade onset velocity threshold is iteratively adjusted such that
//'        the saccade onset threshold is \emph{sigma} standard deviations higher than the mean of all velocity samples
//'        lower than the saccade onset threshold, when glissade detection is enabled \emph{sigma}/2 is used as the
//'        saccade offset velocity threshold
//' @param minblink the minimum duration in seconds required for noise to be considered a blink
//' @param minsac the minimum saccade duration in seconds
//' @param glswin the duration (in seconds) of the window post-saccade to look for glissades in, setting to 0 disables glissade detection
//' @param alpha the weight (from 0 to 1) of the saccade onset threshold component of the saccade offset threshold,
//'        \emph{1-alpha} is used as the weight for the noise threshold component the saccade offset threshold
//'
//' @export
// [[Rcpp::export]]
Rcpp::IntegerVector classify(std::vector<double> v, std::vector<bool> e, int samplerate, double vt=100, double sigma=6, double minblink=.08, double minsac=.02, double glswin=.04, double alpha=.7) {

  // ******************************************************************************
  // TODO:
  //  * Catch saccade->blink & fixation->blink transitions and vise versa
  //
  // ******************************************************************************

  double ts = 1.0 / samplerate;
  double st = vt;
  double beta = 1.0 - alpha;
  int i=0, j=0, n=0, sb=0, se=0;

  int gls = 0;
  if (glswin>0) gls = std::ceil(glswin/ts);
  int bln = 0;
  if (minblink>0) bln = std::ceil(minblink/ts);

  if (sigma>0) {
    vt = sigthresh(v, e, vt, sigma); // Iteratively find saccade peak threshold
    st = sigthresh(v, e, vt, sigma/2); // Iteratively find saccade onset threshold
  }

  n = v.size();
  Rcpp::IntegerVector out(n);
  for(i = 0; i < n; ++i) {
    if (v[i]>vt)
      out[i] = SACCADE;
    else
      out[i] = FIXATION;
  }
  int bb=-1,be=-1;
  for(i = 0; i < n;) {
    j = i;
    if (e[i]==true) {
      out[i] = NOISE;
      if ((i-1>=0) && out[i-1]!=NOISE) {
        while ((j-1>=0) && (v[j] > st || v[j] >= v[j-1])) {
          out[j] = NOISE;
          --j;
        }
        out[j] = NOISE;
        bb=j;
      } else if ((i+1<n) && out[i+1]!=NOISE) {
        while ((j+1<n) && (v[j] > st || v[j] >= v[j+1])) {
          out[j] = NOISE;
          ++j;
        }
        out[j] = NOISE;
        be=j;
        if ((be-bb)>bln) {
          for (j=bb;j<=be;++j)
            out[j] = BLINK;
        }
      }
      i=j;
    } else if ((i-1>=0) && out[i]==SACCADE && out[i-1]!=SACCADE) { // Find saccade onset
      while ((j-1>=0) && (v[j] > v[j-1] && v[j]-v[j-1]>0)) { // Saccade onset is first local minima under saccade onset threshold
        out[j] = SACCADE;
        --j;
      }
      out[j] = SACCADE;
      sb = j; // Save saccade begin index
    } else if ((i+1<n) && out[i]==SACCADE && out[i+1]!=SACCADE) { // Find saccade offset
      double tmpsum=0, tmpmean=0, tmpsd=0;
      for (int k=sb; k>=std::max(sb-gls,0); --k) {
        tmpsum += v[k];
      }
      tmpmean = tmpsum/gls;
      tmpsum = 0;
      for (int k=sb; k>=std::max(sb-gls,0); --k) {
        tmpsum += (v[k]-tmpmean)*(v[k]-tmpmean);
      }
      tmpsd = sqrt(tmpsum/(double)gls);
      double nt = alpha*st + beta*(tmpmean + (sigma/2 * tmpsd));
      bool do_glissade = true;
      while ((j+1<n) && (v[j] > v[j+1] || v[j] > nt))  {// Saccade offset is first local minima under saccade offset threshold
        if (e[j+1]) {// we hit a blink, rewind and break
          while (j>0 && v[j]>v[j-1]) {
            out[j] = FIXATION;
            --j;
          }
          do_glissade = false;
          break;
        }
        out[j] = SACCADE;
        ++j;
      }
      out[j] = SACCADE;
      se = j; // Save saccade end index
      if ((se-sb+1) < minsac/ts) { // If the saccade is too short, wipe it out
        for (j=sb; j<=se; ++j)
          out[j] = FIXATION;
      }
      i = se; // Move index to end of saccade
      //do_glissade = false;
      if (do_glissade && out[i]!=FIXATION && gls>0 && (i+1)<n) {// Now look for glissades
        int gmax = std::min(i+gls,n-1);
        int slow = 0;
        int fast = 0;
        int glissade_type = 0;
        for (j=(i+1);j<=gmax;j++) {
          if (v[j-1]>vt && v[j]<=vt) ++fast;
          if (v[j-1]>nt && v[j]<=nt) ++slow;
        }
        if (fast>0) {
          glissade_type = GLISSADE_FAST;
        } else if (slow>0) {
          glissade_type = GLISSADE_SLOW;
        }
        if (glissade_type == GLISSADE_FAST || glissade_type == GLISSADE_SLOW) {
          j=i;
          while (j<=gmax && v[j]<=v[j+1]) {
            out[j] = glissade_type;
            ++j;
          }
          while (j<n && (v[j]>nt || (v[j]>v[j+1]))) {
            out[j] = glissade_type;
            ++j;
          }
          i=j;
        }
      }
    }
    ++i;
  }

  Rcpp::IntegerVector levels(6);
  levels[0] = NOISE;
  levels[1] = BLINK;
  levels[2] = FIXATION;
  levels[3] = SACCADE;
  levels[4] = GLISSADE_FAST;
  levels[5] = GLISSADE_SLOW;
  Rcpp::CharacterVector labels(6);
  labels[0] = "Noise";
  labels[1] = "Blink";
  labels[2] = "Fixation";
  labels[3] = "Saccade";
  labels[4] = "Glissade-fast";
  labels[5] = "Glissade-slow";
  Rcpp::IntegerVector c = match(out, levels);
  c.attr("levels") = labels;
  c.attr("class") = "factor";
  c.attr("saccade-peak-threshold") = vt;
  c.attr("saccade-onset-threshold") = st;
  c.attr("sigma") = sigma;
  c.attr("minblink") = minblink;
  c.attr("minsac") = minsac;
  c.attr("glswin") = glswin;
  c.attr("alpha") = alpha;

  return c;
}
