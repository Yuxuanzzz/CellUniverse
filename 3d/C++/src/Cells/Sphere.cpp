#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

class SphereConfig : public CellConfig {
public:
    PerturbParams x;
    PerturbParams y;
    PerturbParams z;
    PerturbParams radius;
    double minRadius;
    double maxRadius;
};

class SphereParams {
public:
    double x;
    double y;
    double z;
    double radius;

    SphereParams(double x_val, double y_val, double z_val, double radius_val)
        : x(x_val), y(y_val), z(z_val), radius(radius_val) {}
};


class Sphere : public Cell {
private:
    std::string _name;
    cv::Point3f _position;
    double _radius;
    double _rotation;
    bool dormant;
    SphereParams paramClass;
    SphereConfig cellConfig;

public:
    Sphere(const SphereParams& init_props)
        : _name(init_props.name), _position({ init_props.x, init_props.y, init_props.z }),
        _radius(init_props.radius), _rotation(0), dormant(false) {}

    double get_radius_at(double z) const {
        if (std::abs(_position.values[2] - z) > _radius) {
            return 0;
        }
        return std::sqrt((_radius * _radius) - ((_position.values[2] - z) * (_position.values[2] - z)));
    }

    void draw(cv::Mat& image, SimulationConfig simulationConfig, cv::Mat* cellMap = nullptr, float z = 0) {
        if (dormant) {
            return;
        }

        float currentRadius = getRadiusAt(z);
        if (currentRadius <= 0) {
            return;
        }

        cv::Scalar backgroundColor(simulationConfig.backgroundColor[0], simulationConfig.backgroundColor[1], simulationConfig.backgroundColor[2]);
        cv::Scalar cellColor(simulationConfig.cellColor[0], simulationConfig.cellColor[1], simulationConfig.cellColor[2]);

        cv::Point center(static_cast<int>(_position.x), static_cast<int>(_position.y));
        cv::Size axes(static_cast<int>(currentRadius), static_cast<int>(currentRadius));
        cv::ellipse(image, center, axes, 0, 0, 360, cellColor, -1);
    }

    void drawOutline(cv::Mat& image, cv::Scalar color, float z = 0) {
        if (dormant) {
            return;
        }

        float currentRadius = getRadiusAt(z);
        if (currentRadius <= 0) {
            return;
        }

        cv::Point center(static_cast<int>(_position.x), static_cast<int>(_position.y));
        cv::Size axes(static_cast<int>(currentRadius), static_cast<int>(currentRadius));
        cv::ellipse(image, center, axes, 0, 0, 360, color, 1);
    }

    Sphere get_perturbed_cell() {
        return Sphere(SphereParams{
            .name = _name,
            .x = _position.x + Sphere.cellConfig.x.get_perturb_offset(),
            .y = _position.y + Sphere.cellConfig.y.get_perturb_offset(),
            .z = _position.z + Sphere.cellConfig.z.get_perturb_offset(),
            .radius = _radius + Sphere.cellConfig.radius.get_perturb_offset()
            });
    }

    Sphere get_parameterized_cell(DefaultDict<float> params) {
        float xOffset = params["x"];
        float yOffset = params["y"];
        float zOffset = params["z"];
        float radiusOffset = params["radius"];

        if (params.empty()) {
            xOffset = Sphere.cellConfig.x.get_perturb_offset();
            yOffset = Sphere.cellConfig.y.get_perturb_offset();
            zOffset = Sphere.cellConfig.z.get_perturb_offset();
            radiusOffset = Sphere.cellConfig.radius.get_perturb_offset();
        }

        float newRadius = std::min(std::max(Sphere.cellConfig.minRadius, _radius + radiusOffset), Sphere.cellConfig.maxRadius);

        return Sphere(SphereParams{
            .name = _name,
            .x = _position.x + xOffset,
            .y = _position.y + yOffset,
            .z = _position.z + zOffset,
            .radius = newRadius
            });
    }

    std::tuple<Sphere, Sphere, bool> get_split_cells() const {
        double theta = ((double)rand() / RAND_MAX) * 2 * M_PI;
        double phi = ((double)rand() / RAND_MAX) * M_PI;

        cv::Point3f split_axis(
        sin(phi) * cos(theta),
        sin(phi) * sin(theta),
        cos(phi));

        cv::Point3f offset = split_axis * (_radius / 2.0);
        cv::Point3f new_position1 = _position + offset;
        cv::Point3f new_position2 = _position - offset;

        double halfRadius = _radius / 2.0;

        Sphere cell1(SphereParams(_name + "0", new_position1.values[0], new_position1.values[1], new_position1.values[2], halfRadius));
        Sphere cell2(SphereParams(_name + "1", new_position2.values[0], new_position2.values[1], new_position2.values[2], halfRadius));

        bool constraints = cell1.check_constraints() && cell2.check_constraints();

        return std::make_tuple(cell1, cell2, constraints);
    }

    bool check_constraints() const {
        return (SphereConfig::minRadius <= _radius) && (_radius <= SphereConfig::maxRadius);
    }

    float get_radius_at(float z) {
        if (std::abs(_position.z - z) > _radius) {
            return 0;
        }
        return std::sqrt((_radius * _radius) - ((_position.z - z) * (_position.z - z)));
    }

    SphereParams get_cell_params() const {
        return SphereParams(_name, _position.values[0], _position.values[1], _position.values[2], _radius);
    }

    std::pair<std::vector<float>, std::vector<float>> calculate_corners() {
        std::vector<float> min_corner = { _position.x - _radius, _position.y - _radius, _position.z - _radius };
        std::vector<float> max_corner = { _position.x + _radius, _position.y + _radius, _position.z + _radius };
        return std::make_pair(min_corner, max_corner);
    }

    std::pair<std::vector<float>, std::vector<float>> calculate_minimum_box(Sphere& perturbed_cell) {
        auto [cell1_min_corner, cell1_max_corner] = calculate_corners();
        auto [cell2_min_corner, cell2_max_corner] = perturbed_cell.calculate_corners();

        std::vector<float> min_corner, max_corner;
        for (int i = 0; i < 3; ++i) {
            min_corner.push_back(std::min(cell1_min_corner[i], cell2_min_corner[i]));
            max_corner.push_back(std::max(cell1_max_corner[i], cell2_max_corner[i]));
        }
        return std::make_pair(min_corner, max_corner);
    }

    static bool check_if_cells_overlap(const std::vector<Sphere>& spheres) {
        std::vector<std::vector<float>> positions;
        std::vector<float> radii;

        for (const auto& cell : spheres) {
            positions.push_back({ cell._position.x, cell._position.y, cell._position.z });
            radii.push_back(cell._radius * 0.95);
        }

        std::vector<std::vector<float>> distances;
        for (const auto& position1 : positions) {
            std::vector<float> distance_row;
            for (const auto& position2 : positions) {
                float distance = 0.0f;
                for (int i = 0; i < 3; ++i) {
                    distance += pow(position1[i] - position2[i], 2);
                }
                distance = sqrt(distance);
                distance_row.push_back(distance);
            }
            distances.push_back(distance_row);
        }

        std::vector<std::vector<float>> radii_sums;
        for (const auto& radius1 : radii) {
            std::vector<float> radii_row;
            for (const auto& radius2 : radii) {
                radii_row.push_back(radius1 + radius2);
            }
            radii_sums.push_back(radii_row);
        }

        bool overlap = false;
        for (std::size_t i = 0; i < spheres.size(); ++i) {
            for (std::size_t j = 0; j < spheres.size(); ++j) {
                if (i != j && distances[i][j] < radii_sums[i][j]) {
                    overlap = true;
                    break;
                }
            }
            if (overlap) {
                break;
            }
        }

        return overlap;
    }
};
