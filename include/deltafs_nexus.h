/*
 * Copyright (c) 2016-2017 Carnegie Mellon University.
 *
 * All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. See the AUTHORS file for names of contributors.
 */

#pragma once

#include <map>
#include <mercury.h>
using namespace std;

/*
 * Nexus library context
 */
struct nexus_ctx
{
    int myrank;                       /* my MPI rank */
    int localrank;                    /* my local MPI rank */
    int reprank;                      /* my representative's rank */

    int ranksize;                     /* total number of ranks */
    int localsize;                    /* number of local ranks (in my node) */

    int *replist;                     /* array of rank -> representative */
    map<int, hg_addr_t *> lcladdrs;   /* map of local rank -> Hg address */
    map<int, hg_addr_t *> rmtaddrs;   /* map of remote rank -> Hg address */

    /* Mercury endpoint state */
    hg_class_t *remote_hgcl;          /* Remote Hg class */
    hg_context_t *remote_hgctx;       /* Remote Hg context */
    hg_class_t *local_hgcl;           /* Local Hg class */
    hg_context_t *local_hgctx;        /* Local Hg context */
};

typedef struct nexus_ctx nexus_ctx_t;

/*
 * Bootstraps all MPI ranks with the Nexus library
 */
int nexus_bootstrap(nexus_ctx_t *nctx,
                    int minport,
                    int maxport,
                    char *subnet,
                    char *proto);

/*
 * Destroys the Nexus library freeing all allocated resources
 */
int nexus_destroy(nexus_ctx_t *nctx);

/*
 * Returns true if the rank is local to the caller
 */
bool nexus_is_local(nexus_ctx_t *nctx, int rank);

/*
 * Returns the representative MPI rank (in COMM_WORLD) in the node
 * with the provided rank
 */
int nexus_get_rep(nexus_ctx_t *nctx, int rank);

/*
 * Returns the Mercury address of the provided rank
 */
int nexus_get_addr(nexus_ctx_t *nctx, int rank, hg_addr_t *addr);
