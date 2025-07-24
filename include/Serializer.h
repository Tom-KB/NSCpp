/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */

#include <unordered_map>
#include <string>
#include <vector>
#include <shared_mutex>

#ifndef _SERIALIZER_H
#define _SERIALIZER_H

/**
 * @brief This function remove every occurence of the separator inside the value string.
 * @param separator Separator's string to remove inside the given string.
 * @param value String in which the separators will be removed.
 */
void sanitizeString(const std::string& separator, std::string& value);

/**
 * @brief Insert a value at the given index in a serialized string.
 * @warning The value at this index will be replaced by the one you gave.
 * @details Index is based on separator's split. E.g. : "A/B/C" with sep="/" => [0] = "A", [1] = "B", [2] = "C"
 * @param index Index of the insertion.
 * @param separator Separator used in the serialized string.
 * @param seriStr Serialized string on which the insertion is made.
 * @param value Value to insert at the given index.
 */
void insertAt(const size_t& index, const std::string& separator, std::string& seriStr, const std::string& value);


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
     * @brief Add to the string (given by its ID) the serialized version of the value, at the end.
     * @param ID Serialized string's ID.
     * @param value Value to append to the serialized string.
     */
    void add(const std::string& ID, const std::string& value);
    
    /**
     * @brief Add to the string (given by its ID) the serialized version of the value at the given index, or at the end.
     * @details Same behaviour as add without index if the given slot is higher than the number of slots in the string.
     * @etails This method have an higher time complexity than the other one since it need to search inside the string.
     * @param ID Serialized string's ID.
     * @param value Value to insert in the serialized string.
     * @param slot Slot in which the value should be insert.
     */
    void add(const std::string& ID, const std::string& value, size_t slot);
    
    /**
     * @brief Return the string of the given ID.
     * @param ID Desired serialized string's ID.
     */
    const std::string& get(const std::string& ID);

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
    std::unordered_map<std::string, std::string> serializedStrings;

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