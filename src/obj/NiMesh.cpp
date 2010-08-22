/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for license. */

//-----------------------------------NOTICE----------------------------------//
// Some of this file is automatically filled in by a Python script.  Only    //
// add custom code in the designated areas or it will be overwritten during  //
// the next update.                                                          //
//-----------------------------------NOTICE----------------------------------//

//--BEGIN FILE HEAD CUSTOM CODE--//

//--END CUSTOM CODE--//

#include "../../include/FixLink.h"
#include "../../include/ObjectRegistry.h"
#include "../../include/NIF_IO.h"
#include "../../include/obj/NiMesh.h"
#include "../../include/gen/SphereBV.h"
#include "../../include/gen/MeshData.h"
#include "../../include/gen/SemanticData.h"
#include "../../include/obj/NiDataStream.h"
#include "../../include/obj/NiMeshModifier.h"
using namespace Niflib;

//Definition of TYPE constant
const Type NiMesh::TYPE("NiMesh", &NiRenderObject::TYPE );

NiMesh::NiMesh() : primitiveType((MeshPrimitiveType)0), numSubmeshes((unsigned short)0), instancingEnabled(false), numDatas((unsigned int)0), numModifiers((unsigned int)0) {
	//--BEGIN CONSTRUCTOR CUSTOM CODE--//

	//--END CUSTOM CODE--//
}

NiMesh::~NiMesh() {
	//--BEGIN DESTRUCTOR CUSTOM CODE--//

	//--END CUSTOM CODE--//
}

const Type & NiMesh::GetType() const {
	return TYPE;
}

NiObject * NiMesh::Create() {
	return new NiMesh;
}

void NiMesh::Read( istream& in, list<unsigned int> & link_stack, const NifInfo & info ) {
	//--BEGIN PRE-READ CUSTOM CODE--//

	//--END CUSTOM CODE--//

	unsigned int block_num;
	NiRenderObject::Read( in, link_stack, info );
	NifStream( primitiveType, in, info );
	NifStream( numSubmeshes, in, info );
	NifStream( instancingEnabled, in, info );
	NifStream( bound.center, in, info );
	NifStream( bound.radius, in, info );
	NifStream( numDatas, in, info );
	datas.resize(numDatas);
	for (unsigned int i1 = 0; i1 < datas.size(); i1++) {
		NifStream( block_num, in, info );
		link_stack.push_back( block_num );
		NifStream( datas[i1].isPerInstance, in, info );
		NifStream( datas[i1].numSubmeshes, in, info );
		datas[i1].submeshToRegionMap.resize(datas[i1].numSubmeshes);
		for (unsigned int i2 = 0; i2 < datas[i1].submeshToRegionMap.size(); i2++) {
			NifStream( datas[i1].submeshToRegionMap[i2], in, info );
		};
		NifStream( datas[i1].numComponents, in, info );
		datas[i1].componentSemantics.resize(datas[i1].numComponents);
		for (unsigned int i2 = 0; i2 < datas[i1].componentSemantics.size(); i2++) {
			NifStream( datas[i1].componentSemantics[i2].name, in, info );
			NifStream( datas[i1].componentSemantics[i2].index, in, info );
		};
	};
	NifStream( numModifiers, in, info );
	modifiers.resize(numModifiers);
	for (unsigned int i1 = 0; i1 < modifiers.size(); i1++) {
		NifStream( block_num, in, info );
		link_stack.push_back( block_num );
	};

	//--BEGIN POST-READ CUSTOM CODE--//

	//--END CUSTOM CODE--//
}

void NiMesh::Write( ostream& out, const map<NiObjectRef,unsigned int> & link_map, const NifInfo & info ) const {
	//--BEGIN PRE-WRITE CUSTOM CODE--//

	//--END CUSTOM CODE--//

	NiRenderObject::Write( out, link_map, info );
	numModifiers = (unsigned int)(modifiers.size());
	numDatas = (unsigned int)(datas.size());
	NifStream( primitiveType, out, info );
	NifStream( numSubmeshes, out, info );
	NifStream( instancingEnabled, out, info );
	NifStream( bound.center, out, info );
	NifStream( bound.radius, out, info );
	NifStream( numDatas, out, info );
	for (unsigned int i1 = 0; i1 < datas.size(); i1++) {
		datas[i1].numComponents = (int)(datas[i1].componentSemantics.size());
		datas[i1].numSubmeshes = (unsigned short)(datas[i1].submeshToRegionMap.size());
		if ( info.version < VER_3_3_0_13 ) {
			WritePtr32( &(*datas[i1].stream), out );
		} else {
			if ( datas[i1].stream != NULL ) {
				NifStream( link_map.find( StaticCast<NiObject>(datas[i1].stream) )->second, out, info );
			} else {
				NifStream( 0xFFFFFFFF, out, info );
			}
		}
		NifStream( datas[i1].isPerInstance, out, info );
		NifStream( datas[i1].numSubmeshes, out, info );
		for (unsigned int i2 = 0; i2 < datas[i1].submeshToRegionMap.size(); i2++) {
			NifStream( datas[i1].submeshToRegionMap[i2], out, info );
		};
		NifStream( datas[i1].numComponents, out, info );
		for (unsigned int i2 = 0; i2 < datas[i1].componentSemantics.size(); i2++) {
			NifStream( datas[i1].componentSemantics[i2].name, out, info );
			NifStream( datas[i1].componentSemantics[i2].index, out, info );
		};
	};
	NifStream( numModifiers, out, info );
	for (unsigned int i1 = 0; i1 < modifiers.size(); i1++) {
		if ( info.version < VER_3_3_0_13 ) {
			WritePtr32( &(*modifiers[i1]), out );
		} else {
			if ( modifiers[i1] != NULL ) {
				NifStream( link_map.find( StaticCast<NiObject>(modifiers[i1]) )->second, out, info );
			} else {
				NifStream( 0xFFFFFFFF, out, info );
			}
		}
	};

	//--BEGIN POST-WRITE CUSTOM CODE--//

	//--END CUSTOM CODE--//
}

std::string NiMesh::asString( bool verbose ) const {
	//--BEGIN PRE-STRING CUSTOM CODE--//

	//--END CUSTOM CODE--//

	stringstream out;
	unsigned int array_output_count = 0;
	out << NiRenderObject::asString();
	numModifiers = (unsigned int)(modifiers.size());
	numDatas = (unsigned int)(datas.size());
	out << "  Primitive Type:  " << primitiveType << endl;
	out << "  Num Submeshes:  " << numSubmeshes << endl;
	out << "  Instancing Enabled:  " << instancingEnabled << endl;
	out << "  Center:  " << bound.center << endl;
	out << "  Radius:  " << bound.radius << endl;
	out << "  Num Datas:  " << numDatas << endl;
	array_output_count = 0;
	for (unsigned int i1 = 0; i1 < datas.size(); i1++) {
		if ( !verbose && ( array_output_count > MAXARRAYDUMP ) ) {
			out << "<Data Truncated. Use verbose mode to see complete listing.>" << endl;
			break;
		};
		datas[i1].numComponents = (int)(datas[i1].componentSemantics.size());
		datas[i1].numSubmeshes = (unsigned short)(datas[i1].submeshToRegionMap.size());
		out << "    Stream:  " << datas[i1].stream << endl;
		out << "    Is Per Instance:  " << datas[i1].isPerInstance << endl;
		out << "    Num Submeshes:  " << datas[i1].numSubmeshes << endl;
		array_output_count = 0;
		for (unsigned int i2 = 0; i2 < datas[i1].submeshToRegionMap.size(); i2++) {
			if ( !verbose && ( array_output_count > MAXARRAYDUMP ) ) {
				out << "<Data Truncated. Use verbose mode to see complete listing.>" << endl;
				break;
			};
			if ( !verbose && ( array_output_count > MAXARRAYDUMP ) ) {
				break;
			};
			out << "      Submesh To Region Map[" << i2 << "]:  " << datas[i1].submeshToRegionMap[i2] << endl;
			array_output_count++;
		};
		out << "    Num Components:  " << datas[i1].numComponents << endl;
		array_output_count = 0;
		for (unsigned int i2 = 0; i2 < datas[i1].componentSemantics.size(); i2++) {
			if ( !verbose && ( array_output_count > MAXARRAYDUMP ) ) {
				out << "<Data Truncated. Use verbose mode to see complete listing.>" << endl;
				break;
			};
			out << "      Name:  " << datas[i1].componentSemantics[i2].name << endl;
			out << "      Index:  " << datas[i1].componentSemantics[i2].index << endl;
		};
	};
	out << "  Num Modifiers:  " << numModifiers << endl;
	array_output_count = 0;
	for (unsigned int i1 = 0; i1 < modifiers.size(); i1++) {
		if ( !verbose && ( array_output_count > MAXARRAYDUMP ) ) {
			out << "<Data Truncated. Use verbose mode to see complete listing.>" << endl;
			break;
		};
		if ( !verbose && ( array_output_count > MAXARRAYDUMP ) ) {
			break;
		};
		out << "    Modifiers[" << i1 << "]:  " << modifiers[i1] << endl;
		array_output_count++;
	};
	return out.str();

	//--BEGIN POST-STRING CUSTOM CODE--//

	//--END CUSTOM CODE--//
}

void NiMesh::FixLinks( const map<unsigned int,NiObjectRef> & objects, list<unsigned int> & link_stack, const NifInfo & info ) {
	//--BEGIN PRE-FIXLINKS CUSTOM CODE--//

	//--END CUSTOM CODE--//

	NiRenderObject::FixLinks( objects, link_stack, info );
	for (unsigned int i1 = 0; i1 < datas.size(); i1++) {
		datas[i1].stream = FixLink<NiDataStream>( objects, link_stack, info );
	};
	for (unsigned int i1 = 0; i1 < modifiers.size(); i1++) {
		modifiers[i1] = FixLink<NiMeshModifier>( objects, link_stack, info );
	};

	//--BEGIN POST-FIXLINKS CUSTOM CODE--//

	//--END CUSTOM CODE--//
}

std::list<NiObjectRef> NiMesh::GetRefs() const {
	list<Ref<NiObject> > refs;
	refs = NiRenderObject::GetRefs();
	for (unsigned int i1 = 0; i1 < datas.size(); i1++) {
		if ( datas[i1].stream != NULL )
			refs.push_back(StaticCast<NiObject>(datas[i1].stream));
	};
	for (unsigned int i1 = 0; i1 < modifiers.size(); i1++) {
		if ( modifiers[i1] != NULL )
			refs.push_back(StaticCast<NiObject>(modifiers[i1]));
	};
	return refs;
}

//--BEGIN MISC CUSTOM CODE--//

//--END CUSTOM CODE--//
