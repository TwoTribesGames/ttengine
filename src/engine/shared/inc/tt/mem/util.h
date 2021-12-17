#if !defined(INC_TT_MEM_UTIL_H)
#define INC_TT_MEM_UTIL_H

#include <tt/mem/types.h>


namespace tt {
namespace mem {

/*! \brief Copies a block of memory, no alignment required.
    \param p_dest Pointer to the destination buffer of at least p_size bytes.
                  Must NOT overlap p_source.
    \param p_source Pointer to the source buffer of at least p_size bytes.
                    Must NOT overlap p_dest.
    \param p_size The number of bytes to copy.*/
void copy8(void* p_dest, const void* p_source, size_type p_size);

/*! \brief Copies a block of memory, 16 bit alignment required.
    \param p_dest Pointer to the destination buffer of at least p_size bytes.
                  Must be 16 bit aligned, must NOT overlap p_source.
    \param p_source Pointer to the source buffer of at least p_size bytes.
                    Must be 16 bit aligned, must NOT overlap p_dest.
    \param p_size The number of bytes to copy. Must be 16 bit aligned.*/
void copy16(void* p_dest, const void* p_source, size_type p_size);

/*! \brief Copies a block of memory, 32 bit alignment required.
    \param p_dest Pointer to the destination buffer of at least p_size bytes.
                  Must be 32 bit aligned, must NOT overlap p_source.
    \param p_source Pointer to the source buffer of at least p_size bytes.
                    Must be 32 bit aligned, must NOT overlap p_dest.
    \param p_size The number of bytes to copy. Must be 32 bit aligned.*/
void copy32(void* p_dest, const void* p_source, size_type p_size);


void copyDMA(void* p_dest, const void* p_source, size_type p_size);

/*! \brief Copies a 2-dimensional block of memory that requires a certain pitch
    \param p_dest Pointer to the destination buffer of at least p_dstPitch * p_rows bytes.
    \param p_source Pointer to the source buffer of at least p_srcPitch * p_rows bytes.
    \param p_srcPitch Number of bytes in a single source row.
    \param p_dstPitch Number of bytes in a single destination row
    \paraw p_rows     Number of rows to copy */
void copyWithPitch(void* p_dest, const void* p_source, u32 p_srcPitch, u32 p_dstPitch, u32 p_rows);


/*! \brief Moves a block of memory, no alignment required, source and destination may overlap.
    \param p_dest Pointer to the destination buffer of at least p_size bytes.
                  May overlap p_source.
    \param p_source Pointer to the source buffer of at least p_size bytes.
                    May overlap p_dest.
    \param p_size The number of bytes to move.*/
void move8(void* p_dest, const void* p_source, size_type p_size);

/*! \brief Moves a block of memory, 16 bit alignment required, source and destination may overlap.
    \param p_dest Pointer to the destination buffer of at least p_size bytes.
                  Must be 16 bit aligned, may overlap p_source.
    \param p_source Pointer to the source buffer of at least p_size bytes.
                    Must be 16 bit aligned, may overlap p_dest.
    \param p_size The number of bytes to move. Must be 16 bit aligned.*/
void move16(void* p_dest, const void* p_source, size_type p_size);

/*! \brief Moves a block of memory, 32 bit alignment required, source and destination may overlap.
    \param p_dest Pointer to the destination buffer of at least p_size bytes.
                  Must be 32 bit aligned, may overlap p_source.
    \param p_source Pointer to the source buffer of at least p_size bytes.
                    Must be 32 bit aligned, may overlap p_dest.
    \param p_size The number of bytes to move. Must be 32 bit aligned.*/
void move32(void* p_dest, const void* p_source, size_type p_size);


/*! \brief Fills a block of memory with the specified value.
    \param p_buffer Pointer to the destination buffer of at least p_size bytes.
    \param p_value The value with which the buffer should be filled.
    \param p_size The number of bytes to fill.*/
void fill8(void* p_buffer, u8 p_value, size_type p_size);

/*! \brief Fills a block of memory with the specified 16 bit value.
    \param p_buffer Pointer to the destination buffer of at least p_size bytes.
                    Must be 16 bit aligned.
    \param p_value The value with which the buffer should be filled.
    \param p_size The number of bytes to fill.
                  Must be 16 bit aligned.*/
void fill16(void* p_buffer, u16 p_value, size_type p_size);

/*! \brief Fills a block of memory with the specified 32 bit value.
    \param p_buffer Pointer to the destination buffer of at least p_size bytes.
                    Must be 32 bit aligned.
    \param p_value The value with which the buffer should be filled.
    \param p_size The number of bytes to fill.
                  Must be 32 bit aligned.*/
void fill32(void* p_buffer, u32 p_value, size_type p_size);


/*! \brief Zero clears a block of memory.
    \param p_buffer Pointer to the destination buffer of at least p_size bytes.
    \param p_size The number of bytes to clear.*/
void zero8(void* p_buffer, size_type p_size);

/*! \brief Zero clears a block of memory in 16 bit units.
    \param p_buffer Pointer to the destination buffer of at least p_size bytes.
                    Must be 16 bit aligned.
    \param p_size The number of bytes to clear. Must be 16 bit aligned.*/
void zero16(void* p_buffer, size_type p_size);

/*! \brief Zero clears a block of memory in 32 bit units.
    \param p_buffer Pointer to the destination buffer of at least p_size bytes.
                    Must be 32 bit aligned.
    \param p_size The number of bytes to clear. Must be 32 bit aligned.*/
void zero32(void* p_buffer, size_type p_size);


}
}

#include <tt/mem/util.inl>

#endif // !defined(INC_TT_MEM_UTIL_H)
