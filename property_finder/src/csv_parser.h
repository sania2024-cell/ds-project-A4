#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <string>
#include <vector>
#include <fstream>

/**
 * CSVParser class for reading and writing CSV files
 */
class CSVParser
{
public:
    CSVParser();
    ~CSVParser();

    // Parsing methods
    std::vector<std::vector<std::string>> parseCSV(const std::string &filename);
    bool writeCSV(const std::string &filename, const std::vector<std::vector<std::string>> &data);

    // Configuration
    void setDelimiter(char delimiter) { delimiter_ = delimiter; }
    char getDelimiter() const { return delimiter_; }

    void setQuoteChar(char quote) { quote_char_ = quote; }
    char getQuoteChar() const { return quote_char_; }

private:
    char delimiter_;
    char quote_char_;

    // Helper methods
    std::vector<std::string> parseLine(const std::string &line);
    std::string escapeField(const std::string &field);
    bool needsQuoting(const std::string &field);
};

#endif // CSV_PARSER_H