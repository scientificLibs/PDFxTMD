#pragma once
#include <gsl/gsl_interp2d.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

class GSLInterp2D
{
  public:
    enum InterpolationType
    {
        Bilinear,
        Bicubic
    };

    // Delete copy constructor and copy assignment operator
    GSLInterp2D(const GSLInterp2D &) = delete;
    GSLInterp2D &operator=(const GSLInterp2D &) = delete;
    // Constructor: Initializes the 2D interpolator with grid
    // and data.
    GSLInterp2D(const std::vector<double> &x, const std::vector<double> &y,
                const std::vector<double> &z, InterpolationType interpType)
        : x_size(x.size()), y_size(y.size()), x_vals(x), y_vals(y), z_vals(z)
    {
        if (x.size() * y.size() != z.size())
        {
            std::cout << x.size() << "---" << y.size() << "----" << z.size() << std::endl;
            throw std::invalid_argument("Mismatch in grid and data sizes");
        }

        // Allocate GSL objects with custom deleters
        accel_x.reset(gsl_interp_accel_alloc());
        accel_y.reset(gsl_interp_accel_alloc());
        switch (interpType)
        {
        case InterpolationType::Bilinear:
            interp.reset(gsl_interp2d_alloc(gsl_interp2d_bilinear, x_size, y_size));
            break;
        case InterpolationType::Bicubic:
            interp.reset(gsl_interp2d_alloc(gsl_interp2d_bicubic, x_size, y_size));
            break;
        default:
            std::runtime_error("[GSLInterp2D] undefined interpolation type for "
                               "gsl is selected!");
            break;
        }

        // Initialize the interpolator with grid and data points
        gsl_interp2d_init(interp.get(), x_vals.data(), y_vals.data(), z.data(), x_size, y_size);
    }

    // Function to interpolate the value at (x, y)
    double interpolate(double x, double y) const
    {
        return gsl_interp2d_eval(interp.get(), x_vals.data(), y_vals.data(), z_vals.data(), x, y,
                                 accel_x.get(), accel_y.get());
    }

    // Move constructor
    GSLInterp2D(GSLInterp2D &&other) noexcept
        : x_size(other.x_size), y_size(other.y_size), x_vals(std::move(other.x_vals)),
          y_vals(std::move(other.y_vals)), z_vals(std::move(other.z_vals)),
          accel_x(std::move(other.accel_x)), accel_y(std::move(other.accel_y)),
          interp(std::move(other.interp))
    {
        // Reset other object to a valid state
        other.x_size = 0;
        other.y_size = 0;
    }

    // Move assignment operator
    GSLInterp2D &operator=(GSLInterp2D &&other) noexcept
    {
        if (this != &other)
        {
            x_size = other.x_size;
            y_size = other.y_size;
            x_vals = std::move(other.x_vals);
            y_vals = std::move(other.y_vals);
            z_vals = std::move(other.z_vals);
            accel_x = std::move(other.accel_x);
            accel_y = std::move(other.accel_y);
            interp = std::move(other.interp);

            // Reset other object to a valid state
            other.x_size = 0;
            other.y_size = 0;
        }
        return *this;
    }

  private:
    size_t x_size, y_size;
    std::vector<double> x_vals;
    std::vector<double> y_vals;
    mutable std::vector<double> z_vals;

    // Declare unique pointers with custom deleters for GSL
    // objects
    std::unique_ptr<gsl_interp_accel, decltype(&gsl_interp_accel_free)> accel_x{
        nullptr, gsl_interp_accel_free};
    std::unique_ptr<gsl_interp_accel, decltype(&gsl_interp_accel_free)> accel_y{
        nullptr, gsl_interp_accel_free};
    std::unique_ptr<gsl_interp2d, decltype(&gsl_interp2d_free)> interp{nullptr, gsl_interp2d_free};
};