#ifndef SCHROEDINGER_WAVE_H
#define SCHROEDINGER_WAVE_H

#include <complex>
#include <vector>

#include "Field.h"

typedef std::complex<double> dcomp;

/// Planck constant in Js.
const double PLANCK_CONST = 6.62606957e-34;
/// Gravitational constant in Nm²/kg².
const double GRAVITATIONAL_CONST = 6.673e-11;

/// A wave function of a single, non-relativistic particle, represented as a
/// cellular automaton with complex-valued cells.
class Wave {
public:
  Wave(int width, int height);
  /// Compute the state of the wave in the next time step.
  void evolve();
  /// Add c times a bump function to the wave.
  void addBump(int x, int y, dcomp c, int size);
  /// Add c times a bump function to the static potential.
  void addPotentialBump(int x, int y, double c, int size);
  /// Normalize the wave function, so that it has norm 1.
  void normalize();
  /// Draw the wave function and potential using the given color mapping.
  void draw(std::uint32_t *pixels,
            std::uint32_t toColor(dcomp c, double p)) const;

private:
  const int width_;
  const int height_;
  const BoundaryCondition boundary_ = WRAP;
  const double area_ = 1.0; // The total area in m².
  const double sarea_ = sqrt(area_);
  const double dr_ = sqrt(area_ / (width_ * height_));
  const double qdrdr_ = 1.0 / (dr_ * dr_);
  const double maxAbs_ = 6.0 / area_;
  const double m_ = 1000 * 9.10938291e-31; // The particle's mass in kg.
  const double dt_ = 10; // The time resolution in s.
  std::vector<Field<dcomp>> tmpPsi_;
  Field<dcomp> psi_ = Field<dcomp>(width_, height_, 1, boundary_);
  Field<double> potential_ = Field<double>(width_, height_, 1, boundary_);
  Field<double> dynPotential_ = Field<double>(width_, height_, 1, boundary_);
  Field<double> tmpPotential_ = Field<double>(width_, height_, 1, boundary_);
  Field<double> tmpReal_ = Field<double>(width_, height_, 1, boundary_);
  dcomp calcDPsiXY(dcomp laplaceXY, dcomp psiXY, dcomp VXY) const;
  void calcK(Field<dcomp> &newk, const Field<dcomp> &oldk, double factor);
  void calcLaplaceV(Field<double> &laplaceV) const;
  void calcV(const Field<double> &laplaceV);
  dcomp laplace(const Field<dcomp> &f, int x, int y) const;
};

#endif // SCHROEDINGER_WAVE_H
