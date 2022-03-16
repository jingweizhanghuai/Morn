#include <stdio.h>
#include <stdarg.h>

#include <geos_c.h>

static void geos_message_handler(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf (fmt, ap);
    va_end(ap);
}

int main()
{
    initGEOS(geos_message_handler, geos_message_handler);

    // GEOSCoordSequence* GEOSCoordSeq_create(5,2);

    double coord_a[10]={0,0,10,0,10,10,0,10,0,0};
    GEOSCoordSequence* seq_a = GEOSCoordSeq_copyFromBuffer(coord_a,5,0,0);
    GEOSGeometry* ring_a = GEOSGeom_createLinearRing(seq_a);
    GEOSGeometry* geom_a = GEOSGeom_createPolygon(ring_a,NULL,0);

    double coord_b[10]={5,5,15,5,15,15,5,15,5,5};
    GEOSCoordSequence* seq_b = GEOSCoordSeq_copyFromBuffer(coord_b,5,0,0);
    GEOSGeometry* ring_b = GEOSGeom_createLinearRing(seq_b);
    GEOSGeometry* geom_b = GEOSGeom_createPolygon(ring_b,NULL,0);

    // const char* wkt_a = "POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))";
    // const char* wkt_b = "POLYGON((5 5, 15 5, 15 15, 5 15, 5 5))";

    // GEOSWKTReader* reader = GEOSWKTReader_create();
    // GEOSGeometry* geom_a = GEOSWKTReader_read(reader, wkt_a);
    // GEOSGeometry* geom_b = GEOSWKTReader_read(reader, wkt_b);
    
    GEOSGeometry* inter = GEOSIntersection(geom_a, geom_b);
    
    const GEOSCoordSequence* inter_seq = GEOSGeom_getCoordSeq(inter);


    GEOSWKTWriter* writer = GEOSWKTWriter_create();
    GEOSWKTWriter_setTrim(writer, 1);
    char* wkt_inter = GEOSWKTWriter_write(writer, inter);


    // printf("Geometry A:         %s\n", wkt_a);
    // printf("Geometry B:         %s\n", wkt_b);
    printf("Intersection(A, B): %s\n", wkt_inter);

    // GEOSWKTReader_destroy(reader);
    GEOSWKTWriter_destroy(writer);
    GEOSGeom_destroy(geom_a);
    GEOSGeom_destroy(geom_b);
    GEOSGeom_destroy(inter);
    GEOSFree(wkt_inter);

    finishGEOS();
    return 0;
}