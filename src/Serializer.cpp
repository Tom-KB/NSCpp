/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */

#include "Serializer.h"

using namespace std;

void sanitizeString(const string& separator, string& value) {
    if (!separator.size()) return;
    size_t pos = value.find(separator);

    while (pos != string::npos) {
        value.erase(pos, separator.size());
        pos = value.find(separator, pos);
    }
}

Serializer& Serializer::getInstance() {
    static Serializer instance;
    return instance;
}


string Serializer::getSeparator() {
    return separator;
}

void Serializer::setSeparator(const string& sep) {
    separator = sep;
}

void Serializer::add(const string& ID, const string& value) {
    scoped_lock lock(mtx);
    string edit = value;
    sanitizeString(separator, edit);
    serializedStrings[ID].push_back(edit);
}

void Serializer::add(const string& ID, const string& value, size_t idx) {
    scoped_lock lock(mtx);
    if (idx < 0) return;
    string edit = value;
    sanitizeString(separator, edit);
    if (idx >= serializedStrings[ID].size()) {
        serializedStrings[ID].push_back(edit);
    }
    else {
        serializedStrings[ID].insert(serializedStrings[ID].begin() + idx, edit);
    }
}

void Serializer::edit(const std::string& ID, const std::string& value, size_t idx) {
    scoped_lock lock(mtx);
    if (idx < 0) return;
    string edit = value;
    sanitizeString(separator, edit);
    if (serializedStrings[ID].size() <= idx) serializedStrings[ID].push_back(edit);
    serializedStrings[ID][idx] = edit;
}

string Serializer::get(const string& ID) {
    scoped_lock lock(mtx);
    stringstream ss;
    for (int i = 0; i < serializedStrings[ID].size(); ++i) {
        ss << serializedStrings[ID][i];
        if (i != serializedStrings[ID].size() - 1) ss << separator;
    }
    return ss.str();
}

vector<string> Serializer::split(const string& str) {
    vector<string> result;
    size_t posStart = 0;
    size_t posSep = string::npos;
    while ((posSep = str.find(separator, posStart)) != string::npos) {
        result.emplace_back(str.substr(posStart, posSep - posStart));
        posStart = posSep + separator.size(); // We restart after the separator
    }
    // Add the last part
    result.emplace_back(str.substr(posStart));
    return result;
}

void Serializer::clear(const string& ID) {
    scoped_lock lock(mtx);
    auto it = serializedStrings.find(ID);
    if (it != serializedStrings.end()) serializedStrings[ID].clear();
}