#pragma once
#include <gsl/gsl_interp.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

class GSLInterp1D
{
  public:
    enum InterpolationType
    {
        Linear,
        Cubic
    };

    // Delete copy constructor and copy assignment operator
    GSLInterp1D(const GSLInterp1D &) = delete;
    GSLInterp1D &operator=(const GSLInterp1D &) = delete;

    // Constructor: Initializes the 1D interpolator with grid and data.
    GSLInterp1D(const std::vector<double> &x, const std::vector<double> &y, InterpolationType interpType)
        : x_size(x.size()), x_vals(x), y_vals(y)
    {
        if (x.size() != y.size())
        {
            std::cout << x.size() << "----" << y.size() << std::endl;
            throw std::invalid_argument("Mismatch in grid and data sizes");
        }

        // Allocate GSL object with custom deleter
        accel.reset(gsl_interp_accel_alloc());
        switch (interpType)
        {
        case InterpolationType::Linear:
            interp.reset(gsl_interp_alloc(gsl_interp_linear, x_size));
            break;
        case InterpolationType::Cubic:
            interp.reset(gsl_interp_alloc(gsl_interp_cspline, x_size));
            break;
        default:
            throw std::runtime_error("[GSLInterp1D] undefined interpolation type for gsl is selected!");
        }

        // Initialize the interpolator with grid and data points
        gsl_interp_init(interp.get(), x_vals.data(), y_vals.data(), x_size);
    }

    // Function to interpolate the value at x
    double interpolate(double x) const
    {
        return gsl_interp_eval(interp.get(), x_vals.data(), y_vals.data(), x, accel.get());
    }

    // Move constructor
    GSLInterp1D(GSLInterp1D &&other) noexcept
        : x_size(other.x_size), x_vals(std::move(other.x_vals)), y_vals(std::move(other.y_vals)),
          accel(std::move(other.accel)), interp(std::move(other.interp))
    {
        // Reset other object to a valid state
        other.x_size = 0;
    }

    // Move assignment operator
    GSLInterp1D &operator=(GSLInterp1D &&other) noexcept
    {
        if (this != &other)
        {
            x_size = other.x_size;
            x_vals = std::move(other.x_vals);
            y_vals = std::move(other.y_vals);
            accel = std::move(other.accel);
            interp = std::move(other.interp);

            // Reset other object to a valid state
            other.x_size = 0;
        }
        return *this;
    }

  private:
    size_t x_size;
    std::vector<double> x_vals;
    mutable std::vector<double> y_vals;

    // Declare unique pointers with custom deleters for GSL objects
    std::unique_ptr<gsl_interp_accel, decltype(&gsl_interp_accel_free)> accel{
        nullptr, gsl_interp_accel_free};
    std::unique_ptr<gsl_interp, decltype(&gsl_interp_free)> interp{nullptr, gsl_interp_free};
}; 