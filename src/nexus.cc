/*
 * Copyright (c) 2017-2019, Carnegie Mellon University and
 *     Los Alamos National Laboratory.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
 * HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 * WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "nexus_internal.h"

#include <assert.h>

/*
 * utility function
 */
namespace {
/* nx_addr_get: retrieve address from a given nexus map. return HG_ADDR_NULL if
 * no such address if found */
hg_addr_t nx_addr_get(nexus_map_t* map, int key) {
  nexus_map_t::iterator it = map->find(key);
  return (it == map->end()) ? HG_ADDR_NULL : it->second;
}
}  // namespace

nexus_ret_t nexus_next_hop(nexus_ctx_t nctx, int dest, int* rank,
                           hg_addr_t* addr) {
  int srcrep, destrep;
  int destn;
  assert(nctx != NULL);

  /* stop here if we are the final hop */
  if (nctx->grank == dest) return NX_DONE;

  /* if dest is local we return its local address */
  if ((*addr = nx_addr_get(&nctx->lmap, dest)) != HG_ADDR_NULL) {
    *rank = dest; /* the next stop is the final dest */
    /* we are either the original src, or a dest rep */
    return NX_ISLOCAL;
  }

  /* dest >> dest node */
  destn = nctx->rank2node[dest];
  /* dest node >> src rep's global rank */
  srcrep = nctx->local2global[(destn % nctx->lsize)];
  /* dest node >> dest rep's global rank */
  destrep = nctx->node2rep[destn];
#ifdef NEXUS_DEBUG
  fprintf(stderr, "NX-%d: dest=%d, destnode=%d, srcrep=%d, destrep=%d\n",
          nctx->grank, dest, destn, srcrep, destrep);
#endif

  if (nctx->grank != srcrep) {
    *addr = nx_addr_get(&nctx->lmap, srcrep);
    if (*addr == HG_ADDR_NULL) return NX_NOTFOUND;
    *rank = srcrep; /* the next stop is src rep */
    /* we are the original src */
    return NX_SRCREP;
  } else {
    *addr = nx_addr_get(&nctx->rmap, destn);
    if (*addr == HG_ADDR_NULL) return NX_NOTFOUND;
    *rank = destrep; /* the next stop is dest rep */
    /* we are the src rep */
    return NX_DESTREP;
  }
}

nexus_ret_t nexus_set_grank(nexus_ctx_t nctx, int rank) {
  nctx->grank = rank;
  return NX_SUCCESS;
}

int nexus_global_rank(nexus_ctx_t nctx) {
  assert(nctx != NULL);
  return nctx->grank;
}

int nexus_global_size(nexus_ctx_t nctx) {
  assert(nctx != NULL);
  return nctx->gsize;
}

int nexus_local_rank(nexus_ctx_t nctx) {
  assert(nctx != NULL);
  return nctx->lrank;
}

int nexus_local_size(nexus_ctx_t nctx) {
  assert(nctx != NULL);
  return nctx->lsize;
}

nexus_ret_t nexus_global_barrier(nexus_ctx_t nctx) {
  assert(nctx != NULL);
  int rv = MPI_Barrier(MPI_COMM_WORLD);
  if (rv != MPI_SUCCESS) return NX_ERROR;
  return NX_SUCCESS;
}

nexus_ret_t nexus_local_barrier(nexus_ctx_t nctx) {
  assert(nctx != NULL);
  int rv = MPI_Barrier(nctx->localcomm);
  if (rv != MPI_SUCCESS) return NX_ERROR;
  return NX_SUCCESS;
}
