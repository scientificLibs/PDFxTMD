import pdfxtmd
import numpy as np
import matplotlib.pyplot as plt

# Initialize a collinear PDF set
cpdf_set = pdfxtmd.CPDFSet("CT18NLO")

# Access metadata
std_info = cpdf_set.getStdPDFInfo()
print(f"PDF set: {std_info.SetDesc}({std_info.NumMembers} members)")

# Evaluate PDFs for visualization
x_values = np.logspace(-4, -1, 100)  # x range from 10^-4 to 10^-1
mu2 = 1000  # GeV^2
kt2 = 1  # GeV^2

# Calculate gluon PDFs and uncertainties
gluon_pdfs = [cpdf_set[0].pdf(pdfxtmd.PartonFlavor.g, x, mu2) for x in x_values]
uncertainties = [cpdf_set.Uncertainty(pdfxtmd.PartonFlavor.g, x, mu2) for x in x_values]

# Extract upper and lower uncertainty bands for PDFs
upper_band = [g + u.errplus for g, u in zip(gluon_pdfs, uncertainties)]
lower_band = [g - u.errminus for g, u in zip(gluon_pdfs, uncertainties)]

# Initialize a TMD set
tmd_set = pdfxtmd.TMDSet("PB-LO-HERAI+II-2020-set2")
# Access metadata
std_info = tmd_set.getStdPDFInfo()
print(f"TMD set: {std_info.SetDesc}({std_info.NumMembers} members)")

# Calculate gluon TMDs and uncertainties
gluon_tmds = [tmd_set[0].tmd(pdfxtmd.PartonFlavor.g, x, kt2, mu2) for x in x_values]
tmd_uncertainties = [tmd_set.Uncertainty(pdfxtmd.PartonFlavor.g, x, kt2, mu2) for x in x_values]

# Extract upper and lower uncertainty bands for TMDs
tmd_upper_band = [g + u.errplus for g, u in zip(gluon_tmds, tmd_uncertainties)]
tmd_lower_band = [g - u.errminus for g, u in zip(gluon_tmds, tmd_uncertainties)]

# Plot Gluon PDF with uncertainty and save to file
plt.figure()
plt.plot(x_values, gluon_pdfs, label='Gluon PDF')
plt.fill_between(x_values, lower_band, upper_band, alpha=0.3, label='Uncertainty')
plt.xscale('log')
plt.xlabel('x')
plt.ylabel('PDF')
plt.title('Gluon PDF with Uncertainty')
plt.legend()
plt.savefig('gluon_pdf_plot.png')
plt.close()

# Plot Gluon TMD with uncertainty and save to file
plt.figure()
plt.plot(x_values, gluon_tmds, label='Gluon TMD')
plt.fill_between(x_values, tmd_lower_band, tmd_upper_band, alpha=0.3, label='Uncertainty')
plt.xscale('log')
plt.xlabel('x')
plt.ylabel('TMD')
plt.title('Gluon TMD with Uncertainty')
plt.legend()
plt.savefig('gluon_tmd_plot.png')
plt.close()