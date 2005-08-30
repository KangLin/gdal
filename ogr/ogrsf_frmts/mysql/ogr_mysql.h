/******************************************************************************
 * $Id$
 *
 * Project:  OpenGIS Simple Features Reference Implementation
 * Purpose:  Declarations for MySQL OGR Driver Classes.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2004, Frank Warmerdam <warmerdam@pobox.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log$
 * Revision 1.5  2005/08/30 23:53:16  fwarmerdam
 * implement binary field support
 *
 * Revision 1.4  2005/02/22 12:54:27  fwarmerdam
 * use OGRLayer base spatial filter support
 *
 * Revision 1.3  2004/10/12 16:59:31  fwarmerdam
 * rearrange include files for win32
 *
 * Revision 1.2  2004/10/08 20:49:01  fwarmerdam
 * enable ExecuteSQL
 *
 * Revision 1.1  2004/10/07 20:57:50  fwarmerdam
 * New
 *
 */

#ifndef _OGR_MYSQL_H_INCLUDED
#define _OGR_MYSQL_H_INLLUDED

#include <my_global.h>
#include <mysql.h>

#include "ogrsf_frmts.h"

/************************************************************************/
/*                            OGRMySQLLayer                             */
/************************************************************************/

class OGRMySQLDataSource;
    
class OGRMySQLLayer : public OGRLayer
{
  protected:
    OGRFeatureDefn     *poFeatureDefn;

    // Layer spatial reference system, and srid.
    OGRSpatialReference *poSRS;
    int                 nSRSId;

    int                 iNextShapeId;

    OGRMySQLDataSource    *poDS;

    char               *pszQueryStatement;

    int                 nResultOffset;

    char                *pszGeomColumn;

    int                 bHasFid;
    char                *pszFIDColumn;

    MYSQL_RES           *hResultSet;

  public:
                        OGRMySQLLayer();
    virtual             ~OGRMySQLLayer();

    virtual void        ResetReading();

    virtual OGRFeature *GetNextFeature();

    virtual OGRFeature *GetFeature( long nFeatureId );
    
    OGRFeatureDefn *    GetLayerDefn() { return poFeatureDefn; }

    virtual OGRSpatialReference *GetSpatialRef();

    virtual int         TestCapability( const char * );

    /* custom methods */
    virtual OGRFeature *RecordToFeature( char **papszRow, unsigned long * );
    virtual OGRFeature *GetNextRawFeature();
};

/************************************************************************/
/*                          OGRMySQLTableLayer                          */
/************************************************************************/

class OGRMySQLTableLayer : public OGRMySQLLayer
{
    int                 bUpdateAccess;

    OGRFeatureDefn     *ReadTableDefinition(const char *);

    void                BuildWhere(void);
    char               *BuildFields(void);
    void                BuildFullQueryStatement(void);

    char                *pszQuery;
    char                *pszWHERE;

    int                 bLaunderColumnNames;
    int                 bPreservePrecision;
    
  public:
                        OGRMySQLTableLayer( OGRMySQLDataSource *,
                                         const char * pszName,
                                         int bUpdate, int nSRSId = -2 );
                        ~OGRMySQLTableLayer();

    virtual OGRFeature *GetFeature( long nFeatureId );
    virtual void        ResetReading();
    virtual int         GetFeatureCount( int );

    void                SetSpatialFilter( OGRGeometry * );

    virtual OGRErr      SetAttributeFilter( const char * );
#ifdef notdef
    virtual OGRErr      CreateFeature( OGRFeature *poFeature );
    
    virtual OGRErr      CreateField( OGRFieldDefn *poField,
                                     int bApproxOK = TRUE );
#endif
    
    virtual OGRSpatialReference *GetSpatialRef();

    virtual int         TestCapability( const char * );
};

/************************************************************************/
/*                         OGRMySQLResultLayer                          */
/************************************************************************/

class OGRMySQLResultLayer : public OGRMySQLLayer
{
    void                BuildFullQueryStatement(void);

    char                *pszRawStatement;

    int                 nFeatureCount;

  public:
                        OGRMySQLResultLayer( OGRMySQLDataSource *,
                                             const char * pszRawStatement,
                                             MYSQL_RES *hResultSetIn );
    virtual             ~OGRMySQLResultLayer();

    OGRFeatureDefn     *ReadResultDefinition();

    virtual void        ResetReading();
    virtual int         GetFeatureCount( int );
};

/************************************************************************/
/*                          OGRMySQLDataSource                          */
/************************************************************************/

class OGRMySQLDataSource : public OGRDataSource
{
    OGRMySQLLayer        **papoLayers;
    int                 nLayers;
    
    char               *pszName;

    int                 bDSUpdate;
    int                 bHavePostGIS;
    int                 nSoftTransactionLevel;

    MYSQL              *hConn;

    void                DeleteLayer( const char *pszLayerName );

    // We maintain a list of known SRID to reduce the number of trips to
    // the database to get SRSes. 
    int                 nKnownSRID;
    int                *panSRID;
    OGRSpatialReference **papoSRS;

    OGRMySQLLayer      *poLongResultLayer;
    
  public:
                        OGRMySQLDataSource();
                        ~OGRMySQLDataSource();

    MYSQL              *GetConn() { return hConn; }

#ifdef notdef
    int                 FetchSRSId( OGRSpatialReference * poSRS );
    OGRSpatialReference *FetchSRS( int nSRSId );
    OGRErr              InitializeMetadataTables();
#endif

    int                 Open( const char *, int bUpdate, int bTestOpen );
    int                 OpenTable( const char *, int bUpdate, int bTestOpen );

    const char          *GetName() { return pszName; }
    int                 GetLayerCount() { return nLayers; }
    OGRLayer            *GetLayer( int );

#ifdef notdef
    virtual OGRLayer    *CreateLayer( const char *, 
                                      OGRSpatialReference * = NULL,
                                      OGRwkbGeometryType = wkbUnknown,
                                      char ** = NULL );
#endif

    int                 TestCapability( const char * );

    virtual OGRLayer *  ExecuteSQL( const char *pszSQLCommand,
                                    OGRGeometry *poSpatialFilter,
                                    const char *pszDialect );
    virtual void        ReleaseResultSet( OGRLayer * poLayer );

    // nonstandard

    void                ReportError( const char * = NULL );
    
    char               *LaunderName( const char * );

    void                RequestLongResult( OGRMySQLLayer * );
    void                InterruptLongResult();
};

/************************************************************************/
/*                            OGRMySQLDriver                            */
/************************************************************************/

class OGRMySQLDriver : public OGRSFDriver
{
  public:
                ~OGRMySQLDriver();
                
    const char *GetName();
    OGRDataSource *Open( const char *, int );
#ifdef notdef
    virtual OGRDataSource *CreateDataSource( const char *pszName,
                                             char ** = NULL );
#endif    
    int                 TestCapability( const char * );
};


#endif /* ndef _OGR_PG_H_INCLUDED */


