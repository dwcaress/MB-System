///
/// @file mbrt-net.c
/// @authors k. Headley
/// @date 01 jan 2018
 
/// MBRT platform-independent socket wrappers

/////////////////////////
// Terms of use 
/////////////////////////
/*
Copyright Information

Copyright 2000-2018 MBARI
Monterey Bay Aquarium Research Institute, all rights reserved.
 
Terms of Use

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version. You can access the GPLv3 license at
http://www.gnu.org/licenses/gpl-3.0.html

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details 
(http://www.gnu.org/licenses/gpl-3.0.html)
 
 MBARI provides the documentation and software code "as is", with no warranty,
 express or implied, as to the software, title, non-infringement of third party 
 rights, merchantability, or fitness for any particular purpose, the accuracy of
 the code, or the performance or results which you may obtain from its use. You 
 assume the entire risk associated with use of the code, and you agree to be 
 responsible for the entire cost of repair or servicing of the program with 
 which you are using the code.
 
 In no event shall MBARI be liable for any damages, whether general, special,
 incidental or consequential damages, arising out of your use of the software, 
 including, but not limited to, the loss or corruption of your data or damages 
 of any kind resulting from use of the software, any prohibited use, or your 
 inability to use the software. You agree to defend, indemnify and hold harmless
 MBARI and its officers, directors, and employees against any claim, loss, 
 liability or expense, including attorneys' fees, resulting from loss of or 
 damage to property or the injury to or death of any person arising out of the 
 use of the software.
 
 The MBARI software is provided without obligation on the part of the 
 Monterey Bay Aquarium Research Institute to assist in its use, correction, 
 modification, or enhancement.
 
 MBARI assumes no responsibility or liability for any third party and/or 
 commercial software required for the database or applications. Licensee agrees 
 to obtain and maintain valid licenses for any additional third party software 
 required.
*/
/////////////////////////
// Headers 
/////////////////////////
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>

/////////////////////////
// Macros
/////////////////////////

// These macros should only be defined for 
// application main files rather than general C files
/*
/// @def PRODUCT
/// @brief header software product name
#define PRODUCT "MBRT"

/// @def COPYRIGHT
/// @brief header software copyright info
#define COPYRIGHT "Copyright 2002-2013 MBARI Monterey Bay Aquarium Research Institute, all rights reserved."
/// @def NOWARRANTY
/// @brief header software terms of use
#define NOWARRANTY  \
"This program is distributed in the hope that it will be useful,\n"\
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"\
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"\
"GNU General Public License for more details (http://www.gnu.org/licenses/gpl-3.0.html)\n"
*/

/////////////////////////
// Declarations 
/////////////////////////

/////////////////////////
// Imports
/////////////////////////

/////////////////////////
// Module Global Variables
/////////////////////////

/////////////////////////
// Function Definitions
/////////////////////////

/// @fn int listen_posix()
/// @brief TBD.
/// @return TBD
static int listen_posix()
{
    return -1;
}
// End function listen_posix

/// @fn int connect_posix()
/// @brief TBD.
/// @return TBD
static int connect_posix()
{
    return -1;
}
// End function connect_posix


/// @fn int listen_win()
/// @brief TBD.
/// @return TBD
static int listen_win()
{
    return -1;
}
// End function listen_win

/// @fn int connect_win()
/// @brief TBD.
/// @return TBD
static int connect_win()
{
    return -1;
}
// End function connect_win


// socket Allocate a socket descriptor
// bind Associate a socket with an IP address and port number
// listen Tell a socket to listen for incoming connections
// accept Accept an incoming connection on a listening socket
// connect Connect a socket to a server
// close Close a socket descriptor
// setsockopt(), getsockopt() Set various options for a socket
// recv(), recvfrom() Receive data on a socket
// send(), sendto() Send data out over a socket
//  getaddrinfo(), freeaddrinfo(), gai_strerror(), Get information about a host name and/or service and load up a struct sockaddr with the result
// getnameinfo Look up the host name and service name information for a given struct sockaddr.
// gethostname Returns the name of the system
// gethostbyname, gethostbyaddr Get an IP address for a hostname, or vice-versa
// getpeername Return address info about the remote side of the connection
// fcntl Control socket descriptors
// htons(), htonl(), ntohs(), ntohl() Convert multi-byte integer types from host byte order to network byte order
// inet_ntoa(), inet_aton(), inet_addr Convert IP addresses from a dots-and-number string to a struct in_addr and back
// inet_ntop(), inet_pton() Convert IP addresses to human-readable form and back.
// poll Test for events on multiple sockets simultaneously
// select Check if sockets descriptors are ready to read/write
// perror(), strerror() Print an error as a human-readable string


/// @fn int listen()
/// @brief TBD.
/// @return TBD
int listen()
{}
// End function listen


/// @fn int connect()
/// @brief TBD.
/// @return TBD
int connect()
{}
// End function connect


