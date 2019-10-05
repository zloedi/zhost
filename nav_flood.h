// #define FLOOD_NAV_IMPLEMENTATION before #include "nav_flood.h" to unroll the implementation
// #define NAV_FOUR_WAY before #include "nav_flood.h" for four-way flood implementation

// Usage:
//
// On Init:
// int frontBufSz = NAV_FrontBufSizeInBytes( GRID_WIDTH, GRID_HEIGHT );
// navFront_t front = NAV_CreateFront( GRID_WIDTH, GRID_HEIGHT, malloc( frontBufSz ) );
// 
// On Update:
// NAV_FloodMap( origin, MAX_MOVE_RANGE, &front, GRID_WIDTH, GRID_HEIGHT, navMap, floodMap );
// ...
// NAV_TracePath( origin, target0, GRID_WIDTH, floodMap, PATH_MAX_SIZE, path, &numTraced);
// ...
// NAV_TracePath( origin, target1, GRID_WIDTH, floodMap, PATH_MAX_SIZE, path, &numTraced);
// ...
// NAV_TracePath( origin, target2, GRID_WIDTH, floodMap, PATH_MAX_SIZE, path, &numTraced);
// ...
// Read the function descriptions for more

#define NAV_FREE 0x1fffffff
#define NAV_BLOC 0x40000000

// Internal struct used to hold flood front
typedef struct {
    int capMask;
    int *buf;
    int head;
    int tail;
} navFront_t;

#ifndef FLOOD_NAV_IMPLEMENTATION

// Get nice round number for allocators
int NAV_FrontBufSizeInBytes( int gridW, int gridH );

// Assumes buffer has NAV_FrontBufSizeInBytes size
// Call once to set nav front buffer for multiple calls to flood
// Front internals get reset on each flood
navFront_t NAV_CreateFront( int gridW, int gridH, int *buffer );

// Origin is index in grid
// Assumes origin is clamped to grid size
// Assumes impassable (NAV_BLOC) map borders
// Front can be reused in the same thread
// Create nav map i.e. like this: navMap[i] = map[i] == wall ? NAV_BLOC : NAV_FREE;
void NAV_FloodMap( int origin, int maxRange, navFront_t *front, int gridW, int gridH, 
                    const int *navMap, int *floodMap );

// Call this to get a (partial) path between two nodes
// outResult contains indices in grid
// returns 1 if traced path from target to origin, 0 otherwise
int NAV_TracePath( int origin, int target, int gridW, const int *floodMap, int resultMaxElems, 
                        int *outResult, int *outResultNumElems );

#else

#include <mem.h>

static inline int Min( int a, int b ) {
    return a < b ? a : b;
}

static inline void NAV_FrontReset( navFront_t *front, int origin, int gridW, int gridH ) {
    front->head = 0;
    front->tail = 1;
    front->buf[front->head] = origin;
}

static inline int NAV_FrontPop( navFront_t *front ) {
    int coord = front->buf[front->head & front->capMask];
    front->head++;
    return coord;
}

static inline void NAV_FrontPush( navFront_t *front, int coord ) {
    front->buf[front->tail & front->capMask] = coord;
    front->tail++;
}

static inline int NAV_FrontIsEmpty( navFront_t *front ) {
    return front->head == front->tail;
}

static inline void NAV_TryExpand( navFront_t *front, int *floodMap, int nbr, int newScore ) {
    if ( ( floodMap[nbr] & ~NAV_BLOC ) > newScore ) {
        floodMap[nbr] = newScore;
        NAV_FrontPush( front, nbr );
    }
}

static int NAV_FrontCapacity( int gridW, int gridH ) {
    int sz = ( gridW + gridH ) * 2;
    int c = sz; // compute the next highest power of 2 of 32-bit c
    c--;
    c |= c >> 1;
    c |= c >> 2;
    c |= c >> 4;
    c |= c >> 8;
    c |= c >> 16;
    c++;
    return c;
}

void NAV_FloodMap( int origin, int maxRange, navFront_t *front, int gridW, int gridH, 
                    const int *navMap, int *floodMap ) {
    const int prims[] = {
        // keep them ordered
        -1, -gridW, 1, gridW,
#ifndef NAV_FOUR_WAY
        -1 - gridW, 1 - gridW, 1 + gridW, -1 + gridW,
#endif
    };
    memcpy( floodMap, navMap, gridW * gridH * sizeof( *floodMap ) );
    if ( floodMap[origin] == NAV_BLOC ) {
        return;
    }
    floodMap[origin] = 1;
    NAV_FrontReset( front, origin, gridW, gridH );
    do {
        int c = NAV_FrontPop( front );
        int headScore = floodMap[c];
        if ( headScore < maxRange * 10 ) {
            int newScore = headScore + 10;
            int nbrs[8] = {
                c + prims[0], c + prims[1], c + prims[2], c + prims[3],
#ifndef NAV_FOUR_WAY
                c + prims[4], c + prims[5], c + prims[6], c + prims[7],
#endif
            };
            NAV_TryExpand( front, floodMap, nbrs[0], newScore );
            NAV_TryExpand( front, floodMap, nbrs[1], newScore );
            NAV_TryExpand( front, floodMap, nbrs[2], newScore );
            NAV_TryExpand( front, floodMap, nbrs[3], newScore );
#ifndef NAV_FOUR_WAY
            int diagMask = 0;
            diagMask |= ( floodMap[nbrs[0]] >> 30 ) << 0;
            diagMask |= ( floodMap[nbrs[1]] >> 30 ) << 1;
            diagMask |= ( floodMap[nbrs[2]] >> 30 ) << 2;
            diagMask |= ( floodMap[nbrs[3]] >> 30 ) << 3;
            newScore = headScore + 14;
            if ( diagMask ) {
                const int ed[] = {
                    ( 1 << 1 ) | ( 1 << 0 ),
                    ( 1 << 2 ) | ( 1 << 1 ),
                    ( 1 << 3 ) | ( 1 << 2 ),
                    ( 1 << 0 ) | ( 1 << 3 ),
                };
                if ( ! ( ed[0] & diagMask ) ) NAV_TryExpand( front, floodMap, nbrs[4], newScore );
                if ( ! ( ed[1] & diagMask ) ) NAV_TryExpand( front, floodMap, nbrs[5], newScore );
                if ( ! ( ed[2] & diagMask ) ) NAV_TryExpand( front, floodMap, nbrs[6], newScore );
                if ( ! ( ed[3] & diagMask ) ) NAV_TryExpand( front, floodMap, nbrs[7], newScore );
            } else {
                NAV_TryExpand( front, floodMap, nbrs[4], newScore );
                NAV_TryExpand( front, floodMap, nbrs[5], newScore );
                NAV_TryExpand( front, floodMap, nbrs[6], newScore );
                NAV_TryExpand( front, floodMap, nbrs[7], newScore );
            }
#endif
        }
    } while ( ! NAV_FrontIsEmpty( front ) );
}

int NAV_TracePath( int origin, int target, int gridW, const int *floodMap, int resultMaxElems, 
                    int *outResult, int *outResultNumElems ) {
    if ( resultMaxElems == 0 
            || floodMap[target] == NAV_BLOC 
            || floodMap[target] == NAV_FREE 
            || floodMap[origin] == NAV_BLOC ) {
        *outResultNumElems = 0;
        return 0;
    }
    const int prims[] = {
        -1, -gridW, 1, gridW,
    };
    // explictly push target, then start at 1
    outResult[0] = target;
    int numElems;
    for ( numElems = 1; numElems < resultMaxElems; numElems++ ) {
        int neighbours[4] = {
            target + prims[0],
            target + prims[1],
            target + prims[2],
            target + prims[3],
        };
        int floods[4] = {
            floodMap[neighbours[0]],
            floodMap[neighbours[1]],
            floodMap[neighbours[2]],
            floodMap[neighbours[3]],
        };
        int scores[4] = {
            ( floods[0] & NAV_BLOC ) | ( ( floods[0] & NAV_FREE ) << 2 ) | 0,
            ( floods[1] & NAV_BLOC ) | ( ( floods[1] & NAV_FREE ) << 2 ) | 1,
            ( floods[2] & NAV_BLOC ) | ( ( floods[2] & NAV_FREE ) << 2 ) | 2,
            ( floods[3] & NAV_BLOC ) | ( ( floods[3] & NAV_FREE ) << 2 ) | 3,
        };
        int min = Min( scores[3], Min( scores[2], Min( scores[1], scores[0] ) ) );
        if ( ( min >> 2 ) >= floodMap[target]) {
            break;
        }
        target = neighbours[min & 3];
        outResult[numElems] = target;
    }
    *outResultNumElems = numElems;
    return target == origin;
}

int NAV_FrontBufSizeInBytes( int gridW, int gridH ) {
    return NAV_FrontCapacity( gridW, gridH ) * sizeof( *( ( navFront_t* ) 0 )->buf );
}

navFront_t NAV_CreateFront( int gridW, int gridH, int *buffer ) {
    navFront_t f = {0};
    f.capMask = NAV_FrontCapacity( gridW, gridH ) - 1;
    f.buf = buffer;
    return f;
}

#endif
