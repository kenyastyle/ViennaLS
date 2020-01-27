#include <iostream>

// #include <lsBooleanOperation.hpp>
#include <hrleSparseIterator.hpp>
#include <lsDomain.hpp>
#include <lsExpand.hpp>
#include <lsPrune.hpp>
// #include <lsFromSurfaceMesh.hpp>
#include <lsAdvect.hpp>
#include <lsMakeGeometry.hpp>
#include <lsToMesh.hpp>
#include <lsToSurfaceMesh.hpp>
#include <lsVTKWriter.hpp>

/**
  Minimal example of a plane surface being moved
  using lsAdvect
  \example AdvectionPlane.cpp
*/

// implement own velocity field
class velocityField : public lsVelocityField<double> {
public:
  double getScalarVelocity(const std::array<double, 3> & /*coordinate*/,
                           int /*material*/,
                           const std::array<double, 3> & /*normalVector*/) {
    return 1.;
  }

  std::array<double, 3>
  getVectorVelocity(const std::array<double, 3> & /*coordinate*/,
                    int /*material*/,
                    const std::array<double, 3> & /*normalVector*/) {
    return std::array<double, 3>({});
  }
};

int main() {
  constexpr int D = 2;
  omp_set_num_threads(1);

  double extent = 25;
  double gridDelta = 1;

  double bounds[2 * D] = {-extent, extent, -extent, extent};
  lsDomain<double, D>::BoundaryType boundaryCons[D];
  boundaryCons[0] = lsDomain<double, D>::BoundaryType::REFLECTIVE_BOUNDARY;
  boundaryCons[1] = lsDomain<double, D>::BoundaryType::INFINITE_BOUNDARY;
  lsDomain<double, D> plane(bounds, boundaryCons, gridDelta);

  double origin[D] = {0., 0.};
  double normal[D] = {2., 1.};

  lsMakeGeometry<double, D>(plane, lsPlane<double, D>(origin, normal)).apply();
  {
    lsMesh mesh;
    lsMesh explMesh;

    std::cout << "Extracting..." << std::endl;
    lsToSurfaceMesh<double, D>(plane, explMesh).apply();
    lsToMesh<double, D>(plane, mesh).apply();

    mesh.print();
    lsVTKWriter(explMesh, "before.vtk").apply();
    lsVTKWriter(mesh, "beforeLS.vtk").apply();
  }

  // fill vector with lsDomain pointers
  std::vector<lsDomain<double, D> *> lsDomains;
  lsDomains.push_back(&plane);

  velocityField velocities;

  std::cout << "number of Points: " << plane.getDomain().getNumberOfPoints()
            << std::endl;

  std::cout << "Advecting" << std::endl;
  lsAdvect<double, D> advectionKernel(lsDomains, velocities);
  advectionKernel.apply();
  double advectionTime = advectionKernel.getAdvectedTime();
  std::cout << "Time difference: " << advectionTime << std::endl;

  lsPrune<double, D>(plane).apply();
  lsExpand<double, D>(plane, 2).apply();

  std::cout << "Extracting..." << std::endl;
  lsMesh mesh;
  lsToSurfaceMesh<double, D>(plane, mesh).apply();

  // mesh.print();

  lsVTKWriter(mesh, "after.vtk").apply();

  return 0;
}