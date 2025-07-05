#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <stdexcept>
#include <string>
#include "PDFxTMDLib/Factory.h"
#include "PDFxTMDLib/Interface/ICPDF.h"
#include "PDFxTMDLib/Interface/ITMD.h"
#include "PDFxTMDLib/Interface/IQCDCoupling.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include <array>

namespace py = pybind11;

PYBIND11_MODULE(pdfxtmd, m) {
    m.doc() = "Python bindings for the PDFxTMD library, providing access to Collinear Parton Distribution Functions (CPDFs), "
              "Transverse Momentum Dependent (TMD) PDFs, and QCD coupling calculations.";

    // Bind PartonFlavor enum
    py::enum_<PDFxTMD::PartonFlavor>(m, "PartonFlavor", "Enum representing parton flavors")
        .value("u", PDFxTMD::PartonFlavor::u, "Up quark")
        .value("d", PDFxTMD::PartonFlavor::d, "Down quark")
        .value("s", PDFxTMD::PartonFlavor::s, "Strange quark")
        .value("c", PDFxTMD::PartonFlavor::c, "Charm quark")
        .value("b", PDFxTMD::PartonFlavor::b, "Bottom quark")
        .value("t", PDFxTMD::PartonFlavor::t, "Top quark")
        .value("g", PDFxTMD::PartonFlavor::g, "Gluon")
        .value("ubar", PDFxTMD::PartonFlavor::ubar, "Up antiquark")
        .value("dbar", PDFxTMD::PartonFlavor::dbar, "Down antiquark")
        .value("sbar", PDFxTMD::PartonFlavor::sbar, "Strange antiquark")
        .value("cbar", PDFxTMD::PartonFlavor::cbar, "Charm antiquark")
        .value("bbar", PDFxTMD::PartonFlavor::bbar, "Bottom antiquark")
        .value("tbar", PDFxTMD::PartonFlavor::tbar, "Top antiquark")
        .export_values();

    // Bind IQCDCoupling
    py::class_<PDFxTMD::IQCDCoupling>(m, "IQCDCoupling", "Interface for QCD coupling calculations")
        .def("AlphaQCDMu2", [](const PDFxTMD::IQCDCoupling& self, double mu2) {
            try {
                if (mu2 <= 0) {
                    throw std::invalid_argument("Factorization scale squared (mu2) must be positive");
                }
                return self.AlphaQCDMu2(mu2);
            } catch (const std::exception& e) {
                throw py::value_error("Error calculating alpha_s at mu2=" + std::to_string(mu2) + ": " + e.what());
            }
        }, py::arg("mu2"),
           "Calculate the strong coupling constant alpha_s at the given scale.\n"
           "\nArgs:\n"
           "    mu2 (float): Factorization scale squared (GeV^2, must be positive).\n"
           "\nReturns:\n"
           "    float: The value of alpha_s.");

    // Bind CouplingFactory
    py::class_<PDFxTMD::CouplingFactory>(m, "CouplingFactory", "Factory for creating QCD coupling objects")
        .def(py::init<>(), "Initialize an empty coupling factory")
        .def("mkCoupling", [](PDFxTMD::CouplingFactory& self, const std::string& pdfSetName) {
            try {
                if (pdfSetName.empty()) {
                    throw std::invalid_argument("PDF set name cannot be empty");
                }
                return self.mkCoupling(pdfSetName);
            } catch (const std::exception& e) {
                throw py::value_error("Error creating QCD coupling for PDF set '" + pdfSetName + "': " + e.what());
            }
        }, py::arg("pdfSetName"), py::return_value_policy::take_ownership,
           "Create a QCD coupling object for the specified PDF set.\n"
           "\nArgs:\n"
           "    pdfSetName (str): Name of the PDF set (e.g., 'CT18').\n"
           "\nReturns:\n"
           "    IQCDCoupling: A QCD coupling object.");

    // Bind ITMD
    py::class_<PDFxTMD::ITMD>(m, "ITMD", "Interface for Transverse Momentum Dependent (TMD) Parton Distribution Functions")
        .def("tmd", [](const PDFxTMD::ITMD& self, PDFxTMD::PartonFlavor flavor, double x, double kt2, double mu2) {
            try {
                if (x <= 0 || x >= 1) {
                    throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                }
                if (kt2 < 0) {
                    throw std::invalid_argument("Transverse momentum squared kt2 must be non-negative");
                }
                if (mu2 <= 0) {
                    throw std::invalid_argument("Factorization scale squared mu2 must be positive");
                }
                return self.tmd(flavor, x, kt2, mu2);
            } catch (const std::exception& e) {
                throw py::value_error("Error evaluating TMD for flavor " + std::to_string(static_cast<int>(flavor)) +
                                      " at x=" + std::to_string(x) + ", kt2=" + std::to_string(kt2) +
                                      ", mu2=" + std::to_string(mu2) + ": " + e.what());
            }
        }, py::arg("flavor"), py::arg("x"), py::arg("kt2"), py::arg("mu2"),
           "Calculate the TMD PDF for a specific parton flavor.\n"
           "\nArgs:\n"
           "    flavor (PartonFlavor): The parton flavor (e.g., u, d, g).\n"
           "    x (float): Momentum fraction (0 < x < 1).\n"
           "    kt2 (float): Transverse momentum squared (GeV^2, non-negative).\n"
           "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
           "\nReturns:\n"
           "    float: The TMD PDF value.")
        .def("tmd", [](const PDFxTMD::ITMD& self, double x, double kt2, double mu2, py::list& output) {
            try {
                // Validate input parameters
                if (x <= 0 || x >= 1) {
                    throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                }
                if (kt2 < 0) {
                    throw std::invalid_argument("Transverse momentum squared kt2 must be non-negative");
                }
                if (mu2 <= 0) {
                    throw std::invalid_argument("Factorization scale squared mu2 must be positive");
                }
                std::array<double, 13> temp;
                self.tmd(x, kt2, mu2, temp);
                // Copy results to Python list
                for (size_t i = 0; i < temp.size(); ++i)
                {
                    output.append(temp[i]);
                }
            } catch (const std::exception& e) {
                throw py::value_error("Error evaluating TMD for all flavors at x=" + std::to_string(x) +
                                      ", kt2=" + std::to_string(kt2) + ", mu2=" + std::to_string(mu2) +
                                      ": " + e.what());
            }
        }, py::arg("x"), py::arg("kt2"), py::arg("mu2"), py::arg("output"),
           "Calculate TMD PDFs for all flavors and store in the provided list.\n"
           "\nArgs:\n"
           "    x (float): Momentum fraction (0 < x < 1).\n"
           "    kt2 (float): Transverse momentum squared (GeV^2, non-negative).\n"
           "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
           "    output (list): A list of 13 floats to store TMD values for "
           "{tbar, bbar, cbar, sbar, ubar, dbar, g, d, u, s, c, b, t}.\n"
           "\nReturns:\n"
           "    None: Modifies the output list in-place.");

    // Bind GenericTMDFactory
    py::class_<PDFxTMD::GenericTMDFactory>(m, "GenericTMDFactory", "Factory for creating TMD objects")
        .def(py::init<>(), "Initialize an empty TMD factory")
        .def("mkTMD", [](PDFxTMD::GenericTMDFactory& self, const std::string& pdfSetName, int setMember) {
            try {
                if (pdfSetName.empty()) {
                    throw std::invalid_argument("PDF set name cannot be empty");
                }
                if (setMember < 0) {
                    throw std::invalid_argument("Set member index must be non-negative");
                }
                return self.mkTMD(pdfSetName, setMember);
            } catch (const std::exception& e) {
                throw py::value_error("Error creating TMD for PDF set '" + pdfSetName + "', member " +
                                      std::to_string(setMember) + ": " + e.what());
            }
        }, py::arg("pdfSetName"), py::arg("setMember"), py::return_value_policy::take_ownership,
           "Create a TMD object for the specified PDF set and member.\n"
           "\nArgs:\n"
           "    pdfSetName (str): Name of the PDF set (e.g., 'PB-NLO-HERAI+II-2023-set2-qs=0.74').\n"
           "    setMember (int): Member index of the PDF set (non-negative).\n"
           "\nReturns:\n"
           "    ITMD: A TMD object.");

    // Bind ICPDF
    py::class_<PDFxTMD::ICPDF>(m, "ICPDF", "Interface for Collinear Parton Distribution Functions (CPDFs)")
        .def("pdf", [](const PDFxTMD::ICPDF& self, PDFxTMD::PartonFlavor flavor, double x, double mu2) {
            try {
                if (x <= 0 || x >= 1) {
                    throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                }
                if (mu2 <= 0) {
                    throw std::invalid_argument("Factorization scale squared mu2 must be positive");
                }
                return self.pdf(flavor, x, mu2);
            } catch (const std::exception& e) {
                throw py::value_error("Error evaluating CPDF for flavor " + std::to_string(static_cast<int>(flavor)) +
                                      " at x=" + std::to_string(x) + ", mu2=" + std::to_string(mu2) +
                                      ": " + e.what());
            }
        }, py::arg("flavor"), py::arg("x"), py::arg("mu2"),
           "Calculate the CPDF for a specific parton flavor.\n"
           "\nArgs:\n"
           "    flavor (PartonFlavor): The parton flavor (e.g., u, d, g).\n"
           "    x (float): Momentum fraction (0 < x < 1).\n"
           "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
           "\nReturns:\n"
           "    float: The CPDF value.")
        .def("pdf", [](const PDFxTMD::ICPDF& self, double x, double mu2, py::list& output) {
            try {
                // Validate input parameters
                if (x <= 0 || x >= 1) {
                    throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                }
                if (mu2 <= 0) {
                    throw std::invalid_argument("Factorization scale squared mu2 must be positive");
                }
                std::array<double, 13> temp;
                self.pdf(x, mu2, temp);
                for (size_t i = 0; i < temp.size(); ++i) {
                    output.append(temp[i]);
                }
            } catch (const std::exception& e) {
                throw py::value_error("Error evaluating CPDF for all flavors at x=" + std::to_string(x) +
                                      ", mu2=" + std::to_string(mu2) + ": " + e.what());
            }
        }, py::arg("x"), py::arg("mu2"), py::arg("output"),
           "Calculate cPDFs for all flavors and store in the provided list.\n"
           "\nArgs:\n"
           "    x (float): Momentum fraction (0 < x < 1).\n"
           "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
           "    output (list): A list of 13 floats to store CPDF values for "
           "{tbar, bbar, cbar, sbar, ubar, dbar, g, d, u, s, c, b, t}.\n"
           "\nReturns:\n"
           "    None: Modifies the output list in-place.");

    // Bind GenericCPDFFactory
    py::class_<PDFxTMD::GenericCPDFFactory>(m, "GenericCPDFFactory", "Factory for creating CPDF objects")
        .def(py::init<>(), "Initialize an empty CPDF factory")
        .def("mkCPDF", [](PDFxTMD::GenericCPDFFactory& self, const std::string& pdfSetName, int setMember) {
            try {
                if (pdfSetName.empty()) {
                    throw std::invalid_argument("PDF set name cannot be empty");
                }
                if (setMember < 0) {
                    throw std::invalid_argument("Set member index must be non-negative");
                }
                return self.mkCPDF(pdfSetName, setMember);
            } catch (const std::exception& e) {
                throw py::value_error("Error creating CPDF for PDF set '" + pdfSetName + "', member " +
                                      std::to_string(setMember) + ": " + e.what());
            }
        }, py::arg("pdfSetName"), py::arg("setMember"), py::return_value_policy::take_ownership,
           "Create a CPDF object for the specified PDF set and member.\n"
           "\nArgs:\n"
           "    pdfSetName (str): Name of the PDF set (e.g., 'CT18').\n"
           "    setMember (int): Member index of the PDF set (non-negative).\n"
           "\nReturns:\n"
           "    ICPDF: A CPDF object.");
}