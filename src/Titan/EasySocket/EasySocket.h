/******************************************************************************/
/*                                                                            */
/*                                   EASYSOCKET.H                             */
/*                               WONMisc Socket Class                         */
/*                                   Include File                             */
/*                                                                            */
/******************************************************************************/

/*

  Class:             EasySocket

  Description:       This class implements methods for communicating using sockets.
                     Useful features include timeout paramaters for all blocking
                     calls, sends and receives which attempt to send/receive all
                     of the data, methods which accept address strings and port
                     numbers and themselves figure out the correct SOCKADDR
                     struct, automatic call to WSAGetLastError() when any error
                     occurs and conversion of error number to enum.

  Author:            Brian Rothstein
  Last Modified:     20 Sept 98

  Base Classes:      none

  Contained Classes: none

  Friend Classes:    none

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Contained Data Types:
    enum SocketType
      The type of socket (i.e. TCP, UDP, SPX or IPX)

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Private Data Members:
    SOCKET mSocket
      This is the socket descriptor.  When connect is called for a TCP or SPX
      socket a new descriptor is always obtained.  For a connectionless
      protocol such as UDP, a new descriptor is only obtained if the current
      one is invalid.  The descriptor is set to invalid on construction and
      when any close operation is performed.  It is also set to invalid for
      TCP if a disconnect is performed.
        
    SocketType mType
      This is the type of socket (i.e TCP, UDP, SPX or IPX [ICMP not supported yet])

    SOCKADDR mDestAddr
      This contains the destination address to which the socket is connected.
      It is filled in whenever a connect is performed.  The reason that
      getpeername() is not used is that it doesn't always seem to work.  For
      instance, after a connect, getpeername() returns a WSAENOTCONN unless
      the connection has been up for a little time (usually under a second,
      though)

    bool mConnected
      This variable is set to true in connect and it's set to false in
      disconnect and close. It DOES NOT indicate that a socket is
      successfully connected.  When it is true, the only thing that holds
      is that the connect method has been called and there hasn't been a
      subsequent call to disconnect or close.  Right now this variable
      is only used in getDestAddr, getDestAddrString, and getDestPort
      to detect if there is a destination address to which the socket is
      supposedly connected.

  Protected Data Members: None
  Public Data Members: None

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

    Constructors:
    EasySocket(SocketType theType)
      This is the main constructor that you want to use since most of the
      time you know what kind of socket you want when you declare it.  The
      constructor doesn't do much.  It doesn't actually get a new socket
      descriptor.  It just initializes the type so that when a connect or
      sendto is performed, the correct kind of socket can be opened.

    EasySocket()
      The default constructor sets the socket's type to NO_TYPE.  Before
      using the socket you must call the setType method.  This constructor
      is useful if you don't initially know what kind of socket you need.

    EasySocket(const Socket&) <-- private
      The copy constructor and assignment operator are both private, because
      we don't want the socket descriptor getting copied around and closed
      because some other copy goes out of scope.

  Destructor:
    Closes the socket descriptor if it is valid.

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Private Methods:
    Socket& operator=(const Socket&)
      The copy constructor and assignment operator are both private, because
      we don't want the socket descriptor getting copied around and closed
      because some other copy goes out of scope.

  Protected Methods: None

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Public Methods:
    void setType(SocketType theType)
      Sets the type of the socket (i.e. TCP, UDP, SPX or IPX)

    ES_ErrorType getNewDescriptor(void)
      Gets a new socket descriptor of the appropriate type.  Closes the
      old descriptor if it's valid.

    ES_ErrorType bind(int thePort)
      Binds the socket to a specific port.  A new socket descriptor is
      fetched if the current one is invalid.

      ES_ErrorType connect(const SOCKADDR &theSockAddr, int theWaitTime)
    ES_ErrorType connect(const SOCKADDR &theSockAddr, int theWaitTime, int theNumTries)
    ES_ErrorType connect(long theAddress, int thePort, int theWaitTime = 5000, int theNumTries = 1) // UDP/TCP
    ES_ErrorType connect(unsigned char theAddress[6], int thePort, int theWaitTime = 5000, int theNumTries = 1); // IPX/SPX
    ES_ErrorType connect(const string &theAddress, int thePort, int theWaitTime = 5000, int theNumTries = 1) // UDP/TCP
      Attempts to connect the socket for a specified amount of time.
      Disconnects the socket first if it's already connected.If the amount
      of time is specified as 0 then the function doesn't wait and a call
      to checkAsynchConnect must be performed to see when/if the socket
      connects.  This function uses my "patented" connection algorithm
      which attemps to connect for n seconds, then n*2 seconds, then
      n*4, ... , n*2^(m-1) seconds where n is theWaitTime and m is theNumTries.

    bool checkAsynchConnect(int theWaitTime)
      Checks to see if an asynchronous connect has succeeded.  This is
      equivalent to calling the waitForWrite method.  Checking if a
      socket is writeable is the standard way of checking if a connect
      has succeeded (see documentation for ::connect)

    bool waitForWrite(int theWaitTime)
      Waits for socket to become writeable (so there is some free buffer
      space.)

    bool waitForRead(int theWaitTime)
      Waits for socket to become readable (so there is something in the
      read buffer.)


    ES_ErrorType sendBuffer(const void *theBuf, int theLen, int *theSentLen = NULL, int theTotalTime = 1000);
      Attempts to send a buffer of data within a certain amount of time.

    ES_ErrorType recvBuffer(void *theBuf, int theLen, int *theRecvLen = NULL, int theTotalTime = 1000);
      Attempts to receive a buffer of data within a certain amount of time.

    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, const SOCKADDR &theSockAddr, int theTotalTime = 1000);
    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, long theAddr, int thePort, int theTotalTime = 1000); // UDP/TCP
    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, unsigned char theAddr[6], int thePort, int theTotalTime = 1000); // IPX/SPX
    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, const string &theAddr, int thePort, int theTotalTime = 1000); // UDP/TCP
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, SOCKADDR *theSockAddr, int *theRecvLen = NULL, int theTotalTime = 1000);
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, long *theAddr, int *thePort, int *theRecvLen = NULL, int theTotalTime = 1000); // UDP/TCP
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, unsigned char theAddr[6], int *thePort, int *theRecvLen = NULL, int theTotalTime = 1000); // IPX/SPX
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, string *theAddrString, int *thePort, int *theRecvLen = NULL, int theTotalTime = 1000); // UDP/TCP
      These functions are disallowed for TCP and SPX sockets.  For UDP and IPX sockets, they do
      exactly what sendBuffer and recvBuffer do with the destination address
      specified as a parameter.

    void disconnect()
      Disconnects a connected socket.  For TCP and SPX sockets, this also closes
      the socket descriptor since a used TCP and SPX descriptor can't be used to
      connect to a different address.

    gracefulClose(int theWaitTime)
      Shuts down the socket and waits for a specified amount of time for the
      other side to close down (This assures that all sent data has gotten
      through.)

    close()
      Closes the socket.  The close is immediate and shutdown is completed
      in the background by winsock (see closesocket and SO_DONTLINGER)  However,
      the program musn't exit before the shutdown is complete.  It's too bad
      you don't know when that is since it's in the background.  Use the
      gracefulClose method if you want to know if the socket has successfully
      shutdown.

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Accessors:
    bool isInvalid()
      Returns whether there is a socket descriptor associated with the
      socket yet.

    long getDestAddr(void); // UDP/TCP
    void getDestAddr(unsigned char theAddr[6]); // IPX/SPX
    string getDestAddrString(void); // UDP/TCP
      Returns the address to which the socket is connected.

    int getDestPort(void)
      Returns the port to which the socket is connected.

    long getLocalAddr(void); // UDP/TCP
    void getLocalAddr(unsigned char theAddr[6]); // IPX/SPX
    string getLocalAddrString(void); // UDP/TCP
      Returns the local address with which the socket is associated.
    
    int getLocalPort(void)
      Returns the local port to which the socket is bound.

    SocketType getType(void)
      Returns the type of socket (i.e. TCP or UDP)

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

  Static Methods:
    static ES_ErrorType startWinsock(void)
      Initializes winsock with a call to WSAStartup

    static ES_ErrorType stopWinsock(void)
      Shuts down winsock with a call to WSACleanup

    static ES_ErrorType WSAErrorToEnum(int theError)
      Converts an error returned by WSAGetLastError() to a Socket::ES_ErrorType
      error.

    static ES_ErrorType getSockAddrIn(SOCKADDR_IN &theSockAddr, const string &theAddress, int thePort) // UDP/TCP
      Converts an address string and port to a SOCKADDR_IN struct.

    static long getAddrFromString(const string &theAddress) // UDP/TCP
      Converts a string address to a long (in network byte order)

    static string getAddrFromLong(long theAddress) // UDP/TCP
          Converts a long address (in network byte order) to a string
*/

#ifndef __ES_SOCKET_H__
#define __ES_SOCKET_H__


#include "STRING"

#ifdef WIN32
//#define _USE_WINSOCK2_
# ifdef _USE_WINSOCK2_
#  include <winsock2.h>
# else
#  include <winsock.h>
# endif
# include <wsipx.h>
# define HAVE_IPX
typedef __int64 INT64;
#else
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <sys/ioctl.h>
# include <sys/time.h>
# include <sys/types.h>
# include <unistd.h>
# include <netdb.h>
# include <errno.h>
extern int h_errno;
typedef int SOCKET;
typedef sockaddr SOCKADDR;
typedef sockaddr_in SOCKADDR_IN;
typedef in_addr IN_ADDR;
typedef hostent HOSTENT;
typedef timeval TIMEVAL;
typedef linger LINGER;
typedef long long int INT64;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
#endif // WIN32

#include "ES_ErrorType.h"

using namespace std;

namespace WONMisc {

class EasySocket {
public:
    enum SocketType { NO_TYPE, TCP, UDP, IPX, SPX };

private:
    SOCKET mSocket;
    SocketType mType;

    SOCKADDR mDestAddr;
    bool mConnected;

private:
    EasySocket(const EasySocket&) { }
    EasySocket& operator=(const EasySocket&) { return *this; }

protected:
    static int ESGetLastError();
    static unsigned long GetTickCount();

public:
    EasySocket();
    EasySocket(SocketType theType); 
    virtual ~EasySocket();

    virtual void setType(SocketType theType);
    ES_ErrorType getNewDescriptor(void);
    ES_ErrorType bind(int thePort, bool allowReuse = false);
    ES_ErrorType connect(const SOCKADDR &theSockAddr, int theWaitTime);
    ES_ErrorType connect(const SOCKADDR &theSockAddr, int theWaitTime, int theNumTries);
    ES_ErrorType connect(long theAddress, int thePort, int theWaitTime = 5000, int theNumTries = 1); // UDP/TCP
#ifdef HAVE_IPX
    ES_ErrorType connect(unsigned char theAddress[6], int thePort, int theWaitTime = 5000, int theNumTries = 1); // IPX/SPX
#endif // HAVE_IPX
    ES_ErrorType connect(const string &theAddress, int thePort, int theWaitTime = 5000, int theNumTries = 1); // UDP/TCP

    ES_ErrorType listen(int theBacklog = 5);
    ES_ErrorType accept(EasySocket* theEasySocket, int theWaitTime);
    
    ES_ErrorType checkAsynchConnect(int theWaitTime);
    bool waitForAccept(int theWaitTime);
    bool waitForWrite(int theWaitTime);
    bool waitForRead(int theWaitTime);

    ES_ErrorType sendBuffer(const void *theBuf, int theLen, int *theSentLen = NULL, int theTotalTime = 1000);
    ES_ErrorType recvBuffer(void *theBuf, int theLen, int *theRecvLen = NULL, int theTotalTime = 1000);

    ES_ErrorType broadcastBuffer(const void *theBuf, int theLen, int thePort, int theTotalTime = 1000);

    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, const SOCKADDR &theSockAddr, int theTotalTime = 1000);
    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, long theAddr, int thePort, int theTotalTime = 1000); // UDP/TCP
#ifdef HAVE_IPX
    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, unsigned char theAddr[6], int thePort, int theTotalTime = 1000); // IPX/SPX
#endif // HAVE_IPX
    ES_ErrorType sendBufferTo(const void *theBuf, int theLen, const string &theAddr, int thePort, int theTotalTime = 1000); // UDP/TCP
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, SOCKADDR *theSockAddr, int *theRecvLen = NULL, int theTotalTime = 1000);
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, long *theAddr, int *thePort, int *theRecvLen = NULL, int theTotalTime = 1000); // UDP/TCP
#ifdef HAVE_IPX
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, unsigned char theAddr[6], int *thePort, int *theRecvLen = NULL, int theTotalTime = 1000); // IPX/SPX
#endif // HAVE_IPX
    ES_ErrorType recvBufferFrom(void *theBuf, int theLen, string *theAddrString, int *thePort, int *theRecvLen = NULL, int theTotalTime = 1000); // UDP/TCP

    ES_ErrorType shutdown(int theHow);
    void disconnect();
    ES_ErrorType gracefulClose(int theWaitTime);
    void close();
    void close(bool linger, unsigned short theLingerSeconds);


    bool isInvalid(void);

    void getDestAddr(SOCKADDR* theSockAddr);
    long getDestAddr(void); // UDP/TCP
#ifdef HAVE_IPX
    void getDestAddr(unsigned char theAddr[6]); // IPX/SPX
#endif // HAVE_IPX
    string getDestAddrString(void);
    int getDestPort(void);


    long getLocalAddr(void); // UDP/TCP
#ifdef HAVE_IPX
    void getLocalAddr(unsigned char theAddr[6]); // IPX/SPX
#endif // HAVE_IPX
    string getLocalAddrString(void);
    int getLocalPort(void);

    SocketType getType(void);

#ifdef WIN32
    ES_ErrorType asyncSelect(HWND hWnd, unsigned int wMsg, long lEvent);
#endif // WIN32

    int setOption(int level, int optname, const char* optval, int optlen);
    int getOption(int level, int optname, char* optval, int* optlen) const;

    static ES_ErrorType startWinsock(void);
    static ES_ErrorType stopWinsock(void);
    static ES_ErrorType WSAErrorToEnum(int theError);
    static ES_ErrorType getSockAddrIn(SOCKADDR_IN &theSockAddr, const string &theAddress, int thePort); // UDP/TCP
    static ES_ErrorType getSockAddrIn(SOCKADDR_IN &theSockAddr, const string &theAddressAndPort); // UDP/TCP
#ifdef HAVE_IPX
    static ES_ErrorType getSockAddrIpx(SOCKADDR_IPX &theSockAddr, const unsigned char theAddress[6], int thePort); // IPX/SPX
#endif // HAVE_IPX

    // Fast versions don't do name resolution
    static ES_ErrorType getSockAddrInFast(SOCKADDR_IN &theSockAddr, const string &theAddress, int thePort); // UDP/TCP
    static ES_ErrorType getSockAddrInFast(SOCKADDR_IN &theSockAddr, const string &theAddressAndPort); // UDP/TCP

    static void getBroadcastSockAddrIn(SOCKADDR_IN &theSockAddr, int thePort); // UDP/TCP
#ifdef HAVE_IPX
    static void getBroadcastSockAddrIpx(SOCKADDR_IPX &theSockAddr, int thePort); // IPX/SPX
#endif HAVE_IPX

    static long getAddrFromString(const string &theAddress); // UDP/TCP
    static string getAddrFromLong(long theAddress); // UDP/TCP

#ifdef HAVE_IPX
    static void getAddrFromString(unsigned char theAddr[6], const string &theAddress); // IPX/SPX
    static string getAddrFromNodeNum(unsigned const char theNodeNum[6]); // IPX/SPX
#endif // HAVE_IPX
};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// This class is used so that the user doesn't have to worry about
// initializing winsock or stopping winsock.  An instance of the class
// is simply created as a global variable in EasySocket.cpp so winsock
// is started at the start of the program and stopped at the end of
// the program.

#ifdef WIN32
class InitWinsock {
public:
    InitWinsock() { EasySocket::startWinsock(); }
    ~InitWinsock() { EasySocket::stopWinsock(); }
};
#endif // WIN32

// This is defined in EasySocketError.cpp
string ES_ErrorTypeToString(ES_ErrorType theError);


}; // end namespace WONMisc
#endif
