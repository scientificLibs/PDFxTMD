#include "PDFxTMDLib/Common/ConfigWrapper.h"
#include "PDFxTMDLib/Common/PartonUtils.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlErrorInfo.h"
#include "PDFxTMDLib/Common/YamlMetaInfo/YamlStandardPDFInfo.h"
#include "PDFxTMDLib/Factory.h"
#include "PDFxTMDLib/Interface/ICPDF.h"
#include "PDFxTMDLib/Interface/IQCDCoupling.h"
#include "PDFxTMDLib/Interface/ITMD.h"
#include "PDFxTMDLib/PDFSet.h"
#include <array>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <stdexcept>
#include <string>

namespace py = pybind11;

PYBIND11_MODULE(pdfxtmd, m)
{
    m.doc() = "Python bindings for the PDFxTMD library, providing access to Collinear Parton "
              "Distribution Functions (CPDFs), "
              "Transverse Momentum Dependent (TMD) PDFs, QCD coupling calculations, and PDF sets.";

    // Existing bindings (unchanged)
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

    py::class_<PDFxTMD::IQCDCoupling>(m, "IQCDCoupling", "Interface for QCD coupling calculations")
        .def(
            "AlphaQCDMu2",
            [](const PDFxTMD::IQCDCoupling &self, double mu2) {
                try
                {
                    if (mu2 <= 0)
                        throw std::invalid_argument(
                            "Factorization scale squared (mu2) must be positive");
                    return self.AlphaQCDMu2(mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating alpha_s at mu2=" +
                                          std::to_string(mu2) + ": " + e.what());
                }
            },
            py::arg("mu2"), "Calculate the strong coupling constant alpha_s at the given scale.");

    py::class_<PDFxTMD::CouplingFactory>(m, "CouplingFactory",
                                         "Factory for creating QCD coupling objects")
        .def(py::init<>())
        .def(
            "mkCoupling",
            [](PDFxTMD::CouplingFactory &self, const std::string &pdfSetName) {
                try
                {
                    if (pdfSetName.empty())
                        throw std::invalid_argument("PDF set name cannot be empty");
                    return self.mkCoupling(pdfSetName);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error creating QCD coupling for PDF set '" + pdfSetName +
                                          "': " + e.what());
                }
            },
            py::arg("pdfSetName"), py::return_value_policy::take_ownership);

    py::class_<PDFxTMD::GenericTMDFactory>(m, "GenericTMDFactory",
                                           "Factory for creating TMD objects")
        .def(py::init<>())
        .def(
            "mkTMD",
            [](PDFxTMD::GenericTMDFactory &self, const std::string &pdfSetName, int setMember) {
                try
                {
                    if (pdfSetName.empty())
                        throw std::invalid_argument("PDF set name cannot be empty");
                    if (setMember < 0)
                        throw std::invalid_argument("Set member index must be non-negative");
                    return self.mkTMD(pdfSetName, setMember);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error creating TMD for '" + pdfSetName +
                                          "': " + e.what());
                }
            },
            py::arg("pdfSetName"), py::arg("setMember"), py::return_value_policy::take_ownership);
    // Bind ITMD
    py::class_<PDFxTMD::ITMD>(
        m, "ITMD",
        "Interface for Transverse Momentum Dependent (TMD) Parton Distribution Functions")
        .def(
            "tmd",
            [](const PDFxTMD::ITMD &self, PDFxTMD::PartonFlavor flavor, double x, double kt2,
               double mu2) {
                try
                {
                    if (x <= 0 || x >= 1)
                    {
                        throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                    }
                    if (kt2 < 0)
                    {
                        throw std::invalid_argument(
                            "Transverse momentum squared kt2 must be non-negative");
                    }
                    if (mu2 <= 0)
                    {
                        throw std::invalid_argument(
                            "Factorization scale squared mu2 must be positive");
                    }
                    return self.tmd(flavor, x, kt2, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error evaluating TMD for flavor " +
                                          std::to_string(static_cast<int>(flavor)) + " at x=" +
                                          std::to_string(x) + ", kt2=" + std::to_string(kt2) +
                                          ", mu2=" + std::to_string(mu2) + ": " + e.what());
                }
            },
            py::arg("flavor"), py::arg("x"), py::arg("kt2"), py::arg("mu2"),
            "Calculate the TMD PDF for a specific parton flavor.\n"
            "\nArgs:\n"
            "    flavor (PartonFlavor): The parton flavor (e.g., u, d, g).\n"
            "    x (float): Momentum fraction (0 < x < 1).\n"
            "    kt2 (float): Transverse momentum squared (GeV^2, non-negative).\n"
            "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
            "\nReturns:\n"
            "    float: The TMD PDF value.")
        .def(
            "tmd",
            [](const PDFxTMD::ITMD &self, double x, double kt2, double mu2, py::list &output) {
                try
                {
                    // Validate input parameters
                    if (x <= 0 || x >= 1)
                    {
                        throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                    }
                    if (kt2 < 0)
                    {
                        throw std::invalid_argument(
                            "Transverse momentum squared kt2 must be non-negative");
                    }
                    if (mu2 <= 0)
                    {
                        throw std::invalid_argument(
                            "Factorization scale squared mu2 must be positive");
                    }
                    std::array<double, 13> temp;
                    self.tmd(x, kt2, mu2, temp);
                    // Copy results to Python list
                    for (size_t i = 0; i < temp.size(); ++i)
                    {
                        output.append(temp[i]);
                    }
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error evaluating TMD for all flavors at x=" +
                                          std::to_string(x) + ", kt2=" + std::to_string(kt2) +
                                          ", mu2=" + std::to_string(mu2) + ": " + e.what());
                }
            },
            py::arg("x"), py::arg("kt2"), py::arg("mu2"), py::arg("output"),
            "Calculate TMD PDFs for all flavors and store in the provided list.\n"
            "\nArgs:\n"
            "    x (float): Momentum fraction (0 < x < 1).\n"
            "    kt2 (float): Transverse momentum squared (GeV^2, non-negative).\n"
            "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
            "    output (list): A list of 13 floats to store TMD values for "
            "{tbar, bbar, cbar, sbar, ubar, dbar, g, d, u, s, c, b, t}.\n"
            "\nReturns:\n"
            "    None: Modifies the output list in-place.");
    // Bind ICPDF
    py::class_<PDFxTMD::ICPDF>(m, "ICPDF",
                               "Interface for Collinear Parton Distribution Functions (CPDFs)")
        .def(
            "pdf",
            [](const PDFxTMD::ICPDF &self, PDFxTMD::PartonFlavor flavor, double x, double mu2) {
                try
                {
                    if (x <= 0 || x >= 1)
                    {
                        throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                    }
                    if (mu2 <= 0)
                    {
                        throw std::invalid_argument(
                            "Factorization scale squared mu2 must be positive");
                    }
                    return self.pdf(flavor, x, mu2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error evaluating CPDF for flavor " +
                                          std::to_string(static_cast<int>(flavor)) +
                                          " at x=" + std::to_string(x) +
                                          ", mu2=" + std::to_string(mu2) + ": " + e.what());
                }
            },
            py::arg("flavor"), py::arg("x"), py::arg("mu2"),
            "Calculate the CPDF for a specific parton flavor.\n"
            "\nArgs:\n"
            "    flavor (PartonFlavor): The parton flavor (e.g., u, d, g).\n"
            "    x (float): Momentum fraction (0 < x < 1).\n"
            "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
            "\nReturns:\n"
            "    float: The CPDF value.")
        .def(
            "pdf",
            [](const PDFxTMD::ICPDF &self, double x, double mu2, py::list &output) {
                try
                {
                    // Validate input parameters
                    if (x <= 0 || x >= 1)
                    {
                        throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                    }
                    if (mu2 <= 0)
                    {
                        throw std::invalid_argument(
                            "Factorization scale squared mu2 must be positive");
                    }
                    std::array<double, 13> temp;
                    self.pdf(x, mu2, temp);
                    for (size_t i = 0; i < temp.size(); ++i)
                    {
                        output.append(temp[i]);
                    }
                }
                catch (const std::exception &e)
                {
                    throw py::value_error(
                        "Error evaluating CPDF for all flavors at x=" + std::to_string(x) +
                        ", mu2=" + std::to_string(mu2) + ": " + e.what());
                }
            },
            py::arg("x"), py::arg("mu2"), py::arg("output"),
            "Calculate cPDFs for all flavors and store in the provided list.\n"
            "\nArgs:\n"
            "    x (float): Momentum fraction (0 < x < 1).\n"
            "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
            "    output (list): A list of 13 floats to store CPDF values for "
            "{tbar, bbar, cbar, sbar, ubar, dbar, g, d, u, s, c, b, t}.\n"
            "\nReturns:\n"
            "    None: Modifies the output list in-place.");

    py::class_<PDFxTMD::GenericCPDFFactory>(m, "GenericCPDFFactory",
                                            "Factory for creating CPDF objects")
        .def(py::init<>())
        .def(
            "mkCPDF",
            [](PDFxTMD::GenericCPDFFactory &self, const std::string &pdfSetName, int setMember) {
                try
                {
                    if (pdfSetName.empty())
                        throw std::invalid_argument("PDF set name cannot be empty");
                    if (setMember < 0)
                        throw std::invalid_argument("Set member index must be non-negative");
                    return self.mkCPDF(pdfSetName, setMember);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error creating CPDF for '" + pdfSetName +
                                          "': " + e.what());
                }
            },
            py::arg("pdfSetName"), py::arg("setMember"), py::return_value_policy::take_ownership);

    py::class_<PDFxTMD::PDFUncertainty>(m, "PDFUncertainty",
                                        "Structure holding PDF uncertainty information")
        .def_readonly("central", &PDFxTMD::PDFUncertainty::central, "Central value of the PDF")
        .def_readonly("errplus", &PDFxTMD::PDFUncertainty::errplus, "Positive error")
        .def_readonly("errminus", &PDFxTMD::PDFUncertainty::errminus, "Negative error")
        .def_readonly("errsymm", &PDFxTMD::PDFUncertainty::errsymm, "Symmetric error")
        .def_readonly("scale", &PDFxTMD::PDFUncertainty::scale, "Confidence level scale factor")
        .def_readonly("errplus_pdf", &PDFxTMD::PDFUncertainty::errplus_pdf, "Positive PDF error")
        .def_readonly("errminus_pdf", &PDFxTMD::PDFUncertainty::errminus_pdf, "Negative PDF error")
        .def_readonly("errsymm_pdf", &PDFxTMD::PDFUncertainty::errsymm_pdf, "Symmetric PDF error")
        .def_readonly("errplus_par", &PDFxTMD::PDFUncertainty::errplus_par,
                      "Positive parameter error")
        .def_readonly("errminus_par", &PDFxTMD::PDFUncertainty::errminus_par,
                      "Negative parameter error")
        .def_readonly("errsymm_par", &PDFxTMD::PDFUncertainty::errsymm_par,
                      "Symmetric parameter error")
        .def_property_readonly(
            "errparts",
            [](const PDFxTMD::PDFUncertainty &self) {
                py::list lst;
                for (const auto &p : self.errparts)
                {
                    lst.append(py::make_tuple(p.first, p.second));
                }
                return lst;
            },
            "List of error parts as (plus, minus) tuples");

    py::class_<PDFxTMD::YamlStandardPDFInfo>(m, "YamlStandardPDFInfo", "Standard PDF metadata")
        .def_readonly("NumMembers", &PDFxTMD::YamlStandardPDFInfo::NumMembers,
                      "Number of members in the set")
        .def_readonly("Flavors", &PDFxTMD::YamlStandardPDFInfo::Flavors,
                      "List of parton flavors included")
        .def_readonly("XMin", &PDFxTMD::YamlStandardPDFInfo::XMin, "Minimum value of x")
        .def_readonly("XMax", &PDFxTMD::YamlStandardPDFInfo::XMax, "Maximum value of x")
        .def_readonly("QMin", &PDFxTMD::YamlStandardPDFInfo::QMin, "Minimum value of Q")
        .def_readonly("QMax", &PDFxTMD::YamlStandardPDFInfo::QMax, "Maximum value of Q")
        .def_readonly("Format", &PDFxTMD::YamlStandardPDFInfo::Format, "Format")
        .def_readonly("SetDesc", &PDFxTMD::YamlStandardPDFInfo::SetDesc, "SetDesc")
        .def_readonly("lhapdfID", &PDFxTMD::YamlStandardPDFInfo::lhapdfID, "lhapdfID");
    py::class_<PDFxTMD::YamlStandardTMDInfo>(m, "YamlStandardTMDInfo", "Standard PDF metadata")
        .def_readonly("NumMembers", &PDFxTMD::YamlStandardTMDInfo::NumMembers,
                      "Number of members in the set")
        .def_readonly("Flavors", &PDFxTMD::YamlStandardTMDInfo::Flavors,
                      "List of parton flavors included")
        .def_readonly("XMin", &PDFxTMD::YamlStandardTMDInfo::XMin, "Minimum value of x")
        .def_readonly("XMax", &PDFxTMD::YamlStandardTMDInfo::XMax, "Maximum value of x")
        .def_readonly("QMin", &PDFxTMD::YamlStandardTMDInfo::QMin, "Minimum value of Q")
        .def_readonly("QMax", &PDFxTMD::YamlStandardTMDInfo::QMax, "Maximum value of Q")
        .def_readonly("Format", &PDFxTMD::YamlStandardTMDInfo::Format, "Format")
        .def_readonly("lhapdfID", &PDFxTMD::YamlStandardTMDInfo::lhapdfID, "lhapdfID")
        .def_readonly("KtMin", &PDFxTMD::YamlStandardTMDInfo::KtMin, "KtMin")
        .def_readonly("KtMax", &PDFxTMD::YamlStandardTMDInfo::KtMax, "KtMax")
        .def_readonly("SetDesc", &PDFxTMD::YamlStandardPDFInfo::SetDesc, "SetDesc")
        .def_readonly("TMDScheme", &PDFxTMD::YamlStandardTMDInfo::TMDScheme, "TMDScheme");

    py::class_<PDFxTMD::YamlErrorInfo>(m, "YamlErrorInfo", "PDF error metadata")
        .def_readonly("ErrorType", &PDFxTMD::YamlErrorInfo::ErrorType,
                      "Type of error set (e.g., 'replicas', 'hessian')")
        .def_readonly("ErrorConfLevel", &PDFxTMD::YamlErrorInfo::ErrorConfLevel,
                      "Confidence level of the error set in percent");

    py::class_<PDFxTMD::ConfigWrapper>(m, "ConfigWrapper", "YAML configuration data wrapper")
        .def(
            "get_string",
            [](const PDFxTMD::ConfigWrapper &self, const std::string &key) {
                auto result = self.get<std::string>(key);
                if (result.first)
                    return *result.first;
                throw py::key_error("Key '" + key + "' not found or has wrong type.");
            },
            py::arg("key"), "Get a value as a string.")
        .def(
            "get_int",
            [](const PDFxTMD::ConfigWrapper &self, const std::string &key) {
                auto result = self.get<int>(key);
                if (result.first)
                    return *result.first;
                throw py::key_error("Key '" + key + "' not found or has wrong type.");
            },
            py::arg("key"), "Get a value as an integer.")
        .def(
            "get_double",
            [](const PDFxTMD::ConfigWrapper &self, const std::string &key) {
                auto result = self.get<double>(key);
                if (result.first)
                    return *result.first;
                throw py::key_error("Key '" + key + "' not found or has wrong type.");
            },
            py::arg("key"), "Get a value as a double.")
        .def(
            "get_string_vector",
            [](const PDFxTMD::ConfigWrapper &self, const std::string &key) {
                auto result = self.get<std::vector<std::string>>(key);
                if (result.first)
                    return *result.first;
                throw py::key_error("Key '" + key + "' not found or has wrong type.");
            },
            py::arg("key"), "Get a value as a list of strings.");
    // Bind CPDFSet
    py::class_<PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag>>(
        m, "CPDFSet", "Class representing a set of Collinear PDFs")
        .def(py::init([](const std::string &pdfSetName, bool alternativeReplicaUncertainty) {
                 try
                 {
                     if (pdfSetName.empty())
                         throw std::invalid_argument("PDF set name cannot be empty");
                     return std::make_unique<PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag>>(
                         pdfSetName, alternativeReplicaUncertainty);
                 }
                 catch (const std::exception &e)
                 {
                     throw py::value_error("Error initializing CPDFSet for '" + pdfSetName +
                                           "': " + e.what());
                 }
             }),
             py::arg("pdfSetName"), py::arg("alternativeReplicaUncertainty") = false,
             "Initialize a CPDFSet.\n\n"
             "Args:\n"
             "    pdfSetName (str): Name of the PDF set.\n"
             "    alternativeReplicaUncertainty (bool, optional): Use alternative replica "
             "uncertainty method. Default is False.")
        .def(
            "alphasQ",
            [](const PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> &self, double q) {
                try
                {
                    if (q <= 0)
                        throw std::invalid_argument("Scale Q must be positive");
                    return self.alphasQ(q);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating alpha_s at Q=" + std::to_string(q) +
                                          ": " + e.what());
                }
            },
            py::arg("q"), "Calculate alpha_s at scale Q.")
        .def(
            "alphasQ2",
            [](const PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> &self, double q2) {
                try
                {
                    if (q2 <= 0)
                        throw std::invalid_argument("Scale Q^2 must be positive");
                    return self.alphasQ2(q2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating alpha_s at Q^2=" + std::to_string(q2) +
                                          ": " + e.what());
                }
            },
            py::arg("q2"), "Calculate alpha_s at scale Q^2.")
        .def(
            "__getitem__",
            [](PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> &self, int member) {
                try
                {
                    PDFxTMD::ICPDF *pdf = self[member];
                    if (!pdf)
                        throw py::index_error("Member index " + std::to_string(member) +
                                              " out of range");
                    return pdf;
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error accessing member " + std::to_string(member) +
                                          ": " + e.what());
                }
            },
            py::return_value_policy::reference_internal, py::arg("member"),
            "Get the CPDF object for the specified member index.")
        .def_property_readonly("size", &PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag>::size,
                               "Number of members in the PDF set.")
        .def("__len__", &PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag>::size,
             "Return the number of members in the PDF set.")
        .def(
            "Uncertainty",
            [](PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> &self, PDFxTMD::PartonFlavor flavor,
               double x, double mu2, double cl) {
                try
                {
                    if (x <= 0 || x >= 1)
                        throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                    if (mu2 <= 0)
                        throw std::invalid_argument(
                            "Factorization scale squared mu2 must be positive");
                    return self.Uncertainty(flavor, x, mu2, cl);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating uncertainty: " +
                                          std::string(e.what()));
                }
            },
            py::arg("flavor"), py::arg("x"), py::arg("mu2"), py::arg("cl") = -1,
            "Calculate uncertainty for the specified flavor and kinematics.\n\n"
            "Args:\n"
            "    flavor (PartonFlavor): Parton flavor.\n"
            "    x (float): Momentum fraction (0 < x < 1).\n"
            "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
            "    cl (float, optional): Confidence level percentage (-1 for default).")
        .def(
            "uncertainty",
            [](PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> &self, const std::vector<double> &values,
               double cl) {
                try
                {
                    return self.Uncertainty(values, cl);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating uncertainty from values: " +
                                          std::string(e.what()));
                }
            },
            py::arg("values"), py::arg("cl") = -1,
            "Calculate uncertainty from a vector of PDF values.")
        .def(
            "Correlation",
            [](PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> &self, PDFxTMD::PartonFlavor flavorA,
               double xA, double mu2A, PDFxTMD::PartonFlavor flavorB, double xB, double mu2B) {
                try
                {
                    if (xA <= 0 || xA >= 1 || xB <= 0 || xB >= 1)
                        throw std::invalid_argument("Momentum fractions must be in (0, 1)");
                    if (mu2A <= 0 || mu2B <= 0)
                        throw std::invalid_argument(
                            "Factorization scales squared must be positive");
                    return self.Correlation(flavorA, xA, mu2A, flavorB, xB, mu2B);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating correlation: " +
                                          std::string(e.what()));
                }
            },
            py::arg("flavorA"), py::arg("xA"), py::arg("mu2A"), py::arg("flavorB"), py::arg("xB"),
            py::arg("mu2B"), "Calculate correlation between two PDF points.")
        .def(
            "Correlation",
            [](PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag> &self, const std::vector<double> &valuesA,
               const std::vector<double> &valuesB) {
                try
                {
                    return self.Correlation(valuesA, valuesB);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating correlation from values: " +
                                          std::string(e.what()));
                }
            },
            py::arg("valuesA"), py::arg("valuesB"),
            "Calculate correlation between two vectors of PDF values.")
        .def("getStdPDFInfo", &PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag>::getStdPDFInfo,
             py::return_value_policy::move, "Get the standard metadata info object for the set.")
        .def("getPDFErrorInfo", &PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag>::getPDFErrorInfo,
             py::return_value_policy::move, "Get the error metadata info object for the set.")
        .def("info", &PDFxTMD::PDFSet<PDFxTMD::CollinearPDFTag>::info,
             py::return_value_policy::move, "Get the configuration info object for the set.");

    // Bind TMDSet
    py::class_<PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag>>(m, "TMDSet",
                                                    "Class representing a set of TMD PDFs")
        .def(py::init([](const std::string &pdfSetName, bool alternativeReplicaUncertainty) {
                 try
                 {
                     if (pdfSetName.empty())
                         throw std::invalid_argument("PDF set name cannot be empty");
                     return std::make_unique<PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag>>(
                         pdfSetName, alternativeReplicaUncertainty);
                 }
                 catch (const std::exception &e)
                 {
                     throw py::value_error("Error initializing TMDSet for '" + pdfSetName +
                                           "': " + e.what());
                 }
             }),
             py::arg("pdfSetName"), py::arg("alternativeReplicaUncertainty") = false,
             "Initialize a TMDSet.\n\n"
             "Args:\n"
             "    pdfSetName (str): Name of the PDF set.\n"
             "    alternativeReplicaUncertainty (bool, optional): Use alternative replica "
             "uncertainty method. Default is False.")
        .def(
            "alphasQ",
            [](const PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> &self, double q) {
                try
                {
                    if (q <= 0)
                        throw std::invalid_argument("Scale Q must be positive");
                    return self.alphasQ(q);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating alpha_s at Q=" + std::to_string(q) +
                                          ": " + e.what());
                }
            },
            py::arg("q"), "Calculate alpha_s at scale Q.")
        .def(
            "alphasQ2",
            [](const PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> &self, double q2) {
                try
                {
                    if (q2 <= 0)
                        throw std::invalid_argument("Scale Q^2 must be positive");
                    return self.alphasQ2(q2);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating alpha_s at Q^2=" + std::to_string(q2) +
                                          ": " + e.what());
                }
            },
            py::arg("q2"), "Calculate alpha_s at scale Q^2.")
        .def(
            "__getitem__",
            [](PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> &self, int member) {
                try
                {
                    PDFxTMD::ITMD *pdf = self[member];
                    if (!pdf)
                        throw py::index_error("Member index " + std::to_string(member) +
                                              " out of range");
                    return pdf;
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error accessing member " + std::to_string(member) +
                                          ": " + e.what());
                }
            },
            py::return_value_policy::reference_internal, py::arg("member"),
            "Get the TMD object for the specified member index.")
        .def_property_readonly("size", &PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag>::size,
                               "Number of members in the PDF set.")
        .def("__len__", &PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag>::size,
             "Return the number of members in the PDF set.")
        .def(
            "Uncertainty",
            [](PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> &self, PDFxTMD::PartonFlavor flavor, double x,
               double kt2, double mu2, double cl) {
                try
                {
                    if (x <= 0 || x >= 1)
                        throw std::invalid_argument("Momentum fraction x must be in (0, 1)");
                    if (kt2 < 0)
                        throw std::invalid_argument(
                            "Transverse momentum squared kt2 must be non-negative");
                    if (mu2 <= 0)
                        throw std::invalid_argument(
                            "Factorization scale squared mu2 must be positive");
                    return self.Uncertainty(flavor, x, kt2, mu2, cl);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating uncertainty: " +
                                          std::string(e.what()));
                }
            },
            py::arg("flavor"), py::arg("x"), py::arg("kt2"), py::arg("mu2"), py::arg("cl") = -1,
            "Calculate uncertainty for the specified flavor and kinematics.\n\n"
            "Args:\n"
            "    flavor (PartonFlavor): Parton flavor.\n"
            "    x (float): Momentum fraction (0 < x < 1).\n"
            "    kt2 (float): Transverse momentum squared (GeV^2, non-negative).\n"
            "    mu2 (float): Factorization scale squared (GeV^2, positive).\n"
            "    cl (float, optional): Confidence level percentage (-1 for default).")
        .def(
            "Uncertainty",
            [](PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> &self, const std::vector<double> &values,
               double cl) {
                try
                {
                    return self.Uncertainty(values, cl);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating uncertainty from values: " +
                                          std::string(e.what()));
                }
            },
            py::arg("values"), py::arg("cl") = -1.0,
            "Calculate uncertainty from a vector of PDF values.")
        .def(
            "Correlation",
            [](PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> &self, PDFxTMD::PartonFlavor flavorA, double xA,
               double kt2A, double mu2A, PDFxTMD::PartonFlavor flavorB, double xB, double kt2B,
               double mu2B) {
                try
                {
                    if (xA <= 0 || xA >= 1 || xB <= 0 || xB >= 1)
                        throw std::invalid_argument("Momentum fractions must be in (0, 1)");
                    if (kt2A < 0 || kt2B < 0)
                        throw std::invalid_argument(
                            "Transverse momentum squared must be non-negative");
                    if (mu2A <= 0 || mu2B <= 0)
                        throw std::invalid_argument(
                            "Factorization scales squared must be positive");
                    return self.Correlation(flavorA, xA, kt2A, mu2A, flavorB, xB, kt2B, mu2B);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating correlation: " +
                                          std::string(e.what()));
                }
            },
            py::arg("flavorA"), py::arg("xA"), py::arg("kt2A"), py::arg("mu2A"), py::arg("flavorB"),
            py::arg("xB"), py::arg("kt2B"), py::arg("mu2B"),
            "Calculate correlation between two TMD PDF points.")
        .def(
            "Correlation",
            [](PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag> &self, const std::vector<double> &valuesA,
               const std::vector<double> &valuesB) {
                try
                {
                    return self.Correlation(valuesA, valuesB);
                }
                catch (const std::exception &e)
                {
                    throw py::value_error("Error calculating correlation from values: " +
                                          std::string(e.what()));
                }
            },
            py::arg("valuesA"), py::arg("valuesB"),
            "Calculate correlation between two vectors of PDF values.")
        .def("getStdPDFInfo", &PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag>::getStdPDFInfo,
             py::return_value_policy::move, "Get the standard metadata info object for the set.")
        .def("getPDFErrorInfo", &PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag>::getPDFErrorInfo,
             py::return_value_policy::move, "Get the error metadata info object for the set.")
        .def("info", &PDFxTMD::PDFSet<PDFxTMD::TMDPDFTag>::info, py::return_value_policy::move,
             "Get the configuration info object for the set.");
}