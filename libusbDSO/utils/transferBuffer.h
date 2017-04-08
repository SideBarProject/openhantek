#pragma once

class BulkUSB {};
class ControlUSB {};

//////////////////////////////////////////////////////////////////////////////
///
/// \brief A simple array with a fixed size allocated on the heap
class USBTransferBuffer {
public:
    USBTransferBuffer(unsigned int size, unsigned char extra = 0);
    ~USBTransferBuffer();

    unsigned char *data();
    const unsigned char *data() const;
    unsigned char operator[](unsigned int index);

    unsigned int size() const;

    unsigned char extra;

protected:
    unsigned char *array; ///< Pointer to the array holding the data
    unsigned int _size; ///< Size of the array (Number of variables of type T)
};
