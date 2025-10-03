#include "csv_parser.h"
#include <iostream>
#include <sstream>

CSVParser::CSVParser() : delimiter_(','), quote_char_('"') {}

CSVParser::~CSVParser() {}

std::vector<std::vector<std::string>> CSVParser::parseCSV(const std::string &filename)
{
    std::vector<std::vector<std::string>> data;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return data;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (!line.empty())
        {
            data.push_back(parseLine(line));
        }
    }

    file.close();
    return data;
}

bool CSVParser::writeCSV(const std::string &filename, const std::vector<std::vector<std::string>> &data)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Cannot create file " << filename << std::endl;
        return false;
    }

    for (size_t i = 0; i < data.size(); ++i)
    {
        const auto &row = data[i];

        for (size_t j = 0; j < row.size(); ++j)
        {
            file << escapeField(row[j]);
            if (j < row.size() - 1)
            {
                file << delimiter_;
            }
        }

        if (i < data.size() - 1)
        {
            file << std::endl;
        }
    }

    file.close();
    return true;
}

std::vector<std::string> CSVParser::parseLine(const std::string &line)
{
    std::vector<std::string> fields;
    std::string current_field;
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); ++i)
    {
        char ch = line[i];

        if (ch == quote_char_)
        {
            if (in_quotes && i + 1 < line.length() && line[i + 1] == quote_char_)
            {
                // Escaped quote
                current_field += quote_char_;
                ++i; // Skip next quote
            }
            else
            {
                // Toggle quote state
                in_quotes = !in_quotes;
            }
        }
        else if (ch == delimiter_ && !in_quotes)
        {
            // End of field
            fields.push_back(current_field);
            current_field.clear();
        }
        else
        {
            current_field += ch;
        }
    }

    // Add last field
    fields.push_back(current_field);

    return fields;
}

std::string CSVParser::escapeField(const std::string &field)
{
    if (needsQuoting(field))
    {
        std::string escaped = std::string(1, quote_char_);

        for (char ch : field)
        {
            if (ch == quote_char_)
            {
                escaped += std::string(2, quote_char_); // Escape quote
            }
            else
            {
                escaped += ch;
            }
        }

        escaped += quote_char_;
        return escaped;
    }

    return field;
}

bool CSVParser::needsQuoting(const std::string &field)
{
    return field.find(delimiter_) != std::string::npos ||
           field.find(quote_char_) != std::string::npos ||
           field.find('\n') != std::string::npos ||
           field.find('\r') != std::string::npos;
}