//************************************************************************
//                                                                       *
//                          Copyright (C)  2018                          *
//            University Corporation for Atmospheric Research            *
//                          All Rights Reserved                          *
//                                                                       *
//************************************************************************/
//
//  File:   ContourRenderer.cpp
//
//  Author: Stas Jaroszynski
//          National Center for Atmospheric Research
//          PO 3000, Boulder, Colorado
//
//  Date:   March 2018
//
//  Description:
//          Implementation of ContourRenderer
//

#include <sstream>
#include <string>
#include <iterator>

#include <vapor/glutil.h>	// Must be included first!!!

#include <vapor/Renderer.h>
#include <vapor/ContourRenderer.h>
#include <vapor/Visualizer.h>
//#include <vapor/params.h>
#include <vapor/ContourParams.h>
//#include <vapor/AnimationParams.h>
#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/DataStatus.h>
#include <vapor/errorcodes.h>
#include <vapor/GetAppPath.h>
#include <vapor/ControlExecutive.h>
#include "vapor/debug.h"

using namespace VAPoR;

static RendererRegistrar<ContourRenderer> registrar(
	ContourRenderer::GetClassType(), ContourParams::GetClassType()
);

ContourRenderer::ContourRenderer(const ParamsMgr* pm, string winName, string dataSetName, string instName, DataMgr* dataMgr)
				: Renderer(pm, winName, dataSetName, ContourParams::GetClassType(), ContourRenderer::GetClassType(), instName, dataMgr),
				drawList(0) {
    FTRACE(pm, winName, dataSetName, instName, "*dataMgr");
}

ContourRenderer::~ContourRenderer()
{
    FTRACE();
    if (drawList)
        glDeleteLists(drawList, 1);
}

void ContourRenderer::_saveCacheParams()
{
    ContourParams* p = (ContourParams*)GetActiveParams();
    cacheParams.varName = p->GetVariableName();
    cacheParams.heightVarName = p->GetHeightVariableName();
    cacheParams.ts = p->GetCurrentTimestep();
    cacheParams.level = p->GetRefinementLevel();
    cacheParams.lod = p->GetCompressionLevel();
    cacheParams.useSingleColor = p->UseSingleColor();
    cacheParams.lineThickness = p->GetLineThickness();
    p->GetConstantColor(cacheParams.constantColor);
    p->GetBox()->GetExtents(cacheParams.boxMin, cacheParams.boxMax);
    cacheParams.contourValues = p->GetContourValues(cacheParams.varName);
    
    MapperFunction *tf = p->GetMapperFunc(cacheParams.varName);
    cacheParams.opacity = tf->getOpacityScale();
    cacheParams.contourColors.resize(cacheParams.contourValues.size()*3);
    for (int i = 0; i < cacheParams.contourValues.size(); i++)
        tf->rgbValue(cacheParams.contourValues[i], &cacheParams.contourColors[i*3]);
}

bool ContourRenderer::_isCacheDirty() const
{
    ContourParams *p = (ContourParams*)GetActiveParams();
    if (cacheParams.varName != p->GetVariableName()) return true;
    if (cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (cacheParams.ts      != p->GetCurrentTimestep()) return true;
    if (cacheParams.level   != p->GetRefinementLevel()) return true;
    if (cacheParams.lod     != p->GetCompressionLevel()) return true;
    if (cacheParams.useSingleColor != p->UseSingleColor()) return true;
    if (cacheParams.lineThickness != p->GetLineThickness()) return true;
    
    vector<double> min, max, contourValues;
    p->GetBox()->GetExtents(min, max);
    contourValues = p->GetContourValues(cacheParams.varName);
    
    if (cacheParams.boxMin != min) return true;
    if (cacheParams.boxMax != max) return true;
    if (cacheParams.contourValues != contourValues) return true;
    
    float constantColor[3];
    p->GetConstantColor(constantColor);
    if (memcmp(cacheParams.constantColor, constantColor, sizeof(constantColor))) return true;
    
    MapperFunction *tf = p->GetMapperFunc(cacheParams.varName);
    vector<float> contourColors(cacheParams.contourValues.size()*3);
    for (int i = 0; i < cacheParams.contourValues.size(); i++)
        tf->rgbValue(cacheParams.contourValues[i], &contourColors[i*3]);
    if (cacheParams.contourColors != contourColors) return true;
    if (cacheParams.opacity != tf->getOpacityScale()) return true;
    
    return false;
}

void ContourRenderer::_buildCache()
{
    ContourParams* cParams = (ContourParams*)GetActiveParams();
    _saveCacheParams();
    
    glNewList(drawList, GL_COMPILE);
    glLineWidth(cacheParams.lineThickness);
    if (cParams->GetVariableName().empty()) {
        glEndList();
        return;
    }
    MapperFunction *tf = cParams->GetMapperFunc(cacheParams.varName);
    vector<double> contours = cParams->GetContourValues(cacheParams.varName);
    float contourColors[contours.size()][4];
    if (!cacheParams.useSingleColor)
        for (int i = 0; i < contours.size(); i++)
            tf->rgbValue(contours[i], contourColors[i]);
    else
        for (int i = 0; i < contours.size(); i++)
            memcpy(contourColors[i], cacheParams.constantColor, sizeof(cacheParams.constantColor));
    for (int i = 0; i < contours.size(); i++)
        contourColors[i][3] = cacheParams.opacity;
    
    Grid *grid = _dataMgr->GetVariable(cacheParams.ts, cacheParams.varName, cacheParams.level, cacheParams.lod, cacheParams.boxMin, cacheParams.boxMax);
    Grid *heightGrid = NULL;
    if (!cacheParams.heightVarName.empty())
        heightGrid = _dataMgr->GetVariable(cacheParams.ts, cacheParams.heightVarName, cacheParams.level, cacheParams.lod, cacheParams.boxMin, cacheParams.boxMax);
    //StructuredGrid *sGrid = dynamic_cast<StructuredGrid *>(grid);
    
	for (Grid::ConstCellIterator it = grid->ConstCellBegin(); it != grid->ConstCellEnd(); ++it) {
        vector<size_t> cell = *it;
        vector<vector<size_t>> nodes;
        grid->GetCellNodes(cell, nodes);
        
        vector<double> coords[nodes.size()];
        float values[nodes.size()];
        for (int i = 0; i < nodes.size(); i++) {
            grid->GetUserCoordinates(nodes[i], coords[i]);
            values[i] = grid->GetValue(coords[i]);
        }
        
        glBegin(GL_LINES);

        for (int ci = 0; ci != contours.size(); ci++) {
            glColor4fv(contourColors[ci]);
            for (int a=nodes.size()-1, b=0; b < nodes.size(); a++, b++) {
                if (a == nodes.size()) a = 0;
                double contour = contours[ci];
                
                if ((values[a] <= contour && values[b] <= contour) || (values[a] > contour && values[b] > contour))
                    continue;
                
                float t = (contour - values[a])/(values[b] - values[a]);
                float v[3];
                v[0] = coords[a][0] + t * (coords[b][0] - coords[a][0]);
                v[1] = coords[a][1] + t * (coords[b][1] - coords[a][1]);
                v[2] = 0;
                
                if (heightGrid) {
                    float aHeight = heightGrid->GetValue(coords[a]);
                    float bHeight = heightGrid->GetValue(coords[b]);
                    v[2] = aHeight + t * (bHeight - aHeight);
                }
                
                glVertex3fv(v);
            }
        }
        glEnd();
    }
    
    glEndList();
}

int ContourRenderer::_paintGL()
{
    if (_isCacheDirty())
        _buildCache();
    
    glCallList(drawList);

    return 0;
}

int ContourRenderer::_initializeGL()
{
    drawList = glGenLists(1);
	return 0;
}
