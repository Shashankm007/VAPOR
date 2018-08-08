#include <vapor/DVRParams.h>

using namespace VAPoR;

const std::string DVRParams::_lightingTag       = "LightingTag";
const std::string DVRParams::_lightingCoeffsTag = "LightingCoeffTag";

//
// Register class with object factory
//
static RenParamsRegistrar<DVRParams> registrar( DVRParams::GetClassType() );


DVRParams::DVRParams( DataMgr*                dataManager, 
                      ParamsBase::StateSave*  stateSave )
         : RenderParams( dataManager, 
                         stateSave, 
                         DVRParams::GetClassType(), 
                         3 /* max dim */ )
{
    SetDiagMsg("DVRParams::DVRParams() this=%p", this);
}

// Constructor
DVRParams::DVRParams( DataMgr*                dataManager, 
                      ParamsBase::StateSave*  stateSave,
                      XmlNode*                node )
         : RenderParams( dataManager, 
                         stateSave, 
                         node,
                         3 /* max dim */ )
{
    SetDiagMsg("DVRParams::DVRParams() this=%p", this);
    if( node->GetTag() != DVRParams::GetClassType() )
    {
        node->SetTag( DVRParams::GetClassType() );
    }
}

// Destructor
DVRParams::~DVRParams()
{
    SetDiagMsg( "DVRParams::~DVRParams() this=%p", this );
}
  
MapperFunction* DVRParams::GetMapperFunc()
{
    return RenderParams::GetMapperFunc( GetVariableName() ); 
}


void DVRParams::SetLighting( bool lightingOn )
{
    SetValueLong( _lightingTag, "Apply lighting or not", (long int)lightingOn );
}

bool DVRParams::GetLighting() const
{
    long l = GetValueLong( _lightingTag, 1 );

    return (bool)l;
}
    
std::vector<double> DVRParams::GetLightingCoeffs() const
{
    std::vector<double> defaultVec( 4 );
    defaultVec[0] = 0.5;
    defaultVec[1] = 0.3;
    defaultVec[2] = 0.2;
    defaultVec[3] = 12.0;
    return GetValueDoubleVec( _lightingCoeffsTag, defaultVec );
}
    
void DVRParams::SetLightingCoeffs( const std::vector<double>& coeffs )
{
    SetValueDoubleVec( _lightingCoeffsTag, "Coefficients for lighting effects", coeffs );
}