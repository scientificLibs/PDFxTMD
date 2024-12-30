#include "MissingPDFSetHandler/ArchiveExtractorCommand.h"

namespace PDFxTMD
{
std::pair<bool, std::string> extract_archive(const std::string &archive_path,
                                             const std::string &output_dir)
{
    using namespace std::string_literals;
    struct archive *a;
    struct archive_entry *entry;
    int flags;
    int r;

    // Set extraction flags
    flags =
        ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS;

    // Create a new archive reader
    a = archive_read_new();
    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    // Open the archive file
    if ((r = archive_read_open_filename(a, archive_path.c_str(), 10240)))
    {
        archive_read_free(a);
        return {false, "Error opening archive: "s + archive_error_string(a)};
    }

    // Iterate over each entry in the archive
    while (true)
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
            break;
        if (r < ARCHIVE_OK)
            std::cerr << "Error reading header: " << archive_error_string(a) << std::endl;
        if (r < ARCHIVE_WARN)
        {
            archive_read_free(a);
            return {false, "Error reading header: "s + archive_error_string(a)};
        }

        // Construct the full output path
        std::string full_output_path = output_dir + "/" + archive_entry_pathname(entry);

        // Ensure the directory exists
        std::string dir_path = full_output_path.substr(0, full_output_path.find_last_of('/'));
        mkdir(dir_path.c_str(), 0777);

        // Set the output path
        archive_entry_set_pathname(entry, full_output_path.c_str());

        // Extract the entry
        r = archive_read_extract(a, entry, flags);
        if (r < ARCHIVE_OK)
            std::cerr << "Error extracting file: " << archive_error_string(a) << std::endl;
        if (r < ARCHIVE_WARN)
        {
            archive_read_free(a);
            return {false, "Error extracting file: "s + archive_error_string(a)};
        }
    }

    // Clean up
    archive_read_close(a);
    archive_read_free(a);

    return {true, ""};
}
bool ArchiveExtractorCommand::execute(StandardTypeMap &context)
{
    std::string selectedPath;
    if (std::holds_alternative<std::string>(context["SelectedPath"]))
    {
        selectedPath = std::get<std::string>(context["SelectedPath"]);
    }
    else
    {
        context["Error"] = "SelectedPath is not avilable in the context";
        return false;
    }
    std::string PDFSetAddress;
    if (std::holds_alternative<std::string>(context["PDFSetAddress"]))
    {
        PDFSetAddress = std::get<std::string>(context["PDFSetAddress"]);
    }
    else
    {
        context["Error"] = "PDFSetAddress is not avilable in the context";
        return false;
    }
    auto output = extract_archive(PDFSetAddress, selectedPath);
    if (output.first == false)
    {
        context["Error"] = output.second;
        return false;
    }
    return true;
}
} // namespace PDFxTMD
