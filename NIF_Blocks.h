/* Copyright (c) 2005, NIF File Format Library and Tools
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.

   * Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.

   * Neither the name of the NIF File Format Library and Tools
     project nor the names of its contributors may be used to endorse
     or promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE. */

#ifndef _NIF_BLOCKS_H
#define _NIF_BLOCKS_H

/* INCLUDES */
#include "niflib.h"
#include "nif_math.h"
#include "NIF_IO.h"

#include <vector>
#include <list>
#include <map>
#include <cmath>

//--Types--//
typedef	pair<attr_ref, blk_ref> LinkPair;
typedef multimap<attr_ref, blk_ref> LinkMap;
typedef LinkMap::iterator LinkMapIt;
typedef pair<LinkMapIt,LinkMapIt> LinkMapRange;

//--Constants--//

//Non-Public Interface IDs
const int BlockInternal = -1;
const int SkinInstInternal = -2;
const int SkinDataInternal = -3;


//void GetBuiltUpTransform(blk_ref block, Matrix & m/*, blk_ref stop*/);

class IBlockInternal {
public:
	//Cross Reference
	virtual void AddParent( blk_ref parent ) = 0;
	virtual void RemoveParent( IBlock * match ) = 0;
	virtual void SetBlockNum( int ) = 0;
	virtual void FixUpLinks( const vector<blk_ref> & blocks ) = 0;

	//File I/O
	virtual void Read( ifstream& in ) = 0;
	virtual void Write( ofstream& out ) = 0;	
};

class ABlock : public IBlock, public IBlockInternal {
public:
	ABlock();
	~ABlock();
	void AddAttr( string type, string name );
	attr_ref GetAttr(string attr_name);
	attr_ref GetAttr(int index);
	int AttrCount();
	int GetBlockNum() { return _block_num; }
	bool IsNode() { return false; }
	string asString();

	//Links
	blk_ref GetParent();
	list<blk_ref> GetLinks();

	//Reference Counting
	void AddRef();
	void SubtractRef();

	//Interface
	void * QueryInterface( int id ) {
		if ( id == BlockInternal ) {
			return (void*)static_cast<IBlockInternal*>(this);;
		} else {
			return NULL;
		}
	}

	//--Internal Functions--//
	void AddParent( blk_ref parent);
	void RemoveParent( IBlock * match );
	void SetBlockNum( int n ) { _block_num = n; }
	void FixUpLinks( const vector<blk_ref> & blocks );

	void Read( ifstream& in );
	void Write( ofstream& out );
private:
	map<string, attr_ref> _attr_map;
	vector<attr_ref> _attr_vect;
	int _block_num;
	unsigned int _ref_count;
	vector<IBlock*> _parents;
};

class ANamed : public ABlock {
public:
	ANamed(){
		AddAttr( "string", "Name" );
		AddAttr( "index", "Extra Data" );
		AddAttr( "index", "Controller" );
	}
	~ANamed(){}
};

class ANode : public ANamed, public INode {
public:
	ANode(){
		AddAttr( "flags", "Flags" );
		AddAttr( "float3", "Translation" );
		AddAttr( "matrix", "Rotation" );
		AddAttr( "float", "Scale" );
		AddAttr( "float3", "velocity" );
		AddAttr( "properties", "Properties" );
		AddAttr( "bbox", "Bounding Box" );

		SetIdentity44(bindPosition);
	}
	~ANode(){}
	void * QueryInterface( int id );
	void Read( ifstream& in ) {
		ABlock::Read( in );
		float transform[4][4];
		GetLocalTransform( transform );
		SetBindPosition( transform );
	}

	//Node Functions
	void GetLocalTransform( float out_matrix[4][4] );
	void GetWorldTransform( float out_matrix[4][4] );
	void GetBindPosition( float out_matrix[4][4] );
	void GetLocalBindPos( float out_matrix[4][4] );
	void SetBindPosition( float in_matrix[4][4] );

private:
	float bindPosition[4][4];
};

class AParentNode : public ANode {
public:
	AParentNode(){
		AddAttr( "children", "Children" );
		AddAttr( "effects", "Effects" );
	}
	~AParentNode(){}
};

class AProperty : public ANamed {
public:
	AProperty() {
		AddAttr( "flags", "Flags" );
	}
	~AProperty() {}
};

class AController : public ABlock {
public:
	AController() {
		AddAttr( "index", "Next Controller" );
		AddAttr( "flags", "Flags" );
		AddAttr( "float", "Frequency" );
		AddAttr( "float", "Phase" );
		AddAttr( "float", "Start Time" );
		AddAttr( "float", "Stop Time" );
		AddAttr( "int", "Target Node" );
	}
	~AController() {}
};

class ALight   : public ANamed {
public:
	ALight  (){
		AddAttr( "int", "Unknown Int 1" );
		AddAttr( "int", "Unknown Int 2" );
		AddAttr( "float", "Dimmer" );
		AddAttr( "float3", "Ambient Color" );
		AddAttr( "float3", "Diffuse Color" );
		AddAttr( "float3", "Specular Color" );
	}
	~ALight  (){}
};

// Used to move temporarily stored bone indicies into SkinInstanceData block to facilitate
// the illusion that these are stored together with the bone data
class ISkinInstance { 
};


/**
 * NiNode - Root of each model component.
 */
class NiNode : public AParentNode {
public:

	NiNode(){}
	~NiNode(){}
	string GetBlockType() { return "NiNode"; }
	void Print( ostream &out ) {
		out << ABlock::asString();

		//matrix m;
		//GetAttr("Rotation")->asMatrix( m );

		//Vector rows[3];

		//rows[0] = Vector(m[0][0], m[0][1], m[0][2]);
		//rows[1] = Vector(m[1][0], m[1][1], m[1][2]);
		//rows[2] = Vector(m[2][0], m[2][1], m[2][2]);


		////out << "Rotation Matrix Test:" << endl
		////	<< "   Dot Products of each row with each other row:" << endl;

		////for (int i = 0; i < 3; ++i) {
		////	for (int j = 0; j < 3; ++j) {
		////		out << "      Rows " << i << " & " << j << ":  " << rows[i].dot(rows[j]) << endl;
		////	}
		////}

		//float pi = 3.141592653589793f;
		//out << "Euler Angles:" << endl
		//	<< "   X:  " << atan2( m[1][2], m[2][2] ) / pi * 180.0 << endl
		//	<< "   Y:  " << asin( -m[0][2] ) / pi * 180.0 << endl
		//	<< "   Z:  " << atan2( m[0][1], m[0][0] ) / pi * 180.0 << endl;

		//Quat q = MatrixToQuat( m );
		//out << "Quaternion:  [" << setw(6) << q.w << " (" << setw(6) << q.x << "," << setw(6) << q.y << "," << setw(6) << q.z << ")]" << endl;

		//Matrix built_up = IdentityMatrix();
		//GetBuiltUpTransform(blk_ref(this), built_up);
		//out << "Built Up Transformations:" << endl
		//	<< "   |" << setw(6) << built_up(0,0) << "," << setw(6) << built_up(0,1) << "," << setw(6) << built_up(0,2) << "," << setw(6) << built_up(0,3) << " |" << endl
		//	<< "   |" << setw(6) << built_up(1,0) << "," << setw(6) << built_up(1,1) << "," << setw(6) << built_up(1,2) << "," << setw(6) << built_up(1,3) << " |" << endl
		//	<< "   |" << setw(6) << built_up(2,0) << "," << setw(6) << built_up(2,1) << "," << setw(6) << built_up(2,2) << "," << setw(6) << built_up(2,3) << " |" << endl
		//	<< "   |" << setw(6) << built_up(3,0) << "," << setw(6) << built_up(3,1) << "," << setw(6) << built_up(3,2) << "," << setw(6) << built_up(3,3) << " |" << endl;

		//for ( int r = 0; r < 3; ++r ) {
		//	for ( int c = 0; c < 3; ++c) {
		//		m[r][c] = built_up(r,c);
		//	}
		//}
		//q = MatrixToQuat( m );
		//out << "Quaternion:  [" << setw(6) << q.w << " (" << setw(6) << q.x << "," << setw(6) << q.y << "," << setw(6) << q.z << ")]" << endl;
		//out << "Euler Angles:" << endl
		//	<< "   X:  " << atan2( m[1][2], m[2][2] ) / pi * 180.0 << endl
		//	<< "   Y:  " << asin( -m[0][2] ) / pi * 180.0 << endl
		//	<< "   Z:  " << atan2( m[0][1], m[0][0] ) / pi * 180.0 << endl;

		//built_up = built_up.inverse();

		//out << "Inverse Built Up:" << endl
		//	<< "   |" << setw(6) << built_up(0,0) << "," << setw(6) << built_up(0,1) << "," << setw(6) << built_up(0,2) << "," << setw(6) << built_up(0,3) << " |" << endl
		//	<< "   |" << setw(6) << built_up(1,0) << "," << setw(6) << built_up(1,1) << "," << setw(6) << built_up(1,2) << "," << setw(6) << built_up(1,3) << " |" << endl
		//	<< "   |" << setw(6) << built_up(2,0) << "," << setw(6) << built_up(2,1) << "," << setw(6) << built_up(2,2) << "," << setw(6) << built_up(2,3) << " |" << endl
		//	<< "   |" << setw(6) << built_up(3,0) << "," << setw(6) << built_up(3,1) << "," << setw(6) << built_up(3,2) << "," << setw(6) << built_up(3,3) << " |" << endl;

		//for ( int r = 0; r < 3; ++r ) {
		//	for ( int c = 0; c < 3; ++c) {
		//		m[r][c] = built_up(r,c);
		//	}
		//}
		//q = MatrixToQuat( m );
		//out << "Quaternion:  [" << setw(6) << q.w << " (" << setw(6) << q.x << "," << setw(6) << q.y << "," << setw(6) << q.z << ")]" << endl;

		//out << "Euler Angles:" << endl
		//	<< "   X:  " << atan2( m[1][2], m[2][2] ) / pi * 180.0 << endl
		//	<< "   Y:  " << asin( -m[0][2] ) / pi * 180.0 << endl
		//	<< "   Z:  " << atan2( m[0][1], m[0][0] ) / pi * 180.0 << endl;

		out << "Flag Analysis:" << endl
			<< "   Hidden:  ";

		int flags = GetAttr("Flags")->asInt();

		if (flags & 1)
			out << "Yes" << endl;
		else
			out << "No" << endl;

		out << "   Collision Detection Mode:  ";

		
		if (flags & 2)
			out << "Use Triangles" << endl;
		else if (flags & 4)
			out << "Use Oriented Bounding Boxes" << endl;
		else
			out << "None" << endl;

		out << "   Is Skin Influence:  ";

		if (flags & 8)
			out << "No" << endl;
		else
			out << "Yes" << endl;
	}
};

/**
 * RootCollisionNode
 */
class RootCollisionNode : public AParentNode {

	public:

		RootCollisionNode(){}
		~RootCollisionNode(){}

		string GetBlockType() { return "RootCollisionNode"; }
};

/**
 * AvoidNode
 */
class AvoidNode : public AParentNode {

	public:

		AvoidNode(){}
		~AvoidNode(){}

		string GetBlockType() { return "AvoidNode"; }
};

/**
 * NiBillboardNode
 */
class NiBillboardNode : public AParentNode {

	public:

		NiBillboardNode(){}
		~NiBillboardNode(){}

		string GetBlockType() { return "NiBillboardNode"; }
};

/**
 * NiBSParticleNode
 */
class NiBSParticleNode : public AParentNode {

	public:

		NiBSParticleNode(){}
		~NiBSParticleNode(){}

		string GetBlockType() { return "NiBSParticleNode"; }
};

/**
 * NiBSAnimationNode
 */
class NiBSAnimationNode : public AParentNode {

	public:

		NiBSAnimationNode(){}
		~NiBSAnimationNode(){}

		string GetBlockType() { return "NiBSAnimationNode"; }
};


/**
 * ZBuffer data.
 */
class NiZBufferProperty : public AProperty {

	public:

		NiZBufferProperty(){}
		~NiZBufferProperty(){}

		string GetBlockType() { return "NiZBufferProperty"; }
};

/**
 * NiShadeProperty
 */
class NiShadeProperty : public AProperty {

	public:

		NiShadeProperty(){}
		~NiShadeProperty(){}

		string GetBlockType() { return "NiShadeProperty"; }
};

/**
 * ZBuffer data.NiWireframeProperty
 */
class NiWireframeProperty : public AProperty {

	public:

		NiWireframeProperty(){}
		~NiWireframeProperty(){}

		string GetBlockType() { return "NiWireframeProperty"; }
};

/**
 * NiDitherProperty
 */
class NiDitherProperty : public AProperty {

	public:

		NiDitherProperty(){}
		~NiDitherProperty(){}

		string GetBlockType() { return "NiDitherProperty"; }
};

/**
 * NiSequenceStreamHelper 
 */
class NiSequenceStreamHelper  : public ANamed {

	public:

		NiSequenceStreamHelper (){}
		~NiSequenceStreamHelper (){}

		string GetBlockType() { return "NiSequenceStreamHelper"; }
};

/**
 * NiVertexColorProperty - Vertex colour data.
 */
class NiVertexColorProperty : public AProperty{
	public:

		NiVertexColorProperty(){
			AddAttr( "vertmode", "Vertex Mode" );
			AddAttr( "lightmode", "Lighting Mode" );
		}
		~NiVertexColorProperty(){}

		string GetBlockType() { return "NiVertexColorProperty"; }
};



/**
 * NiTriShape - 
 */
class NiTriShape : public ANode {
	public:
		NiTriShape() {
			AddAttr( "index", "Data" );
			AddAttr( "index", "Skin Instance" );
		}
		~NiTriShape(){}
		string GetBlockType() { return "NiTriShape"; }
};

/**
 * NiTexturingProperty -
 */
class NiTexturingProperty : public AProperty {

	public:
		NiTexturingProperty( ) {
			AddAttr( "applymode", "Apply Mode" );
			AddAttr( "int", "Texture Count?" );
			AddAttr( "texture", "Base Texture" );
			AddAttr( "texture", "Dark Texture" );
			AddAttr( "texture", "Detail Texture" );
			AddAttr( "texture", "Gloss Texture" );
			AddAttr( "texture", "Glow Texture" );
			AddAttr( "bumpmap", "Bump Map Texture" );
			AddAttr( "texture", "Decal 0 Texture" );
		}
		~NiTexturingProperty(){}
		string GetBlockType() { return "NiTexturingProperty"; }
};

    //byte useExternal      - set to 0 or 1

    //if(useExternal == 0)
    //    byte unknown      = 1
    //    int pixelIndex    - index of NiPixelData record
    //else
    //    string file       - texture file name

    //int pixelLayout
    //int mipmap
    //int alpha             - see the NIFLA spec

    //byte unknown          = 1

/**
 * NiSourceTexture - Data for the associated texture, included in nif or external.
 */
class NiSourceTexture : public ANamed{

	public:
		NiSourceTexture(){
			AddAttr( "texsource", "Texture Source" );
			AddAttr( "pixellayout", "Pixel Layout" );
			AddAttr( "mipmapformat", "Use Mipmaps" );
			AddAttr( "alphaformat", "Alpha Format" );
			AddAttr( "byte", "Unknown Byte" );
		}
		~NiSourceTexture(){}
		string GetBlockType() { return "NiSourceTexture"; }
};



/**
 * NiPixelData - Texture data for an included texture.
 */
class NiPixelData : public ABlock{

	public:
		NiPixelData(){ data = NULL; }
		~NiPixelData(){ if (data != NULL) delete [] data; }

		void Read( ifstream& in );
		void Write( ofstream& out ){}
		string asString();
		string GetBlockType() { return "NiPixelData"; }

 	private:
		struct MipMap {
			uint width, height, offset;
		};
		
		uint unknownInt, rMask, gMask, bMask, aMask, bpp;
		byte unknown8Bytes[8];
		uint unknownIndex, mipCount, bytesPerPixel;
		vector<MipMap> mipmaps;
		uint dataSize;
		byte * data;

};



/**
 * NiMaterialProperty - material properties
 */
class NiMaterialProperty : public AProperty{
	public:
		NiMaterialProperty(){
			AddAttr( "float3", "Ambient Color" );
			AddAttr( "float3", "Diffuse Color" );
			AddAttr( "float3", "Specular Color" );
			AddAttr( "float3", "Emissive Color" );
			AddAttr( "float", "Glossiness" );
			AddAttr( "float", "Alpha" );
		}
		~NiMaterialProperty(){}
		string GetBlockType() { return "NiMaterialProperty"; };
};

/**
 * NiSpecularProperty -
 */
class NiSpecularProperty : public AProperty {

	public:
		NiSpecularProperty() {}
		~NiSpecularProperty(){}
		string GetBlockType() { return "NiSpecularProperty"; };
};



/**
 * NiAlphaProperty - Does the mesh have alpha-blending enabled?
 */
class NiAlphaProperty : public AProperty {

	public:

		NiAlphaProperty(){ 
			AddAttr( "byte", "Unknown Byte" );
		}
		~NiAlphaProperty(){}
		string GetBlockType() { return "NiAlphaProperty"; }
};



/**
 * Contains the mesh data of a group.
 */
class NiTriShapeData : public ABlock, public ITriShapeData {

	public:

		NiTriShapeData() : match_group_mode(false) {
			AddAttr( "float3", "Center" );
			AddAttr( "float", "Radius" );
		}
		~NiTriShapeData(){}

		void Read( ifstream& in );
		void Write( ofstream& out );
		string asString();
		string GetBlockType() { return "NiTriShapeData"; }
		void * QueryInterface( int id );

		//--ITriShapeData--//
		//Counts
		short GetVertexCount() { return short(vertices.size()); }
		short GetUVSetCount() { return short(uv_sets.size()); }
		short GetTriangleCount() { return short(triangles.size()); }
		void SetVertexCount(int n);
		void SetUVSetCount(int n);
		void SetTriangleCount(int n);
		//Match Detection
		void SetMatchDetectionMode(bool choice) { match_group_mode = choice; }
		bool GetMatchDetectionMode() { return match_group_mode; }
		//Getters
		vector<Vector3D> GetVertices() { return vertices; }
		vector<Vector3D> GetNormals() { return normals; }
		vector<Color> GetColors() { return colors; }
		vector<UVCoord> GetUVSet( int index ) { return uv_sets[index]; }
		vector<Triangle> GetTriangles() { return triangles; }
		//Setters
		void SetVertices( const vector<Vector3D> & in );
		void SetNormals( const vector<Vector3D> & in );
		void SetColors( const vector<Color> & in );
		void SetUVSet( int index, const vector<UVCoord> & in );
		void SetTriangles( const vector<Triangle> & in );

	private:
		vector<Vector3D> vertices;
		vector<Vector3D> normals;
		vector<Color> colors;
		vector< vector<UVCoord> > uv_sets;
		vector<Triangle> triangles;
		bool match_group_mode;
		//short vert_count;
};

/**
 * NiKeyframeController
 */
class NiKeyframeController : public AController {
public:
	NiKeyframeController(){
		AddAttr( "index", "Data" );
	}
	~NiKeyframeController(){}
	string GetBlockType() { return "NiKeyframeController"; }
};

/**
 * NiAlphaController
 */
class NiAlphaController : public AController {
public:
	NiAlphaController(){
		AddAttr( "index", "Data" );
	}
	~NiAlphaController(){}
	string GetBlockType() { return "NiAlphaController"; }
};

/**
 * NiVisController
 */
class NiVisController : public AController {
public:
	NiVisController(){
		AddAttr( "index", "Data" );
	}
	~NiVisController(){}
	string GetBlockType() { return "NiVisController"; }
};

/**
 * NiMaterialColorController
 */
class NiMaterialColorController : public AController {
public:

	NiMaterialColorController(){
		AddAttr( "index", "Data" );
	}
	~NiMaterialColorController(){}
	string GetBlockType() { return "NiMaterialColorController"; }
};

/**
 * NiUVController 
 */
class NiUVController : public AController {
public:
	NiUVController (){
		AddAttr( "index", "Data" );
		AddAttr( "short", "Unknown Short" );
	}
	~NiUVController (){}
	string GetBlockType() { return "NiUVController"; }
};

/**
 * NiPathController
 */

class NiPathController : public AController {
public:
	NiPathController  (){
		AddAttr( "int", "Unknown Int 1" );
		AddAttr( "int", "Unknown Int 2" );
		AddAttr( "int", "Unknown Int 3" );
		AddAttr( "short", "Unknown Short" );
		AddAttr( "index", "Pos Data" );
		AddAttr( "index", "Float Data" );
	}
	~NiPathController  (){}
	string GetBlockType() { return "NiPathController"; }
};

/**
 * NiAmbientLight - Not found in any official NIF files
 */

class NiAmbientLight : public ALight {
public:
	NiAmbientLight  (){}
	~NiAmbientLight  (){}
	string GetBlockType() { return "NiAmbientLight"; }
};

/**
 * NiDirectionalLight - Not found in any official NIF files
 */

class NiDirectionalLight : public ALight {
public:
	NiDirectionalLight  (){}
	~NiDirectionalLight  (){}
	string GetBlockType() { return "NiDirectionalLight"; }
};

/**
 * NiAutoNormalParticles
 */

class NiAutoNormalParticles : public ANode {
public:
	NiAutoNormalParticles  (){
		AddAttr( "index", "Data" );
		AddAttr( "index", "Unknown Index?" ); // Always -1
	}
	~NiAutoNormalParticles  (){}
	string GetBlockType() { return "NiAutoNormalParticles"; }
};

/**
 * NiRotatingParticles
 */

class NiRotatingParticles : public ANode {
public:
	NiRotatingParticles  (){
		AddAttr( "index", "Data" );
		AddAttr( "index", "Unknown Index?" ); // Always -1
	}
	~NiRotatingParticles  (){}
	string GetBlockType() { return "NiRotatingParticles"; }
}; 

/**
 * NiTextureEffect
 */

class NiTextureEffect : public ANode {
public:
	NiTextureEffect  (){
		AddAttr( "cint", "Conditional Int" );
		AddAttr( "float", "Unknown Float 1" );
		AddAttr( "float", "Unknown Float 2" );
		AddAttr( "float", "Unknown Float 3" );
		AddAttr( "float", "Unknown Float 4" );
		AddAttr( "float", "Unknown Float 5" );
		AddAttr( "float", "Unknown Float 6" );
		AddAttr( "float", "Unknown Float 7" );
		AddAttr( "float", "Unknown Float 8" );
		AddAttr( "float", "Unknown Float 9" );
		AddAttr( "float", "Unknown Float 10" );
		AddAttr( "float", "Unknown Float 11" );
		AddAttr( "float", "Unknown Float 12" );
		AddAttr( "int", "Unknown Int 1" );
		AddAttr( "int", "Unknown Int 2" );
		AddAttr( "int", "Unknown Int 3" );
		AddAttr( "int", "Unknown Int 4" );
		AddAttr( "index", "Source Texture" );
		AddAttr( "byte", "Unknown Byte" );
		AddAttr( "float", "Unknown Float 13" );
		AddAttr( "float", "Unknown Float 14" );
		AddAttr( "float", "Unknown Float 15" );
		AddAttr( "float", "Unknown Float 16" );
		AddAttr( "short", "PS2 L?" );
		AddAttr( "short", "PS2 K?" );
		AddAttr( "short", "Unknown Short" );
	}
	~NiTextureEffect  (){}
	string GetBlockType() { return "NiTextureEffect"; }
}; 

/**
 * NiCamera
 */

class NiCamera : public ANode {
public:
	NiCamera  (){
		AddAttr( "float", "Frustum Left" );
		AddAttr( "float", "Frustum Right" );
		AddAttr( "float", "Frustum Top" );
		AddAttr( "float", "Frustum Bottom" );
		AddAttr( "float", "Frustum Near" );
		AddAttr( "float", "Frustum Far" );
		AddAttr( "float", "Viewport Left" );
		AddAttr( "float", "Viewport Right" );
		AddAttr( "float", "Viewport Top" );
		AddAttr( "float", "Viewport Bottom" );
		AddAttr( "float", "LOLAdjust" );
		AddAttr( "index", "Unknown Index?" ); // Always -1
		AddAttr( "int", "Unknown Int" ); // Always 0
	}
	~NiCamera  (){}
	string GetBlockType() { return "NiCamera"; }
}; 

/**
 * NiGravity
 */

class NiGravity : public ABlock {
public:
	NiGravity  (){
		AddAttr( "index", "Extra Data" );
		AddAttr( "index", "Controller" );
		AddAttr( "float", "Unknown Float 1" );
		AddAttr( "float", "Unknown Float 2" );
		AddAttr( "int", "Unknown Int 1" );
		AddAttr( "float", "Unknown Float 3" );
		AddAttr( "float", "Unknown Float 4" );
		AddAttr( "float", "Unknown Float 5" );
		AddAttr( "float", "Unknown Float 6" );
		AddAttr( "float", "Unknown Float 7" );
		AddAttr( "float", "Unknown Float 8" );
	}
	~NiGravity  (){}
	string GetBlockType() { return "NiGravity"; }
}; 

/**
 * NiPlanarCollider
 */

class NiPlanarCollider : public ABlock {
public:
	NiPlanarCollider  (){
		AddAttr( "index", "Extra Data" );
		AddAttr( "index", "Controller" );
		AddAttr( "float", "Unknown Float 1" );
		AddAttr( "float", "Unknown Float 2" );
		AddAttr( "float", "Unknown Float 3" );
		AddAttr( "float", "Unknown Float 4" );
		AddAttr( "float", "Unknown Float 5" );
		AddAttr( "float", "Unknown Float 6" );
		AddAttr( "float", "Unknown Float 7" );
		AddAttr( "float", "Unknown Float 8" );
		AddAttr( "float", "Unknown Float 9" );
		AddAttr( "float", "Unknown Float 10" );
		AddAttr( "float", "Unknown Float 11" );
		AddAttr( "float", "Unknown Float 12" );
		AddAttr( "float", "Unknown Float 13" );
		AddAttr( "float", "Unknown Float 14" );
		AddAttr( "float", "Unknown Float 15" );
		AddAttr( "float", "Unknown Float 16" );
	}
	~NiPlanarCollider  (){}
	string GetBlockType() { return "NiPlanarCollider"; }
}; 

/**
 * NiParticleGrowFade
 */

class NiParticleGrowFade : public ABlock {
public:
	NiParticleGrowFade  (){
		AddAttr( "index", "Extra Data" );
		AddAttr( "index", "Controller" );
		AddAttr( "float", "Unknown Float 1" );
		AddAttr( "float", "Unknown Float 2" );
	}
	~NiParticleGrowFade  (){}
	string GetBlockType() { return "NiParticleGrowFade"; }
}; 

/**
 * NiParticleGrowFade
 */

class NiParticleColorModifier : public ABlock {
public:
	NiParticleColorModifier  (){
		AddAttr( "index", "Extra Data" );
		AddAttr( "index", "Controller" );
		AddAttr( "index", "Color Data" );
	}
	~NiParticleColorModifier  (){}
	string GetBlockType() { return "NiParticleColorModifier"; }
}; 

/**
 * NiGravity
 */

class NiParticleRotation : public ABlock {
public:
	NiParticleRotation  (){
		AddAttr( "index", "Extra Data" );
		AddAttr( "index", "Controller" );
		AddAttr( "byte", "Unknown Byte" );
		AddAttr( "float", "Unknown Float 1" );
		AddAttr( "float", "Unknown Float 2" );
		AddAttr( "float", "Unknown Float 3" );
		AddAttr( "float", "Unknown Float 4" );
	}
	~NiParticleRotation  (){}
	string GetBlockType() { return "NiParticleRotation"; }
}; 

//NiGravity : Controlled
//
//    float[2] unknown
//    int unknown
//    float[6] unknown
//
//NiPlanarCollider : Controlled
//
//    float unknown[16]
//
//NiParticleGrowFade : Controlled
//
//    float unknown[2]
//
//NiParticleColorModifier : Controlled
//
//    int colorIndex    - index to NiColorData record
//
//NiParticleRotation : Controlled
//
//    byte unknown
//    float unknown     = 1
//    float unknown[3]



template <class T>
struct Key {
	float time;
	T data, forward_tangent, backward_tangent;
	fVector3 tbc;
};

/**
 * NiKeyframeData -
 */
class NiKeyframeData : public ABlock{

	public:

		NiKeyframeData(){
			rotKeys = NULL;
			transKeys = NULL;
			scaleKeys = NULL;
		}
		~NiKeyframeData(){
			if (rotKeys != NULL)
				delete[] rotKeys;
			if (transKeys != NULL)
				delete[] transKeys;
			if (scaleKeys != NULL)
				delete[] scaleKeys;
		}

		void Read( ifstream& in );
		void Write( ofstream& out ){}
		string asString();
		string GetBlockType() { return "NiKeyframeData"; }

	private:

		uint				numRotations;
		uint				rotationType;
		Key<fVector4> *		rotKeys;

		uint				numTranslations;
		uint				translationType;
		Key<fVector3> *		transKeys;

		uint				numScalings;
		uint				scaleType;
		Key<float> *		scaleKeys;

};


//-- Skin Stuff --//

    //int data              - NiSkinData index
    //int root              - Skeleton root NiNode index (?)
    //int boneCount
    //int bones[boneCount]  - Index of bones (NiNode records)

/**
 * NiSkinInstance
 */

//Non-Public interface to allow NiSkinData to get the bone list read by NiSkinInstance
class ISkinInstInternal {
public:
	virtual vector<int> GetBoneList() = 0;
	virtual void ReadBoneList( ifstream& in ) = 0;
};

class NiSkinInstance : public ABlock, public ISkinInstInternal {

	public:

		NiSkinInstance(){
			AddAttr( "index", "Data" );
			AddAttr( "int", "Skeleton Root" );
			AddAttr( "bones", "Bones" );
		}
		~NiSkinInstance(){}
		string GetBlockType() { return "NiSkinInstance"; }

		void * QueryInterface( int id ) {
			if ( id == SkinInstInternal ) {
				return (void*)static_cast<ISkinInstInternal*>(this);;
			} else {
				return ABlock::QueryInterface( id );
			}
		}

		//ISkinInstInternal

		vector<int> GetBoneList() { return bones; }

		void ReadBoneList( ifstream& in ) {
			int len = ReadUInt( in );
			bones.resize( len );
			for (int i = 0; i < len; ++i ) {
				bones[i] = ReadUInt( in );
			}
		}
private:
	vector<int> bones;
};

class ISkinDataInternal {
public:
	virtual void SetBones( vector<blk_ref> bone_blocks ) = 0;
	virtual void RepositionTriShape() = 0;
	virtual void StraightenSkeleton() = 0;
};

class NiSkinData : public ABlock, public ISkinData, public ISkinDataInternal {

	public:

		NiSkinData(){}
		~NiSkinData(){}

		void Read( ifstream& in );
		void Write( ofstream& out );
		string asString();
		string GetBlockType() { return "NiSkinData"; }
		void * QueryInterface( int id );

		//ISkinDataInternal
		void SetBones( vector<blk_ref> bone_blocks );
		void RepositionTriShape();
		void StraightenSkeleton();

        //ISkinData
		vector<blk_ref> GetBones();
		map<int, float> GetWeights( blk_ref bone );
		void AddBone( blk_ref bone, map<int, float> in );
		void RemoveBone( blk_ref bone ) { bone_map.erase( bone ); }
	private:
		struct Bone {
			matrix rotation;
			fVector3 translation;
			float scale;
			fVector4 unknown4Floats;
			map<int, float> weights;
		};

		matrix rotation;
		fVector3 translation;
		float  scale;
		nifIndex unknown;
		map<blk_ref, Bone> bone_map;
		vector<Bone> bones;		
};

//-- New Nodes--//

class NiGeomMorpherController : public AController{

	public:

		NiGeomMorpherController(){
			AddAttr( "index", "Morph Data" );
			AddAttr( "byte", "Unknown Byte" );
		}
		~NiGeomMorpherController(){}

		string asString();
		string GetBlockType() { return "NiGeomMorpherController"; }
};

class NiColorData : public ABlock{

	public:

		NiColorData(){}
		~NiColorData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiColorData"; };

	private:
		uint colorCount, keyType;
		vector<Key<fVector4> > keys;
};

class NiFloatData : public ABlock{

	public:

		NiFloatData(){}
		~NiFloatData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiFloatData"; };

	private:
		uint colorCount, keyType;
		vector<Key<float> > keys;
};

class NiStringExtraData : public ABlock{

	public:

		NiStringExtraData(){}
		~NiStringExtraData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiStringExtraData"; };

	private:
		nifIndex next_index;
		uint bytesRemaining;
		string strData;

};

class NiMorphData : public ABlock{

	public:

		NiMorphData(){}
		~NiMorphData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiMorphData"; };

	private:
		struct Morph {
			Morph() { morph = NULL; }
			~Morph() { if (morph != NULL) delete [] morph; }
			uint timeCount;
			uint keyType;
			vector<Key<float> > keys;
			fVector3 * morph;
		};
		
		uint morphCount, vertCount;
		byte unknownByte;
		vector<Morph> morphs;
};

class NiPosData : public ABlock{

	public:

		NiPosData(){}
		~NiPosData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiPosData"; }

	private:
		uint posCount, keyType;
		vector<Key<fVector3> > keys;
};

class NiRotatingParticlesData : public ABlock{

	public:

		NiRotatingParticlesData(){}
		~NiRotatingParticlesData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString() { return string(""); };
		string GetBlockType() { return "NiRotationparticlesData"; }

	private:

};

class NiTextKeyExtraData : public ABlock{

	public:

		NiTextKeyExtraData(){}
		~NiTextKeyExtraData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiTextKeyExtraData"; }

	private:
		nifIndex next_index;
		uint unknownInt, keyCount;
		vector<Key<string> > keys;
};

class NiUVData : public ABlock{

	public:

		NiUVData(){}
		~NiUVData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiUVData"; }

	private:
		struct UVGroup {
			uint count;
			uint keyType;
			vector<Key<float> > keys;
		};
		UVGroup groups[4];
};

class NiVertWeightsExtraData : public ABlock{

	public:

		NiVertWeightsExtraData(){}
		~NiVertWeightsExtraData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiVertWeightsExtraData"; }

	private:
		nifIndex next_index;
		uint bytes;
		ushort verts;
		vector<float> weights;
};

class NiVisData : public ABlock{

	public:

		NiVisData(){}
		~NiVisData(){}

		void Read( ifstream& in );
		void Write( ofstream& out ) {}
		string asString();
		string GetBlockType() { return "NiVisData"; }

	private:
		uint visCount;
		vector<Key<byte> > keys;
};

class UnknownMixIn {
public:
	UnknownMixIn( string block_type ){
		data = NULL;
		_block_type = block_type;
	}
	~UnknownMixIn(){ if (data != NULL) delete [] data; }
	void Read( ifstream& in );
	void Write( ofstream& out );
	string asString();
	string GetBlockType() { return _block_type; }

private:
	string _block_type;
	int len;
	byte * data;
};

class UnknownBlock : public ABlock, public UnknownMixIn {
public:
	UnknownBlock( string block_type ) : UnknownMixIn(block_type) {}
	~UnknownBlock(){}
	void Read( ifstream& in ) {
		//cout << endl << "Unknown Block Type found:  " << GetBlockType() << "\a" << endl;
		ABlock::Read( in );
		UnknownMixIn::Read( in );
	}
	void Write( ofstream& out ) {
		ABlock::Write( out );
		UnknownMixIn::Write( out );
	}
	void Print( ostream & out ) {
		out << ABlock::asString();
		out << UnknownMixIn::asString;
	}
	string GetBlockType() { return UnknownMixIn::GetBlockType(); }
};

class UnknownControllerBlock : public AController, public UnknownMixIn {
public:
	UnknownControllerBlock( string block_type ) : UnknownMixIn(block_type) {}
	~UnknownControllerBlock(){}
	void Read( ifstream& in ) {
		ABlock::Read( in );
		UnknownMixIn::Read( in );
	}
	void Write( ofstream& out ) {
		ABlock::Write( out );
		UnknownMixIn::Write( out );
	}
	void Print( ostream & out ) {
		out << ABlock::asString();
		out << UnknownMixIn::asString();
	}
	string GetBlockType() { return UnknownMixIn::GetBlockType(); }
};

class UnknownPropertyBlock : public AProperty, public UnknownMixIn {
public:
	UnknownPropertyBlock( string block_type ) : UnknownMixIn(block_type) {}
	~UnknownPropertyBlock(){}
	void Read( ifstream& in ) {
		ABlock::Read( in );
		UnknownMixIn::Read( in );
	}
	void Write( ofstream& out ) {
		ABlock::Write( out );
		UnknownMixIn::Write( out );
	}
	void Print( ostream & out ) {
		out << ABlock::asString();
		out << UnknownMixIn::asString();
	}
	string GetBlockType() { return UnknownMixIn::GetBlockType(); }
};

///**
// * NiParticleSystemController
// */
//class NiParticleSystemController : public UnknownControllerBlock {
//public:
//	NiParticleSystemController() : UnknownControllerBlock("NiParticleSystemController") {	
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "byte", "Byte?" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "float", "Guess" );
//		AddAttr( "short", "Count?" );
//		
//	
//	}
//	
//	//void Read( ifstream& in ) {
//
//	//	ABlock::Read( in );
//
//
//	//	
//
//
//	//	uint count = GetAttr("Count")->asInt();
//
//	//	cout << "Count:  " << count << endl;
//
//	//	short num = 0;
//	//	short last_num = -1;
//	//	int n = 0;
//	//	//IAttr * attr = new MatrixAttr( "" );
//	//	//cout << setprecision(3);
//	//	while (true) {
//	//		//attr->Read( in );
//	//		//attr->Print( cout );
//	//		ReadFloat( in );	ReadFloat( in );	ReadFloat( in );
//	//		ReadFloat( in );	ReadFloat( in );	ReadFloat( in );
//	//		ReadFloat( in );	ReadFloat( in );	ReadFloat( in );
//	//		ReadUShort( in );
//	//		num = ReadUShort( in );
//	//		//cout << "  " << num << endl;
//
//	//		if ( num != last_num + 1 )
//	//			break;
//
//	//		last_num = num;
//	//		++n;
//
//	//	}
//
//	//	in.seekg( -40, ios::cur );
//
//	//	cout << "True Count:  " << n << endl;
//
//	//	if ( n != count ) {
//	//		cout << "\a";
//	//		cin.get();
//	//	}
//
//	//	unk.Read( in );
//
//	//	
//	//	//if (!b) {
//	//	//	cout << ReadFloat( in ) << endl
//	//	//		<< ReadFloat( in ) << endl
//	//	//		<< ReadFloat( in ) << endl
//	//	//		<< ReadFloat( in ) << endl;
//	//	//}
//	//	
//	//	
//	//}
//
//	~NiParticleSystemController(){}
//	string GetBlockType() { return "NiParticleSystemController"; }
//};


#endif // TAH_NIF_LIB_NIF_BLOCKS_H