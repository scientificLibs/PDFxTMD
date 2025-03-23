#include <LHAPDF/LHAPDF.h>
#include <vector>
#include <memory>
#include <chrono>
#include <fstream>
#include <PDFxTMDLib/Factory.h>
#include <PDFxTMDLib/ITMD.h>
#include <PDFxTMDLib/GenericPDF.h>
#include <PDFxTMDLib/Implementation/Reader/Collinear/CDefaultLHAPDFFileReader.h>
#include <PDFxTMDLib/Implementation/Interpolator/Collinear/CLHAPDFBicubicInterpolator.h>
#include <PDFxTMDLib/Implementation/Extrapolator/Collinear/CContinuationExtrapolator.h>

int main()
{

    PDFxTMD::CollinearPDF mmht2014lo68clPDFxTMD("MMHT2014lo68cl", 0);
    std::vector<double> x_vals = {1e-4, 1e-3, 1e-2, 1e-1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8};
    std::vector<double> mu_vals = {sqrt(4.0), sqrt(5.0), sqrt(10.0), sqrt(20.0), sqrt(50.0), sqrt(100.0), sqrt(200.0), sqrt(500.0), sqrt(1000.0), sqrt(2000.0), sqrt(5000.0)};
    const int iterations = 1e6;
    // Open files for storing results
    std::ofstream performancePDFxTMD_log("performance_resultsPDFxTMD_CollinearPDFSingle.csv");
    std::ofstream validationPDFxTMD_log("numerical_validationPDFxTMD_CollinearPDFSingle.csv");
    performancePDFxTMD_log << "x,mu2,time_per_call_ns" << std::endl;
    validationPDFxTMD_log << "x,mu2,g" << std::endl;

    int t1PDFxTMD = time(0);
    int total_pointsPDFxTMD = x_vals.size() * mu_vals.size();
    int current_pointPDFxTMD = 0;
    for (const auto &x_test : x_vals)
    {
        for (const auto &mu_test : mu_vals)
        {
            current_pointPDFxTMD++;
            // Store results for validation
            auto start_time = std::chrono::high_resolution_clock::now();
            double pdfxtmdResult;
            for (int i = 0; i < iterations; i++)
            {
                pdfxtmdResult = mmht2014lo68clPDFxTMD.pdf(PDFxTMD::PartonFlavor::g, x_test, mu_test * mu_test);
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
            double time_per_call = duration.count() /(iterations);

            // Log performance results
            performancePDFxTMD_log << x_test << "," << mu_test * mu_test << ","
                                   << time_per_call << std::endl;

            // Log numerical validation results
            validationPDFxTMD_log << x_test << "," << mu_test * mu_test << "," << pdfxtmdResult << std::endl;

            // Progress update
            std::cout << "Completed point " << current_pointPDFxTMD << "/" << total_pointsPDFxTMD
                      << " (" << (current_pointPDFxTMD * 100.0 / total_pointsPDFxTMD) << "%)"
                      << " - Time per call: " << time_per_call << " ns" << std::endl;
        }
    }
    int t2PDFxTMD = time(0);

    // Close log files
    performancePDFxTMD_log.close();
    validationPDFxTMD_log.close();

    std::cout << "Total execution time: " << t2PDFxTMD - t1PDFxTMD << " seconds" << std::endl;
    std::cout << "Results written to performance_results.txt and numerical_validation.txt" << std::endl;
    return 0;
}
