///
/// @file mbtrn-server.h
/// @authors k. headley
/// @date 06 nov 2012
 
/// Test server for mbtrn
/// Reads MB data from a file and writes
/// it to a socket (e.g. emulates reson 7k center source)
 
/// @sa doxygen-examples.c for more examples of Doxygen markup
 

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

// include guard
#ifndef MBTRN_SERVER_H
/// @def MBTRN_SERVER_H
/// @brief include guard
#define MBTRN_SERVER_H

/////////////////////////
// Includes 
/////////////////////////

#include <pthread.h>
#include "iowrap.h"
#include "r7kc.h"

/////////////////////////
// Type Definitions
/////////////////////////

/// @typedef struct mbtrn_server_s mbtrn_server_t
/// @brief test server structure
typedef struct mbtrn_server_s
{
    /// @var mbtrn_server_s::sock_if
    /// @brief socket interface
    iow_socket_t *sock_if;
    /// @var mbtrn_server_s::in_file
    /// @brief file interface
    iow_file_t *in_file;
    /// @var mbtrn_server_s::t
    /// @brief server thread
    iow_thread_t *t;
    /// @var mbtrn_server_s::auto_free
    /// @brief autofree file/socket resources
    bool auto_free;
    /// @var mbtrn_server_s::stop
    /// @brief stop flag (allows caller to stop server thread)
    bool stop;
}mbtrn_server_t;

/////////////////////////
// Macros
/////////////////////////

/////////////////////////
// Exports
/////////////////////////

mbtrn_server_t *mbtrn_server_new(iow_socket_t *s, iow_file_t *mb_data);
void mbtrn_server_destroy(mbtrn_server_t **pself);


// start test server to emulate reson
// using data from file
int mbtrn_server_start(mbtrn_server_t *self);
int mbtrn_server_stop(mbtrn_server_t *self);

// include guard
#endif