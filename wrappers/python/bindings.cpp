#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "Factory.h"
#include "PolyWrapper.h"

namespace py = pybind11;

PYBIND11_MODULE(pdfxtmd, m) {
    m.doc() = "Python bindings for PDFxTMD library";

    py::enum_<PDFxTMD::PartonFlavor>(m, "PartonFlavor")
        .value("u", PDFxTMD::PartonFlavor::u)
        .value("d", PDFxTMD::PartonFlavor::d)
        .value("s", PDFxTMD::PartonFlavor::s)
        .value("c", PDFxTMD::PartonFlavor::c)
        .value("b", PDFxTMD::PartonFlavor::b)
        .value("t", PDFxTMD::PartonFlavor::t)
        .value("g", PDFxTMD::PartonFlavor::g)
        .value("ubar", PDFxTMD::PartonFlavor::ubar)
        .value("dbar", PDFxTMD::PartonFlavor::dbar)
        .value("sbar", PDFxTMD::PartonFlavor::sbar)
        .value("cbar", PDFxTMD::PartonFlavor::cbar)
        .value("bbar", PDFxTMD::PartonFlavor::bbar)
        .export_values();

    py::class_<PDFxTMD::PolyWrapper<PDFxTMD::IQCDCoupling>>(m, "IQCDCoupling")
        .def("alpha_qcd_mu2", &PDFxTMD::PolyWrapper<PDFxTMD::IQCDCoupling>::alpha_qcd_mu2,
             py::arg("mu2"),
             "Calculate alpha_s at given scale");

    py::class_<PDFxTMD::CouplingFactory>(m, "CouplingFactory")
        .def(py::init<>())
        .def("make_coupling", [](PDFxTMD::CouplingFactory &self, const std::string &pdfSetName) {
            return PDFxTMD::PolyWrapper<PDFxTMD::IQCDCoupling>(self.mkCoupling(pdfSetName));
        }, py::arg("pdf_set_name"),
           "Create a QCD coupling object");

    py::class_<PDFxTMD::PolyWrapper<PDFxTMD::ITMD>>(m, "ITMD")
        .def("tmd", &PDFxTMD::PolyWrapper<PDFxTMD::ITMD>::tmd, 
             py::arg("flavor"), py::arg("x"), py::arg("kt2"), py::arg("mu2"),
             "Calculate TMD value");
    py::class_<PDFxTMD::GenericTMDFactory>(m, "TMDFactory")
        .def(py::init<>())
        .def("make_tmd", [](PDFxTMD::GenericTMDFactory &self, const std::string &pdfSetName, int setMember) {
            return PDFxTMD::PolyWrapper<PDFxTMD::ITMD>(self.mkTMD(pdfSetName, setMember));
        }, py::arg("pdf_set_name"), py::arg("set_member"),
           "Create a TMD object");

    py::class_<PDFxTMD::PolyWrapper<PDFxTMD::ICPDF>>(m, "ICPDF")
        .def("pdf", &PDFxTMD::PolyWrapper<PDFxTMD::ICPDF>::pdf,
             py::arg("flavor"), py::arg("x"), py::arg("mu2"),
             "Calculate PDF value");

    py::class_<PDFxTMD::GenericCPDFFactory>(m, "CPDFFactory")
        .def(py::init<>())
        .def("make_cpdf", [](PDFxTMD::GenericCPDFFactory &self, const std::string &pdfSetName, int setMember) {
            return PDFxTMD::PolyWrapper<PDFxTMD::ICPDF>(self.mkCPDF(pdfSetName, setMember));
        }, py::arg("pdf_set_name"), py::arg("set_member"),
           "Create a CPDF object");

}