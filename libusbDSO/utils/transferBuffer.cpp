#include "utils/transferBuffer.h"

/// \brief Initializes the data array.
/// \param size Size of the data array.
USBTransferBuffer::USBTransferBuffer(unsigned int size, unsigned char extra) : extra(extra) {
        this->array = new unsigned char[size];
        for(unsigned int index = 0; index < size; ++index)
                this->array[index] = 0;
        this->_size = size;
}

/// \brief Deletes the allocated data array.
USBTransferBuffer::~USBTransferBuffer() {
        delete[] this->array;
}

/// \brief Returns a pointer to the array data.
/// \return The internal data array.
const unsigned char *USBTransferBuffer::data() const {
        return this->array;
}

/// \brief Returns a pointer to the array data.
/// \return The internal data array.
unsigned char *USBTransferBuffer::data() {
        return this->array;
}


/// \brief Returns array element when using square brackets.
/// \return The array element.
unsigned char USBTransferBuffer::operator[](unsigned int index) {
        return this->array[index];
}

/// \brief Gets the size of the array.
/// \return The size of the command in bytes.
unsigned int USBTransferBuffer::size() const {
        return this->_size;
}
