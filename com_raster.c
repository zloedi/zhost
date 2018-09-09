#include "common.h"
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

static byte com_qm[4 * 8]  = {
    0x00, 0xff, 0xff, 0x00,
    0xff, 0x00, 0x00, 0xff,
    0xff, 0x00, 0x00, 0xff,
    0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

const bitmap_t com_questionBitmap = { 
    .sizeInPixels = {.x = 4, .y = 8},
    .bytesPerPixel = 1,
    .bits = com_qm,
};

static bool_t COM_PackRects( stbrp_rect *rects, int numRects, c2_t *ioAtlasSize ) {
    c2_t atlasSize = *ioAtlasSize;
    for ( int i = 0; i < 10; i++ ) {
        stbrp_context context = {0};
        int numNodes = atlasSize.x;
        stbrp_node nodes[numNodes];
        stbrp_init_target( &context, atlasSize.x, atlasSize.y, nodes, numNodes );
        if ( stbrp_pack_rects( &context, rects, numRects ) ) {
            COM_PRINTF( "Packed all atlas rects. Atlas size: %d,%d\n", atlasSize.x, atlasSize.y );
            *ioAtlasSize = atlasSize;
            return true;
        }
        atlasSize = c2Scale( atlasSize, 2 );
    }
    COM_PRINTF( "ERROR: COM_PackRects: Failed to pack atlas rects\n" );
    *ioAtlasSize = c2zero;
    return false;
} 

void COM_CopyBitmap( byte *src, c2_t srcSize, int srcBytesPerPixel, 
                    byte *dst, c2_t dstSize, c2_t copyOrigin, int dstBytesPerPixel ) {
    int spitch = srcSize.x * srcBytesPerPixel;
    int dpitch = dstSize.x * dstBytesPerPixel;
    if ( srcBytesPerPixel == dstBytesPerPixel ) {
        for ( int i = 0, y = 0; y < srcSize.y; y++ ) {
            int start = copyOrigin.x + y * dpitch;
            for ( int x = 0; x < spitch; i++, x++ ) {
                dst[start + x] = src[i];
            }
        }
    } else {
        // FIXME: implement!!
        COM_PRINTF( "ERROR: COM_CopyBitmap: unsupported bytes per pixel format mismatch\n" );
    }
}

bitmap_t COM_PackBitmaps( const bitmap_t *bitmaps, int numBitmaps, int targetBytesPerPixel, 
                            c2_t *ioPackedBitmapCoords ) {
    bitmap_t atlas = com_questionBitmap;
    stbrp_rect *rects = A_MallocZero( numBitmaps * sizeof( *rects ) );
    for ( int i = 0; i < numBitmaps; i++ ) {
        const bitmap_t *bitmap = &bitmaps[i];
        stbrp_rect r = {
            .id = i,
            .w = bitmap->sizeInPixels.x,
            .h = bitmap->sizeInPixels.y,
        };
        rects[i] = r;
    }
    atlas.sizeInPixels = c2one;
    if ( COM_PackRects( rects, numBitmaps, &atlas.sizeInPixels ) ) {
        atlas.bits = A_MallocZero( c2MulComps( atlas.sizeInPixels ) );
        for ( int i = 0; i < numBitmaps; i++ ) {
            const bitmap_t *bitmap = &bitmaps[i];
            stbrp_rect *r = &rects[i];
            c2_t originInAtlas = c2xy( r->x, r->y );
            COM_CopyBitmap( bitmap->bits, bitmap->sizeInPixels, bitmap->bytesPerPixel, 
                            atlas.bits, atlas.sizeInPixels, originInAtlas, atlas.bytesPerPixel );
            ioPackedBitmapCoords[i] = originInAtlas;
        }
    }
    A_Free( rects );
    return atlas;
}

// FIXME: these are lifted from the net, clean them up

/*
 * Filled horizontal segment of scanline y for xl<=x<=xr.
 * Parent segment was on line y-dy.  dy=1 or -1
 */
typedef struct {int y, xl, xr, dy;} Segment_t;

#define MAX_SEG_STACK (10*1024)

void COM_FloodFill( int x, int y, int winX0, int winY0, int winX1, int winY1, float color, int destWidth, int destHeight, float *dest ) {
    int size = destWidth * destHeight;

    int l, x1, x2, dy;
    int ov; /* old pixel value */
    Segment_t stack[MAX_SEG_STACK], *sp = stack;    /* stack of filled segments */

#define PUSH(Y, XL, XR, DY) /* push new segment on stack */ \
    if (sp<stack+MAX_SEG_STACK && Y+(DY)>=winY0 && Y+(DY)<=winY1) \
    {sp->y = Y; sp->xl = XL; sp->xr = XR; sp->dy = DY; sp++;}

#define POP(Y, XL, XR, DY)  /* pop segment off stack */ \
    {sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}

#define getPixel(a,b) (dest[( ( int )( a + b * destWidth ) ) % size])
#define setPixel(a,b) (dest[( ( int )( a + b * destWidth ) ) % size] = color)

    ov = getPixel(x, y);        /* read pv at seed point */
    if (ov==color || x<winX0 || x>winX1 || y<winY0 || y>winY1) return;
    PUSH(y, x, x, 1);           /* needed in some cases */
    PUSH(y+1, x, x, -1);        /* seed segment (popped 1st) */

    while (sp>stack) {
        /* pop segment off stack and fill a neighboring scan line */
        POP(y, x1, x2, dy);
        /*
         * segment of scan line y-dy for x1<=x<=x2 was previously filled,
         * now explore adjacent pixels in scan line y
         */
        for (x=x1; x>=winX0 && getPixel(x, y)==ov; x--)
            setPixel(x, y);
        if (x>=x1) goto skip;
        l = x+1;
        if (l<x1) PUSH(y, l, x1-1, -dy);        /* leak on left? */
        x = x1+1;
        do {
            for (; x<=winX1 && getPixel(x, y)==ov; x++)
                setPixel(x, y);
            PUSH(y, l, x-1, dy);
            if (x>x2+1) PUSH(y, x2+1, x-1, -dy);    /* leak on right? */
skip:       for (x++; x<=x2 && getPixel(x, y)!=ov; x++);
            l = x;
        } while (x<=x2);
    }

#undef getPixel
#undef setPixel

}

void COM_RasterizeRectangle8( c2_t topLeft, c2_t size, byte color, c2_t destSize, byte *dest ) {
    int minX = Maxi( topLeft.x, 0 );
    int minY = Maxi( topLeft.y, 0 );
    int maxX = Mini( topLeft.x + size.x, destSize.x );
    int maxY = Mini( topLeft.y + size.y, destSize.y );

    for ( int j = minY; j < maxY; j++ ) {
        memset( &dest[minX + j * destSize.x], color, maxX - minX );
    }
}

void COM_RasterizeRectangle32( int x, int y, int w, int h, int color, int destWidth, int destHeight, int *dest ) {
    int size = destWidth * destHeight;

    for ( int j = 0; j < h; j++ ) {
        for ( int i = 0; i < w; i++ ) {
            dest[( i + x + ( j + y ) * destWidth ) % size] = color;
        }
    }
}

void COM_RasterizeLine32( int p1x, int p1y, int p2x, int p2y, int color, int destWidth, int destHeight, int *dest ) {
    int F, x, y, t;
    int size = destWidth * destHeight;

#define swap(a,b) t=a, a=b, b=t
#define setPixel(a,b) dest[( ( int )( a + b * destWidth ) ) % size] = color;

    if (p1x > p2x)  // Swap points if p1 is on the right of p2
    {
        swap(p1x, p2x);
        swap(p1y, p2y);
    }

    // Handle trivial cases separately for algorithm speed up.
    // Trivial case 1: m = +/-INF (Vertical line)
    if (p1x == p2x)
    {
        if (p1y > p2y)  // Swap y-coordinates if p1 is above p2
        {
            swap(p1y, p2y);
        }

        x = p1x;
        y = p1y;
        while (y <= p2y)
        {
            setPixel(x, y);
            y++;
        }
        return;
    }
    // Trivial case 2: m = 0 (Horizontal line)
    else if (p1y == p2y)
    {
        x = p1x;
        y = p1y;

        while (x <= p2x)
        {
            setPixel(x, y);
            x++;
        }
        return;
    }

    int dy            = p2y - p1y;  // y-increment from p1 to p2
    int dx            = p2x - p1x;  // x-increment from p1 to p2
    int dy2           = (dy << 1);  // dy << 1 == 2*dy
    int dx2           = (dx << 1);
    int dy2_minus_dx2 = dy2 - dx2;  // precompute constant for speed up
    int dy2_plus_dx2  = dy2 + dx2;


    if (dy >= 0)    // m >= 0
    {
        // Case 1: 0 <= m <= 1 (Original case)
        if (dy <= dx)   
        {
            F = dy2 - dx;    // initial F

            x = p1x;
            y = p1y;
            while (x <= p2x)
           {
               setPixel(x, y);
               if (F <= 0)
               {
                   F += dy2;
               }
               else
               {
                   y++;
                   F += dy2_minus_dx2;
               }
               x++;
           }
       }
       // Case 2: 1 < m < INF (Mirror about y=x line
       // replace all dy by dx and dx by dy)
       else
       {
           F = dx2 - dy;    // initial F

           y = p1y;
           x = p1x;
           while (y <= p2y)
           {
               setPixel(x, y);
               if (F <= 0)
               {
                   F += dx2;
                }
                else
                {
                    x++;
                    F -= dy2_minus_dx2;
                }
                y++;
            }
        }
    }
    else    // m < 0
    {
        // Case 3: -1 <= m < 0 (Mirror about x-axis, replace all dy by -dy)
        if (dx >= -dy)
        {
            F = -dy2 - dx;    // initial F

            x = p1x;
            y = p1y;
            while (x <= p2x)
            {
                setPixel(x, y);
                if (F <= 0)
                {
                    F -= dy2;
                }
                else
                {
                    y--;
                    F -= dy2_plus_dx2;
                }
                x++;
            }
        }
        // Case 4: -INF < m < -1 (Mirror about x-axis and mirror 
        // about y=x line, replace all dx by -dy and dy by dx)
        else    
        {
            F = dx2 + dy;    // initial F

            y = p1y;
            x = p1x;
            while (y >= p2y)
            {
                setPixel(x, y);
                if (F <= 0)
                {
                    F += dx2;
                }
                else
                {
                    x++;
                    F += dy2_plus_dx2;
                }
                y--;
            }
        }
    }

#undef getPixel
#undef setPixel

}
