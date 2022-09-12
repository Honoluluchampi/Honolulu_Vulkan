// hnll
#include <physics/bounding_volume.hpp>

namespace hnll::physics {

bounding_volume bounding_volume::create_bounding_sphere(bv_ctor_type type, std::vector<Eigen::Vector3d> &vertices)
{
  switch (type) {
    case bv_ctor_type::RITTER:
      return ritter_ctor(vertices);
    default:
      std::runtime_error("invalid bounding-sphere-ctor type");
  }
}

std::pair<int,int> most_separated_points_on_aabb(const std::vector<Eigen::Vector3d> &vertices)
{
  // represents min/max point's index of each axis
  int minx = 0, maxx = 0, miny = 0, maxy = 0, minz = 0, maxz = 0, min = 0, max = 0;
  int n_vertices = vertices.size();
  for (int i = 1; i < n_vertices; i ++) {
    if (vertices[i].x() < vertices[minx].x()) minx = i;
    if (vertices[i].x() > vertices[maxx].x()) maxx = i;
    if (vertices[i].y() < vertices[miny].y()) miny = i;
    if (vertices[i].y() > vertices[maxy].y()) maxy = i;
    if (vertices[i].z() < vertices[minz].z()) minz = i;
    if (vertices[i].z() > vertices[maxz].z()) maxz = i;
  }
  auto diffx = vertices[maxx] - vertices[minx],
       diffy = vertices[maxy] - vertices[miny],
       diffz = vertices[maxz] - vertices[minz];
  // compute the squared distances for each pairs
  float dist2x = diffx.dot(diffx),
        dist2y = diffy.dot(diffy),
        dist2z = diffz.dot(diffz);
  // pick the most distant pair
  min = minx, max = maxx;
  if (dist2y > dist2x && dist2y > dist2z)
    min = miny, max = maxy;
  if (dist2z > dist2x && dist2z > dist2y)
    min = minz, max = maxz;
  return {min, max};
}

bounding_volume sphere_from_distant_points(const std::vector<Eigen::Vector3d> &vertices)
{
  auto separated_idx = most_separated_points_on_aabb(vertices);
  auto center_point = (vertices[separated_idx.first] + vertices[separated_idx.second]) * 0.5f;
  auto radius = (vertices[separated_idx.first] - center_point).dot(vertices[separated_idx.first] - center_point);
  radius = std::sqrt(radius);
  return bounding_volume{center_point, radius};
}

void extend_sphere_to_point(bounding_volume& sphere, const Eigen::Vector3d& point)
{
  auto diff = point - sphere.get_center_point();
  auto dist2 = diff.dot(diff);
  if (dist2 > sphere.get_sphere_radius() * sphere.get_sphere_radius()) {
    auto dist = std::sqrt(dist2);
    auto new_radius = (sphere.get_sphere_radius() + dist) * 0.5f;
    auto k = (new_radius - sphere.get_sphere_radius()) / dist;
    sphere.set_sphere_radius(new_radius);
    sphere.set_center_point(sphere.get_center_point() + diff * k);
  }
}

bounding_volume bounding_volume::ritter_ctor(const std::vector<Eigen::Vector3d> &vertices)
{
  auto sphere = sphere_from_distant_points(vertices);
  for (const auto& vertex : vertices)
    extend_sphere_to_point(sphere, vertex);
  return sphere;
}

bool bounding_volume::intersection_test(const bounding_volume &other)
{
  // TODO : implement test for each pair
  Eigen::Vector3d difference = this->center_point_ - other.get_center_point();
  double distance2 = difference.dot(difference);
  float radius_sum = this->get_sphere_radius() + other.get_sphere_radius();
  return distance2 <= radius_sum * radius_sum;
}

} // namespace hnll::physics