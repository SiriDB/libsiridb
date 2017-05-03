/*
 * uvwrap.h
 *
 *  Created on: Apr 07, 2017
 *      Author: Jeroen van der Heijden <jeroen@transceptor.technology>
 */

#ifndef SIRIDB_UVWRAP_H_
#define SIRIDB_UVWRAP_H_

#include <uv.h>
#include <siridb.h>

int uvwrap_init(siridb_t * conn, uv_loop_t * loop);
int uvwrap_handle_init(siridb_handle_t * handle);

#endif /* SIRIDB_UVWRAP_H_ */
