// sockstream.C -*- C++ -*- socket library
// Copyright (C) 2002 Herbert Straub for my changes, see ChangeLog.
//
// Copyright (C) 1992-1996 Gnanasekaran Swaminathan <gs4t@virginia.edu>
//
// Permission is granted to use at your own risk and distribute this software
// in source and  binary forms provided  the above copyright notice and  this
// paragraph are  preserved on all copies.  This software is provided "as is"
// with no express or implied warranty.
//
// Version: 12Jan97 1.11
//
// You can simultaneously read and write into
// a sockbuf just like you can listen and talk
// through a telephone. Hence, the read and the
// write buffers are different. That is, they do not
// share the same memory.
// 
// Read:
// gptr() points to the start of the get area.
// The unread chars are gptr() - egptr().
// base() points to the read buffer
// 
// eback() is set to base() so that pbackfail()
// is called only when there is no place to
// putback a char. And pbackfail() always returns EOF.
// 
// Write:
// pptr() points to the start of the put area
// The unflushed chars are pbase() - pptr()
// pbase() points to the write buffer.
// epptr() points to the end of the write buffer.
// 
// Output is flushed whenever one of the following conditions
// holds:
// (1) pptr() == epptr()
// (2) EOF is written
// (3) linebuffered and '\n' is written
// 
// Unbuffered:
// Input buffer size is assumed to be of size 1 and output
// buffer is of size 0. That is, egptr() <= base()+1 and
// epptr() == pbase().
//
// Version: 1.2 2002-07-25 Herbert Straub 
// 	Improved Error Handling - extending the sockerr class by cOperation

using namespace std;

#include "config.h"
#include "sockstream.h"
#ifdef HAVE_SSTREAM
#include <sstream>
#else
#include <strstream>
#endif
#include <string>
#if defined(__APPLE__)
#typedef int socklen_t;
#endif

#ifndef WIN32
	EXTERN_C_BEGIN
#	include <sys/time.h>
#	include <sys/socket.h>
#	include <sys/ioctl.h>
#	include <unistd.h>
#	include <errno.h>
	EXTERN_C_END
#else
#	define socklen_t int

#	define EWOULDBLOCK			WSAEWOULDBLOCK
#	define EINPROGRESS			WSAEINPROGRESS
#	define EALREADY					WSAEALREADY
#	define ENOTSOCK					WSAENOTSOCK
#	define EDESTADDRREQ			WSAEDESTADDRREQ
#	define EMSGSIZE					WSAEMSGSIZE
#	define EPROTOTYPE				WSAEPROTOTYPE
#	define ENOPROTOOPT			WSAENOPROTOOPT
#	define EPROTONOSUPPORT	WSAEPROTONOSUPPORT
#	define ESOCKTNOSUPPORT	WSAESOCKTNOSUPPORT
#	define EOPNOTSUPP				WSAEOPNOTSUPP
#	define EPFNOSUPPORT			WSAEPFNOSUPPORT
#	define EAFNOSUPPORT			WSAEAFNOSUPPORT
#	define EADDRINUSE				WSAEADDRINUSE
#	define EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define ENETDOWN					WSAENETDOWN
#	define ENETUNREACH			WSAENETUNREACH
#	define ENETRESET				WSAENETRESET
#	define ECONNABORTED			WSAECONNABORTED
#	define ECONNRESET				WSAECONNRESET
#	define ENOBUFS					WSAENOBUFS
#	define EISCONN					WSAEISCONN
#	define ENOTCONN					WSAENOTCONN
#	define ESHUTDOWN				WSAESHUTDOWN
#	define ETOOMANYREFS			WSAETOOMANYREFS
#	define ETIMEDOUT				WSAETIMEDOUT
#	define ECONNREFUSED			WSAECONNREFUSED
#	define ELOOP						WSAELOOP
#	define EHOSTDOWN				WSAEHOSTDOWN
#	define EHOSTUNREACH			WSAEHOSTUNREACH
#	define EPROCLIM					WSAEPROCLIM
#	define EUSERS						WSAEUSERS
#	define EDQUOT						WSAEDQUOT
#	define EISCONN					WSAEISCONN
#	define ENOTCONN					WSAENOTCONN
#	define ECONNRESET				WSAECONNRESET
#	define ECONNREFUSED			WSAECONNREFUSED
#	define ETIMEDOUT				WSAETIMEDOUT
#	define EADDRINUSE				WSAEADDRINUSE
#	define EADDRNOTAVAIL		WSAEADDRNOTAVAIL
#	define EWOULDBLOCK			WSAEWOULDBLOCK


#endif // !WIN32

#ifndef BUFSIZ
#  define BUFSIZ 1024
#endif

#ifdef FD_ZERO
#  undef FD_ZERO    // bzero causes so much trouble to us
#endif
#define FD_ZERO(p) (memset ((p), 0, sizeof *(p)))

const char* sockerr::errstr () const
{
#ifndef WIN32
  return strerror(err);
#else
	return 0; // TODO
#endif
}

bool sockerr::io () const
// recoverable io error.
{
  switch (err) {
  case EWOULDBLOCK:
  case EINPROGRESS:
  case EALREADY:
    return true;
  }
  return false;
}

bool sockerr::arg () const
// recoverable argument error.
{
  switch (err) {
  case ENOTSOCK:
  case EDESTADDRREQ:
  case EMSGSIZE:
  case EPROTOTYPE:
  case ENOPROTOOPT:
  case EPROTONOSUPPORT:
  case ESOCKTNOSUPPORT:
  case EOPNOTSUPP:
  case EPFNOSUPPORT:
  case EAFNOSUPPORT:
  case EADDRINUSE:
  case EADDRNOTAVAIL:
    return true;
  }
  return false;
}

bool sockerr::op () const
// operational error encountered 
{
  switch (err) {
  case ENETDOWN:
  case ENETUNREACH:
  case ENETRESET:
  case ECONNABORTED:
  case ECONNRESET:
  case ENOBUFS:
  case EISCONN:
  case ENOTCONN:
  case ESHUTDOWN:
  case ETOOMANYREFS:
  case ETIMEDOUT:
  case ECONNREFUSED:
  case ELOOP:
  case ENAMETOOLONG:
  case EHOSTDOWN:
  case EHOSTUNREACH:
  case ENOTEMPTY:
#	if !defined(__linux__) // LN
  case EPROCLIM:
#	endif
  case EUSERS:
  case EDQUOT:
    return true;
  }
  return false;
}

bool sockerr::conn () const
// return true if err is EISCONN, ENOTCONN, ECONNRESET, ECONNREFUSED,
// ETIMEDOUT, or EPIPE
{
  switch (err) {
  case EISCONN:
  case ENOTCONN:
  case ECONNRESET:
  case ECONNREFUSED:
  case ETIMEDOUT:
	case EPIPE:
    return true;
  }
  return false;
}

bool sockerr::addr () const
// return true if err is EADDRINUSE or EADDRNOTAVAIL
{
  switch (err) {
  case EADDRINUSE:
  case EADDRNOTAVAIL:
    return true;
  }
  return false;
}

bool sockerr::benign () const
// return true if err is EINTR, EWOULDBLOCK, or EAGAIN
{
  switch (err) {
  case EINTR:
  case EWOULDBLOCK:
// On FreeBSD (and probably on Linux too) 
// EAGAIN has the same value as EWOULDBLOCK
#if !defined(__linux__) && !(defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__bsdi__) || defined(__APPLE__)) // LN
  case EAGAIN:
#endif
    return true;
  }
  return false;
}

sockbuf::sockbuf (const sockbuf::sockdesc& sd)
//  : rep (new sockbuf::sockcnt (sd.sock))
{
  rep = new sockbuf::sockcnt (sd.sock);
  char_type* gbuf = new char_type [BUFSIZ];
  char_type* pbuf = new char_type [BUFSIZ];
  setg (gbuf, gbuf + BUFSIZ, gbuf + BUFSIZ);
  setp (pbuf, pbuf + BUFSIZ);
  rep->gend = gbuf + BUFSIZ;
  rep->pend = pbuf + BUFSIZ;
}	   

sockbuf::sockbuf (int domain, sockbuf::type st, int proto)
  : rep (0)
{
  SOCKET soc = ::socket (domain, st, proto);
  
  if (soc == SOCKET_ERROR)
#ifndef WIN32
    throw sockerr (errno, "sockbuf::sockbuf");
#else
		throw sockerr(WSAGetLastError(), "sockbuf::sockbuf");
#endif

  rep = new sockbuf::sockcnt (soc);
  
  char_type* gbuf = new char_type [BUFSIZ];
  char_type* pbuf = new char_type [BUFSIZ];
  setg (gbuf, gbuf + BUFSIZ, gbuf + BUFSIZ);
  setp (pbuf, pbuf + BUFSIZ);
  rep->gend = gbuf + BUFSIZ;
  rep->pend = pbuf + BUFSIZ;
}

sockbuf::sockbuf (const sockbuf& sb)
:
//streambuf (sb),
rep (sb.rep)
{
  // the streambuf::streambuf (const streambuf&) is assumed
  // to haved handled pbase () and gbase () correctly.

  rep->cnt++; 
}

/*sockbuf& sockbuf::operator = (const sockbuf& sb)
{
  if (this != &sb && rep != sb.rep && rep->sock != sb.rep->sock) {
    streambuf::operator = (sb);
    this->sockbuf::~sockbuf();

    // the streambuf::operator = (const streambuf&) is assumed
    // to have handled pbase () and gbase () correctly.
    rep  = sb.rep;
    rep->cnt++;
  }
  return *this;
}*/

sockbuf::~sockbuf ()
{
  overflow (eof); // flush write buffer
  if (--rep->cnt == 0) {
    delete [] pbase ();
    delete [] eback ();
#ifndef WIN32
    int c = close (rep->sock);
#else
		int c = closesocket(rep->sock);
#endif
    delete rep;
    if (c == SOCKET_ERROR) 
#ifndef WIN32
			throw sockerr (errno, "sockbuf::~sockbuf", sockname.c_str());
#else
			throw sockerr(WSAGetLastError(), "sockbuf::~sockbuf", sockname.c_str());
#endif
  }
}

bool sockbuf::is_open () const
// if socket is still connected to the peer, return true
// else return false
{
  return false;
}

int sockbuf::sync ()
// we never return -1 because we throw sockerr
// exception in the event of an error.
{
  if (pptr () && pbase () < pptr () && pptr () <= epptr ()) {
    // we have some data to flush
    try {
      write (pbase (), pptr () - pbase ());
    }
    catch (int wlen) {
      // write was not completely successful
#ifdef HAVE_SSTREAM
      stringstream sb;
#else
      strstream sb;
#endif
      string err ("sockbuf::sync");
      err += "(" + sockname + ")";
      if (wlen) {
	// reposition unwritten chars
	char* pto = pbase ();
	char* pfrom = pbase () + wlen;
	int len = pptr () - pbase () - wlen;
	while (pfrom < pptr ()) *pto++ = *pfrom++;
	setp (pbase (), (char_type*) rep->pend);
	pbump (len);
	sb << " wlen=(" << wlen << ")";
	err += sb.rdbuf()->str();
      }
      throw sockerr (errno, err.c_str ());
    }

    setp (pbase (), (char_type*) rep->pend);
  }

  // we cannot restore input data back to the socket stream
  // thus we do not do anything on the input stream

  return 0;
}

int sockbuf::showmanyc () const
// return the number of chars in the input sequence
{
  if (gptr () && gptr () < egptr ())
    return egptr () - gptr ();
  return 0;
}

sockbuf::int_type sockbuf::underflow ()
{
  if (gptr () == 0)
    return eof; // input stream has been disabled

  if (gptr () < egptr ())
    return (unsigned char) *gptr (); // eof is a -ve number; make it
                                     // unsigned to be diff from eof

  int rlen = read (eback (), (char*) rep->gend - (char*) eback ());

  if (rlen == 0)
    return eof;

  setg (eback (), eback (), eback () + rlen);
  return (unsigned char) *gptr ();
}

sockbuf::int_type sockbuf::uflow ()
{
  int_type ret = underflow ();
  if (ret == eof)
    return eof;

  gbump (1);
  return ret;
}

streamsize sockbuf::xsgetn (char_type* s, streamsize n)
{
  int rval = showmanyc ();
  if (rval >= n) {
    memcpy (s, gptr (), n * sizeof (char_type));
    gbump (n);
    return n;
  }

  memcpy (s, gptr (), rval * sizeof (char_type));
  gbump (rval);

  if (underflow () != eof)
    return rval + xsgetn (s + rval, n - rval);
  
  return rval;
}

sockbuf::int_type sockbuf::pbackfail (int c)
{
  return eof;
}

sockbuf::int_type sockbuf::overflow (sockbuf::int_type c)
// if pbase () == 0, no write is allowed and thus return eof.
// if c == eof, we sync the output and return 0.
// if pptr () == epptr (), buffer is full and thus sync the output,
//                         insert c into buffer, and return c.
// In all cases, if error happens, throw exception.
{
  if (pbase () == 0)
    return eof;

  if (c == eof)
    return sync ();

  if (pptr () == epptr ())
    sync ();
  *pptr () = (char_type)c;
  pbump (1);
  return c;
}

streamsize sockbuf::xsputn (const char_type* s, streamsize n)
{
  int wval = epptr () - pptr ();
  if (n <= wval) {
    memcpy (pptr (), s, n * sizeof (char_type));
    pbump (n);
    return n;
  }

  memcpy (pptr (), s, wval * sizeof (char_type));
  pbump (wval);
  
  if (overflow () != eof)
    return wval + xsputn (s + wval, n - wval);

  return wval;
}

void sockbuf::bind (sockAddr& sa)
{
  if (::bind (rep->sock, sa.addr (), sa.size ()) == -1)
    throw sockerr (errno, "sockbuf::bind", sockname.c_str());
}

void sockbuf::connect (sockAddr& sa)
{
  if (::connect(rep->sock, sa.addr (), sa.size()) == -1)
    throw sockerr (errno, "sockbuf::connect", sockname.c_str());
}

void sockbuf::listen (int num)
{
  if (::listen (rep->sock, num) == -1)
    throw sockerr (errno, "sockbuf::listen", sockname.c_str());
}

sockbuf::sockdesc sockbuf::accept (sockAddr& sa)
{
  int len = sa.size ();
  int soc = -1;
  if ((soc = ::accept (rep->sock, sa.addr (), (socklen_t*) // LN
                       &len)) == -1)
    throw sockerr (errno, "sockbuf::sockdesc", sockname.c_str());
  return sockdesc (soc);
}

sockbuf::sockdesc sockbuf::accept ()
{
  int soc = -1;
  if ((soc = ::accept (rep->sock, 0, 0)) == -1)
    throw sockerr (errno, "sockbuf::sockdesc", sockname.c_str());
  return sockdesc (soc);
}

int sockbuf::read (void* buf, int len)
{
  if (rep->rtmo != -1 && is_readready (rep->rtmo)==0) {
    throw sockerr (ETIMEDOUT, "sockbuf::read", sockname.c_str());
  }
  
  if (rep->oob && atmark ())
    throw sockoob ();

  int rval = 0;
  if ((rval = ::read (rep->sock, (char*) buf, len)) == -1)
    throw sockerr (errno, "sockbuf::read", sockname.c_str());
  return rval;
}

int sockbuf::recv (void* buf, int len, int msgf)
{
  if (rep->rtmo != -1 && is_readready (rep->rtmo)==0)
    throw sockerr (ETIMEDOUT, "sockbuf::recv", sockname.c_str());
  
  if (rep->oob && atmark ())
    throw sockoob ();

  int rval = 0;
  if ((rval = ::recv (rep->sock, (char*) buf, len, msgf)) == -1)
    throw sockerr (errno, "sockbuf::recv", sockname.c_str());
  return rval;
}

int sockbuf::recvfrom (sockAddr& sa, void* buf, int len, int msgf)
{
  if (rep->rtmo != -1 && is_readready (rep->rtmo)==0)
    throw sockerr (ETIMEDOUT, "sockbuf::recvfrom", sockname.c_str());
  
  if (rep->oob && atmark ())
    throw sockoob ();

  int rval = 0;
  int sa_len = sa.size ();
  
  if ((rval = ::recvfrom (rep->sock, (char*) buf, len,
                          msgf, sa.addr (), (socklen_t*) // LN
                          &sa_len)) == -1)
    throw sockerr (errno, "sockbuf::recvfrom", sockname.c_str());
  return rval;
}

int sockbuf::write(const void* buf, int len)
// upon error, write throws the number of bytes writen so far instead
// of sockerr.
{
  char *pbuf = (char*) buf;
  if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
    throw sockerr (ETIMEDOUT, "sockbuf::write", sockname.c_str());
  
  int wlen=0;
  while(len>0) {
    int	wval = ::write (rep->sock, pbuf+wlen, len);
    if (wval == -1) throw wlen;
    len -= wval;
    wlen += wval;
  }
  return wlen; // == len if every thing is all right
}

int sockbuf::send (const void* buf, int len, int msgf)
// upon error, write throws the number of bytes writen so far instead
// of sockerr.
{
  char *pbuf = (char *) buf;
  if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
    throw sockerr (ETIMEDOUT, "sockbuf::send", sockname.c_str());
  
  int wlen=0;
  while(len>0) {
    int	wval = ::send (rep->sock, pbuf+wlen, len, msgf);
    if (wval == -1) throw wlen;
    len -= wval;
    wlen += wval;
  }
  return wlen;
}

int sockbuf::sendto (sockAddr& sa, const void* buf, int len, int msgf)
// upon error, write throws the number of bytes writen so far instead
// of sockerr.
{
  char *pbuf = (char *) buf;
  if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
    throw sockerr (ETIMEDOUT, "sockbuf::sendto", sockname.c_str());
  
  int wlen=0;
  while(len>0) {
    int	wval = ::sendto (rep->sock, pbuf+wlen, len, msgf,
			 sa.addr (), sa.size());
    if (wval == -1) throw wlen;
    len -= wval;
    wlen += wval;
  }
  return wlen;
}

#if	!defined(__linux__) && !defined(WIN32)
// does not have sendmsg or recvmsg

int sockbuf::recvmsg (msghdr* msg, int msgf)
{
  if (rep->rtmo != -1 && is_readready (rep->rtmo)==0)
    throw sockerr (ETIMEDOUT, "sockbuf::recvmsg", sockname.c_str());
  
  if (rep->oob && atmark ())
    throw sockoob ();

  int rval = ::recvmsg(rep->sock, msg, msgf);
  if (rval == -1) throw sockerr (errno, "sockbuf::recvmsg", sockname.c_str());
  return rval;
}

int sockbuf::sendmsg (msghdr* msg, int msgf)
// upon error, write throws the number of bytes writen so far instead
// of sockerr.
{
  if (rep->stmo != -1 && is_writeready (rep->stmo)==0)
    throw sockerr (ETIMEDOUT, "sockbuf::sendmsg", sockname.c_str());
  
  int wlen = ::sendmsg (rep->sock, msg, msgf);
  if (wlen == -1) throw 0;
  return wlen;
}
#endif // !__linux__ && !WIN32

int sockbuf::sendtimeout (int wp)
{
  int oldstmo = rep->stmo;
  rep->stmo = (wp < 0) ? -1: wp;
  return oldstmo;
}

int sockbuf::recvtimeout (int wp)
{
  int oldrtmo = rep->rtmo;
  rep->rtmo = (wp < 0) ? -1: wp;
  return oldrtmo;
}

int sockbuf::is_readready (int wp_sec, int wp_usec) const
{
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (rep->sock, &fds);
  
  timeval tv;
  tv.tv_sec  = wp_sec;
  tv.tv_usec = wp_usec;
  
  int ret = select (rep->sock+1, &fds, 0, 0, (wp_sec == -1) ? 0: &tv);
  if (ret == -1) throw sockerr (errno, "sockbuf::is_readready", sockname.c_str());
  return ret;
}

int sockbuf::is_writeready (int wp_sec, int wp_usec) const
{
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET (rep->sock, &fds);
  
  timeval tv;
  tv.tv_sec  = wp_sec;
  tv.tv_usec = wp_usec;
  
  int ret = select (rep->sock+1, 0, &fds, 0, (wp_sec == -1) ? 0: &tv);
  if (ret == -1) throw sockerr (errno, "sockbuf::is_writeready", sockname.c_str());
  return ret;
}

int sockbuf::is_exceptionpending (int wp_sec, int wp_usec) const
{
  fd_set fds;
  FD_ZERO (&fds);
  FD_SET  (rep->sock, &fds);
  
  timeval tv;
  tv.tv_sec = wp_sec;
  tv.tv_usec = wp_usec;
  
  int ret = select (rep->sock+1, 0, 0, &fds, (wp_sec == -1) ? 0: &tv);
  if (ret == -1) throw sockerr (errno, "sockbuf::is_exceptionpending", sockname.c_str());
  return ret;
}

void sockbuf::shutdown (shuthow sh)
{
  switch (sh) {
  case shut_read:
    delete [] eback ();
    setg (0, 0, 0);
    break;
  case shut_write:
    delete [] pbase ();
    setp (0, 0);
    break;
  case shut_readwrite:
    shutdown (shut_read);
    shutdown (shut_write);
    break;
  }
  if (::shutdown(rep->sock, sh) == -1) throw sockerr (errno, "sockbuf::shutdown", sockname.c_str());
}

int sockbuf::getopt (int op, void* buf, int len, int level) const
{
    if (::getsockopt (rep->sock, level, op, (char*) buf, (socklen_t*) // LN
                      &len) == -1)
    throw sockerr (errno, "sockbuf::getopt", sockname.c_str());
  return len;
}

void sockbuf::setopt (int op, void* buf, int len, int level) const
{
  if (::setsockopt (rep->sock, level, op, (char*) buf, len) == -1)
    throw sockerr (errno, "sockbuf::setopt", sockname.c_str());
}

sockbuf::type sockbuf::gettype () const
{
  int ty=0;
  getopt (so_type, &ty, sizeof (ty));
  return sockbuf::type(ty);
}

int sockbuf::clearerror () const
{
  int err=0;
  getopt (so_error, &err, sizeof (err));
  return err;
}

bool sockbuf::debug () const
{
  int old = 0;
  getopt (so_debug, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::debug (bool set) const
{
  int old=0;
  int opt = set;
  getopt (so_debug, &old, sizeof (old));
  setopt (so_debug, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::reuseaddr () const
{
  int old = 0;
  getopt (so_reuseaddr, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::reuseaddr (bool set) const
{
  int old=0;
  int opt = set;
  getopt (so_reuseaddr, &old, sizeof (old));
  setopt (so_reuseaddr, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::keepalive () const
{
  int old = 0;
  getopt (so_keepalive, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::keepalive (bool set) const
{
  int old=0;
  int opt = set;
  getopt (so_keepalive, &old, sizeof (old));
  setopt (so_keepalive, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::dontroute () const
{
  int old = 0;
  getopt (so_dontroute, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::dontroute (bool set) const
{
  int old = 0;
  int opt = set;
  getopt (so_dontroute, &old, sizeof (old));
  setopt (so_dontroute, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::broadcast () const
{
  int old=0;
  getopt (so_broadcast, &old, sizeof (old));
  return old!=0;
}

bool sockbuf::broadcast (bool set) const
{
  int old = 0;
  int opt = set;
  getopt (so_broadcast, &old, sizeof (old));
  setopt (so_broadcast, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::oobinline () const
{
  int old=0;
  getopt (so_oobinline, &old, sizeof (old));
  return old!=0;
}
    
bool sockbuf::oobinline (bool set) const
{
  int old = 0;
  int opt = set;
  getopt (so_oobinline, &old, sizeof (old));
  setopt (so_oobinline, &opt, sizeof (opt));
  return old!=0;
}

bool sockbuf::oob (bool b)
{
  bool old = rep->oob;
  rep->oob = b;
  return old;
}

sockbuf::socklinger sockbuf::linger () const
{
  socklinger old (0, 0);
  getopt (so_linger, &old, sizeof (old));
  return old;
}

sockbuf::socklinger sockbuf::linger (sockbuf::socklinger opt) const
{
  socklinger old (0, 0);
  getopt (so_linger, &old, sizeof (old));
  setopt (so_linger, &opt, sizeof (opt));
  return old;
}

int sockbuf::sendbufsz () const
{
  int old=0;
  getopt (so_sndbuf, &old, sizeof (old));
  return old;
}

int sockbuf::sendbufsz (int sz) const
{
  int old=0;
  getopt (so_sndbuf, &old, sizeof (old));
  setopt (so_sndbuf, &sz, sizeof (sz));
  return old;
}

int sockbuf::recvbufsz () const
{
  int old=0;
  getopt (so_rcvbuf, &old, sizeof (old));
  return old;
}

int sockbuf::recvbufsz (int sz) const
{
  int old=0;
  getopt (so_rcvbuf, &old, sizeof (old));
  setopt (so_rcvbuf, &sz, sizeof (sz));
  return old;
}

bool sockbuf::atmark () const
// return true, if the read pointer for socket points to an
// out of band data
{
#ifndef WIN32
	int arg;
  if (::ioctl (rep->sock, SIOCATMARK, &arg) == -1)
    throw sockerr (errno, "sockbuf::atmark", sockname.c_str());
#else
	unsigned long arg = 0;
  if (::ioctlsocket(rep->sock, SIOCATMARK, &arg) == SOCKET_ERROR)
    throw sockerr (WSAGetLastError(), "sockbuf::atmark", sockname.c_str());
#endif // !WIN32
  return arg!=0;
}

#ifndef WIN32
int sockbuf::pgrp () const
// return the process group id that would receive SIGIO and SIGURG
// signals
{
  int arg;
  if (::ioctl (rep->sock, SIOCGPGRP, &arg) == -1)
    throw sockerr (errno, "sockbuf::pgrp", sockname.c_str());
  return arg;
}

int sockbuf::pgrp (int new_pgrp) const
// set the process group id that would receive SIGIO and SIGURG signals.
// return the old pgrp
{
  int old = pgrp ();
  if (::ioctl (rep->sock, SIOCSPGRP, &new_pgrp) == -1)
    throw sockerr (errno, "sockbuf::pgrp", sockname.c_str());
  return old;
}

void sockbuf::closeonexec (bool set) const
// if set is true, set close on exec flag
// else clear close on exec flag
{
  if (set) {
    if (::ioctl (rep->sock, FIOCLEX, 0) == -1)
      throw sockerr (errno, "sockbuf::closeonexec", sockname.c_str());
  } else {
    if (::ioctl (rep->sock, FIONCLEX, 0) == -1)
      throw sockerr (errno, "sockbuf::closeonexec", sockname.c_str());
  }
}
#endif // !WIN32

long sockbuf::nread () const
// return how many chars are available for reading in the recvbuf of
// the socket.
{
	long arg;
#ifndef WIN32  
  if (::ioctl (rep->sock, FIONREAD, &arg) == -1)
    throw sockerr (errno, "sockbuf::nread", sockname.c_str());
#else
	if (::ioctlsocket (rep->sock, FIONREAD, (unsigned long *) &arg) == SOCKET_ERROR)
    throw sockerr (WSAGetLastError(), "sockbuf::nread", sockname.c_str());
#endif // !WIN32
  return arg;
}

long sockbuf::howmanyc () const
// return how many chars are available for reading in the input buffer
// and the recvbuf of the socket.
{
  return showmanyc () + nread ();
}

void sockbuf::nbio (bool set) const
// if set is true, set socket to non-blocking io. Henceforth, any
// write or read operation will not wait if write or read would block.
// The read or write operation will result throwing a sockerr
// exception with errno set to  EWOULDBLOCK.
{
#ifndef WIN32
  int arg = set;
  if (::ioctl (rep->sock, FIONBIO, &arg) == -1)
    throw sockerr (errno, "sockbuf::nbio", sockname.c_str());
#else
  unsigned long arg = (set)?1:0;
  if (::ioctlsocket (rep->sock, FIONBIO, &arg) == -1)
    throw sockerr (WSAGetLastError(), "sockbuf::nbio", sockname.c_str());
#endif // !WIN32
}

#ifndef WIN32
void sockbuf::async (bool set) const
// if set is true, set socket for asynchronous io. If any io is
// possible on the socket, the process will get SIGIO
{
  int arg = set;
  if (::ioctl (rep->sock, FIOASYNC, &arg) == -1)
    throw sockerr (errno, "sockbuf::async", sockname.c_str());
}
#endif // !WIN32

osockstream& crlf (osockstream& o)
{
  o << "\r\n";
  o.rdbuf ()->pubsync ();
  return o;
}

osockstream& lfcr (osockstream& o)
{
  o << "\n\r";
  o.rdbuf ()->pubsync ();
  return o;
}
