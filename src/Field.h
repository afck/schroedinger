#ifndef SCHROEDINGER_FIELD_H
#define SCHROEDINGER_FIELD_H

#include <cstring>
#include <cassert>

enum BoundaryCondition {
  WRAP,   ///< Wrap toroidally:     6 7|3 4 5 6 7|3 4
  MIRROR, ///< Mirror at the edges: 4 3|3 4 5 6 7|7 6
  ZERO,   ///< Set edges to zero:   0 0|3 4 5 6 7|0 0
};

/// A rectangular grid of cells of type T, for use as a cellular automaton.
/// The rectangle has a border of a configurable width, that frames the grid
/// itself. After writing values into the rectangle, the wrap() method populates
/// the border in such a way that calling get() on a point in the frame will
/// return an appropriate value, corresponding to the configured boundary
/// conditions.
///
/// The intended use is as a frame of a cellular automaton, with the size of a
/// neighborhood as the width of the border: After writing a frame, call wrap(),
/// so that the next frame can be computed without any special treatment of
/// coordinates that lie outside the main rectangle.
///
/// Example:
/// Field f = new Field<char>(5, 10, 2, WRAP); // 5*10 cells, border of 2 cells.
/// f.set(1, 9, 'x');
/// f.wrap();
/// f.get(6, -1) == 'x';
/// f.get(1, 9) == 'x';
/// f.get(6, 9) == 'x';
template <typename T> class Field {
public:
  const int width;            ///< Width of the main rectangle.
  const int height;           ///< Height of the main rectangle.
  const int border;           ///< Size of the border.
  BoundaryCondition boundary; ///< Boundary condition.
  /// Width of the frame: main rectangle plus border-sized border.
  const int framew = 2 * border + width;
  /// Height of the frame: main rectangle plus border-sized border.
  const int frameh = 2 * border + height;
  const int framesize = framew * frameh;
  Field(int width_, int height_, int border_, BoundaryCondition boundary_);
  ~Field();
  /// Get the value at point (x, y), where the distance from (x, y) to the main
  /// rectangle is not greater than border.
  T get(int x, int y) const;
  /// Get the value at point (x, y).
  T safeGet(int x, int y) const;
  /// Set the value at point (x, y), where (x, y) is in the main rectangle.
  void set(int x, int y, T value);
  /// Set the value at point (x, y).
  void safeSet(int x, int y, T value);
  /// Copy the values from the given field.
  void set(const Field<T> &other);
  /// Populate the border with the corresponding values, according to the
  /// boundary conditions.
  void fillBorder();
  /// Set everything to zero.
  void zero();
  /// The sum of all cells.
  T sum() const;
  /// Add the given value to every cell.
  void add(T t);

private:
  void wrap();
  void mirror();
  // The extended frame, including the border.
  T *const data = new T[framesize];
  // The first cell of the main rectangle.
  T *const cell0 = data + border * framew + border;
};

template <typename T>
Field<T>::Field(int width_, int height_, int border_,
                BoundaryCondition boundary_)
    : width(width_), height(height_), border(border_), boundary(boundary_) {
  assert(boundary == ZERO || width >= border);
  assert(boundary == ZERO || height >= border);
  zero();
}

template <typename T> Field<T>::~Field() { delete[] data; }

template <typename T> void Field<T>::fillBorder() {
  switch (boundary) {
  case WRAP:
    wrap();
    break;
  case MIRROR:
    mirror();
    break;
  case ZERO:
    break;
  }
}

template <typename T> void Field<T>::zero() {
  memset(data, 0, sizeof(T) * framesize);
}

template <typename T> void Field<T>::wrap() {
  size_t sideBorderSize = sizeof(T) * border;
  for (int y = border; y < height + border; y++) {
    // Copy the rightmost cells in the main rectangle to the left border.
    T *leftOutside = data + y * framew;
    memcpy(leftOutside, leftOutside + width, sideBorderSize);
    // Copy the leftmost cells in the main rectangle to the right border.
    T *leftInside = leftOutside + border;
    memcpy(leftInside + width, leftInside, sideBorderSize);
  }
  // Copy the bottom rows of the main rectangle to the top border.
  size_t topBorderSize = sizeof(T) * framew * border;
  T *topOutside = data;
  memcpy(topOutside, topOutside + framew * height, topBorderSize);
  // Copy the top rows of the main rectangle to the bottom border.
  T *topInside = data + framew * border;
  memcpy(topInside + framew * height, topInside, topBorderSize);
}

template <typename T> void Field<T>::mirror() {
  // Mirror the cells on the left and right.
  for (int y = border; y < height + border; y++) {
    T *left = data + y * framew + border;
    T *right = data + y * framew + border + width;
    for (int x = 0; x < border; x++) {
      *(left - x - 1) = *(left + x);
      *(right + x) = *(right - x - 1);
    }
  }
  // Mirror the top and bottom rows.
  size_t row_size = sizeof(T) * framew;
  T *top = data + border * framew;
  T *bottom = data + (height + border) * framew;
  for (int y = 0; y < border; y++) {
    memcpy(top - (1 + y) * framew, top + y * framew, row_size);
    memcpy(bottom + y * framew, bottom - (1 + y) * framew, row_size);
  }
}

template <typename T> T Field<T>::sum() const {
  T result = 0;
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      result += cell0[x + y * framew];
    }
  }
  return result;
}

template <typename T> void Field<T>::add(T t) {
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      cell0[x + y * framew] += t;
    }
  }
  wrap();
}

template <typename T> inline T Field<T>::get(int x, int y) const {
  return cell0[x + y * framew];
}

inline void mod(int &a, int m) {
  a %= m;
  if (a < 0) {
    a += m;
  }
}

inline void mirrorMod(int &a, int m) {
  mod(a, 2 * m);
  if (a >= m) {
    a = 2 * m - 1 - a;
  }
}

template <typename T> inline T Field<T>::safeGet(int x, int y) const {
  if (x < 0 || x >= width) {
    switch (boundary) {
    case WRAP:
      mod(x, width);
      break;
    case MIRROR:
      mirrorMod(x, width);
      break;
    case ZERO:
      return 0;
    }
  }
  if (y < 0 || y >= height) {
    switch (boundary) {
    case WRAP:
      mod(y, height);
      break;
    case MIRROR:
      mirrorMod(y, height);
      break;
    case ZERO:
      return 0;
    }
  }
  return cell0[x + y * framew];
}

template <typename T> inline void Field<T>::set(int x, int y, T value) {
  cell0[x + y * framew] = value;
}

template <typename T> void Field<T>::safeSet(int x, int y, T value) {
  if (x < 0 || x >= width) {
    switch (boundary) {
    case WRAP:
      mod(x, width);
      break;
    case MIRROR:
      mirrorMod(x, width);
      break;
    case ZERO:
      return;
    }
  }
  if (y < 0 || y >= height) {
    switch (boundary) {
    case WRAP:
      mod(y, height);
      break;
    case MIRROR:
      mirrorMod(y, height);
      break;
    case ZERO:
      return;
    }
  }
  cell0[x + y * framew] = value;
}

template <typename T> void Field<T>::set(const Field<T> &other) {
  assert(other.width == width);
  assert(other.height == height);
  assert(other.border == border);
  memcpy(data, other.data, framesize * sizeof(T));
}

#endif // SCHROEDINGER_FIELD_H
