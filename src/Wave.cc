#include <math.h>
#include <vector>

#include "Wave.h"

using std::vector;

const dcomp I = dcomp(0.0, 1.0);

Wave::Wave(int width, int height) : width_(width), height_(height) {
  const int tmpPageNum = 4;
  // Reserve, as reallocation calls the Field destructor.
  // TODO: Proper move semantics for Field.
  tmpPsi_.reserve(tmpPageNum);
  for (int i = 0; i < tmpPageNum; i++) {
    tmpPsi_.emplace_back(width_, height_, 1, boundary_);
  }
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      psi_.set(x, y, std::polar(1.0, 2.0 * M_PI * x / width_));
    }
  }
  psi_.fillBorder();
  potential_.fillBorder();
}

inline dcomp Wave::laplace(const Field<dcomp> &f, int x, int y) const {
  const static double qsqrt2 = 1.0 / sqrt(2.0);
  dcomp w4 = 4.0 * f.get(x, y);
  dcomp s = f.get(x + 1, y) + f.get(x - 1, y) + f.get(x, y - 1) +
            f.get(x, y + 1) - w4;
  dcomp sdiag = f.get(x + 1, y + 1) + f.get(x + 1, y - 1) +
                f.get(x - 1, y + 1) + f.get(x - 1, y - 1) - w4;
  return 0.5 * (s + sdiag * qsqrt2) * qdrdr_;
}

inline dcomp Wave::calcDPsiXY(dcomp laplaceXY, dcomp psiXY, dcomp VXY) const {
  // The factor of the Laplacian.
  static const double hm = PLANCK_CONST / (2.0 * M_PI * m_);
  // The factor of the potential.
  static const double qh = 2.0 * M_PI / PLANCK_CONST;
  return -I * (qh * VXY * psiXY - hm * laplaceXY);
}

void Wave::calcK(Field<dcomp> &newk, const Field<dcomp> &oldk, double factor) {
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      dcomp laplaceXY = laplace(psi_, x, y) + factor * laplace(oldk, x, y);
      dcomp psiXY = psi_.get(x, y) + factor * oldk.get(x, y);
      double VXY = potential_.get(x, y) + dynPotential_.get(x, y);
      newk.set(x, y, calcDPsiXY(laplaceXY, psiXY, VXY));
    }
  }
  newk.fillBorder();
}

// Compute the Laplacian of the gravitational potential.
void Wave::calcLaplaceV(Field<double> &laplaceV) const {
  const double factor = 4 * M_PI * GRAVITATIONAL_CONST * m_;
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      laplaceV.set(x, y, factor * abs(psi_.get(x, y)));
    }
  }
  laplaceV.fillBorder();
}

// Solve the Poisson equation to compute the potential given its Laplacian.
void Wave::calcV(const Field<double> &laplaceV) {
  double sqrerr;
  double norm;
  do {
    sqrerr = 0;
    norm = 0;
    for (int x = 0; x < width_; x++) {
      for (int y = 0; y < height_; y++) {
        double newV =
            0.25 * (dynPotential_.get(x - 1, y) + dynPotential_.get(x + 1, y) +
                    dynPotential_.get(x, y - 1) + dynPotential_.get(x, y + 1) -
                    laplaceV.get(x, y) * dr_ * dr_);
        tmpPotential_.set(x, y, newV);
        norm += newV * newV;
        double oldV = dynPotential_.get(x, y);
        sqrerr += (oldV - newV) * (oldV - newV);
      }
    }
    tmpPotential_.fillBorder();
    dynPotential_.set(tmpPotential_);
  } while (sqrerr > norm * 0.0001);
  dynPotential_.add(-dynPotential_.sum() / (width_ * height_));
}

void Wave::evolve() {
  // Update the dynamic potential, depending on the current wave.
  calcLaplaceV(tmpReal_);
  calcV(tmpReal_);
  // Compute the next time step using the RK4 method. See:
  // https://en.wikipedia.org/wiki/Runge-Kutta_methods
  calcK(tmpPsi_[0], psi_, 0.0);
  calcK(tmpPsi_[1], tmpPsi_[0], 0.5 * dt_);
  calcK(tmpPsi_[2], tmpPsi_[1], 0.5 * dt_);
  calcK(tmpPsi_[3], tmpPsi_[2], dt_);
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      dcomp tp0 = tmpPsi_[0].get(x, y);
      dcomp tp1 = tmpPsi_[1].get(x, y);
      dcomp tp2 = tmpPsi_[2].get(x, y);
      dcomp tp3 = tmpPsi_[3].get(x, y);
      dcomp avgTp = (tp0 + tp1 * 2.0 + tp2 * 2.0 + tp3) / 6.0;
      psi_.set(x, y, psi_.get(x, y) + dt_ * avgTp);
    }
  }
  psi_.fillBorder();
}

void Wave::normalize() {
  double sintegral = 0;
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      dcomp c = psi_.get(x, y);
      double nc = norm(c);
      if (nc > maxAbs_ * maxAbs_) {
        c *= maxAbs_ / sqrt(nc);
        nc = maxAbs_ * maxAbs_;
        psi_.set(x, y, c);
      }
      sintegral += nc;
    }
  }
  const double a = sqrt(sintegral) * dr_;
  if (a > 0) {
    const double qa = 1.0 / a;
    for (int x = 0; x < width_; x++) {
      for (int y = 0; y < height_; y++) {
        dcomp c = psi_.get(x, y);
        psi_.set(x, y, c * qa);
      }
    }
  }
  psi_.fillBorder();
}

void Wave::addBump(int x, int y, dcomp c, int size) {
  c /= sarea_;
  for (int dx = -size; dx <= size; dx++) {
    for (int dy = -size; dy <= size; dy++) {
      double rr = (dx * dx + dy * dy) / static_cast<double>(size * size);
      if (rr < 1.0) {
        double mx = (x + dx + width_) % width_;
        double my = (y + dy + height_) % height_;
        dcomp oldc = psi_.get(mx, my);
        psi_.set(mx, my, oldc + c * (1.0 - sqrt(rr)));
      }
    }
  }
}

// Multiplier for the potential field, in 1 / J.
static const double POTENTIAL_UNIT = 1e35;

void Wave::addPotentialBump(int x, int y, double c, int size) {
  c /= POTENTIAL_UNIT * area_ * dt_;
  for (int dx = -size; dx <= size; dx++) {
    for (int dy = -size; dy <= size; dy++) {
      double rr = (dx * dx + dy * dy) / (static_cast<double>(size) * size);
      if (rr < 1.0) {
        double mx = (x + dx + width_) % width_;
        double my = (y + dy + height_) % height_;
        double oldc = potential_.get(mx, my);
        potential_.set(mx, my, std::max(oldc, c * (1.0 - sqrt(rr))));
      }
    }
  }
}

void Wave::draw(uint32_t *pixels, uint32_t toColor(dcomp c, double p)) const {
  for (int x = 0; x < width_; x++) {
    for (int y = 0; y < height_; y++) {
      const dcomp psiXY = psi_.get(x, y) * sarea_;
      const double VXY = POTENTIAL_UNIT * potential_.get(x, y) * sarea_ * dt_;
      pixels[x + width_ * y] = toColor(psiXY, VXY);
    }
  }
}
