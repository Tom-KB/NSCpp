/**
 * Project NSCpp
 * @author Thomas K/BIDI
 * @version 2.0
 */

#include <unordered_map>
#include <string>
#include <vector>
#include <shared_mutex>
#include <sstream>

#ifndef _SERIALIZER_H
#define _SERIALIZER_H

/**
 * @brief This function remove every occurence of the separator inside the value string.
 * @param separator Separator's string to remove inside the given string.
 * @param value String in which the separators will be removed.
 */
void sanitizeString(const std::string& separator, std::string& value);

class Serializer {
public: 
    /**
     * @brief Return the current instance of the object or create one if it does not exist.
     * @details This class is a Singleton.
     */
    static Serializer& getInstance();
    
    /**
     * @brief Return the separator of the Serializer.
     */
    std::string getSeparator();
    
    /**
     * @brief Set the separator of the Serializer.
     * @details Automatically called by the Server/Client upon creation.
     * @param separator The separator that should be used by the Serializer.
     */
    void setSeparator(const std::string& separator);
    
    /**
     * @brief Add to the string (given by its ID) the sanitized version of the value, at the end.
     * @param ID Serialized string's ID.
     * @param value Value to append to the serialized string.
     */
    void add(const std::string& ID, const std::string& value);
    
    /**
     * @brief Add to the string (given by its ID) the sanitized version of the value at the given index, or at the end.
     * @details Same behaviour as add without index if the given slot is higher than the number of slots in the string.
     * @details This method have an higher time complexity than the other one in case of insertion inside the string.
     * @param ID Serialized string's ID.
     * @param value Value to insert in the serialized string.
     * @param idx Index at which the value should be insert.
     */
    void add(const std::string& ID, const std::string& value, size_t idx);

    /**
     * @brief Edit the string (given by its ID) with the serialized version of the value at the given index.
     * @details If there is no value at this index, insert the value at the end.
     * @param ID Serialized string's ID.
     * @param value Value to insert in the serialized string.
     * @param idx Index at which the value should be edit.
     */
    void edit(const std::string& ID, const std::string& value, size_t idx);
    
    /**
     * @brief Return the serialized string of the given ID.
     * @warning Build the string with the separator at each run.
     * @param ID Desired serialized string's ID.
     */
    std::string get(const std::string& ID);

    /**
     * @brief Return a vector containing the strings obtained after a split with the separator.
     * @param str The string to split using the separator.
     */
    std::vector<std::string> split(const std::string& str);
    
    /**
     * @brief Clear the string given by the ID.
     * @param ID Serialized string's ID to clear.
     */
    void clear(const std::string& ID);

protected: 
    /**
     * This string virtually divide the serialized strings.
     * E.g.:
     * STR1 = "ABC"
     * STR2 = "CBA"
     * sep = "/"
     * ==> "ABC/CBA" (when STR1 is concatened with STR2)
     */
    std::string separator;

     /**
      * Unordered map linking the ID of the serialized string and the serialized string.
      */
    std::unordered_map<std::string, std::vector<std::string>> serializedStrings;

private: 
    Serializer() = default;
    ~Serializer() = default;

    // Design to be used by multiple threads =>
    std::shared_mutex mtx;

    // We block the different way to copy or deplace it
    Serializer(const Serializer&) = delete;
    Serializer& operator=(const Serializer&) = delete;
    Serializer(Serializer&&) = delete;
    Serializer& operator=(Serializer&&) = delete;
    };

#endif //_SERIALIZER_H