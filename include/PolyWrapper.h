#ifndef POLY_WRAPPER_H
#define POLY_WRAPPER_H

#include <lib3/boost/te.hpp>
#include "Factory.h"

namespace PDFxTMD {

template <typename Interface>
class PolyWrapper {
public:
    PolyWrapper(boost::te::poly<Interface> poly) : poly_(std::move(poly)) {}

    // Example method for ITMD
    double tmd(PartonFlavor flavor, double x, double kt2, double mu2) {
        return poly_.tmd(flavor, x, kt2, mu2);
    }

    // Example method for ICPDF
    double pdf(PartonFlavor flavor, double x, double mu2) {
        return poly_.pdf(flavor, x, mu2);
    }

    // Example method for IQCDCoupling
    double alpha_qcd_mu2(double mu2) {
        return poly_.AlphaQCDMu2(mu2);
    }

private:
    boost::te::poly<Interface> poly_;
};

} // namespace PDFxTMD

#endif // POLY_WRAPPER_H