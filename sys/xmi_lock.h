/**
 * \file xmi_lock.h
 * \brief xmi application lock interface
 * \defgroup lock Application context locks
 * \{
 */
#ifndef __xmi_lock_h__
#define __xmi_lock_h__

#include "xmi.h"

/** \brief XMI lock data type */
typedef size_t xmi_lock_t;

/**
 * \brief Create an atomic lock.
 *
 * \note Does the lock need to be an opaque object, or can it be a handle
 *       to some internal structure? Which is faster?
 *
 * \param[out] realm Application realm
 * \param[out] lock  XMI lock
 */
xmi_result_t XMI_Lock_create (xmi_application_t   realm,
                              xmi_lock_t        * lock);

/**
 * \brief Attempt to acquire the XMI lock
 *
 * May return \c EAGAIN if the lock was not acquired.
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_try (xmi_lock_t lock);

/**
 * \brief Acquire the XMI lock
 *
 * Block until the lock is aquired.
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_acquire (xmi_lock_t lock);

/**
 * \brief Release the XMI lock
 *
 * \warning It is illegal to release a lock that has not been previously
 *          acquired.
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_release (xmi_lock_t lock);

/**
 * \brief Destroy the XMI lock
 *
 * \warning It is illegal to destroy a lock that has not been created or is
 *          currently acquired..
 *
 * \param[in] lock XMI lock
 */
xmi_result_t XMI_Lock_destroy (xmi_lock_t lock);

/**
 * \}
 * \addtogroup lock
 *
 * More documentation for lock stuff....
 */

#endif /* __xmi_lock_h__ */
