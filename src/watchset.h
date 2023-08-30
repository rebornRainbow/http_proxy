/**
 * File: watchset.h
 * ----------------
 * Exports a ProxyWatchset class that can be used to monitor one or more descriptors for
 * activity.
 */

#pragma once
#include <cstddef>

class ProxyWatchset {
 public:
/**
 * Constructor: ProxyWatchset
 * --------------------------
 * Constructs a ProxyWatchset that can be used to poll one or more
 * descriptors for data availability.  The timeout parameter specifies
 * the number of seconds the watchset is willing to wait for data to
 * become available on any of the descriptors being watched.
 */
  ProxyWatchset(size_t timeout = 5);

/**
 * Destructor: ~ProxyWatchset
 * --------------------------
 * Tears down the ProxyWatchset. The descriptors being watched are not closed, as that is
 * the responsibility of the client.
 */
  ~ProxyWatchset();

/**
 * Method: add
 * -----------
 * Adds the supplied descriptor--assumed to be attached to a resource that
 * supplies bytes--to the watchset.  If the supplied fd isn't valid, then add's
 * behavior is undefined.
 */ 
  void add(int fd);

/**
 * Method: remove
 * --------------
 * Removes the supplied descriptors from the watchset.  The descriptor is not
 * closed by the watchset before it's removed, as that's the responsibility of the
 * client.  If the supplied descriptor isn't part of the watchset, then remove's behavior
 * is undefined.
 */
  void remove(int fd);

/**
 * Method: wait
 * ------------
 * Waits for data to become available on one or more of the descriptors being watched and returns
 * one of them.  If data becomes available on more than one descriptor, then multiple calls to
 * wait will return all of them.  If the establish timeout is exceeded, or if there are any errors
 * at all, wait returns -1.
 */
  int wait() const;
  
 private:
  int watchset;
  int timeout;
  
  ProxyWatchset(const ProxyWatchset& original) = delete;
  void operator=(const ProxyWatchset& rhs) = delete;
};
