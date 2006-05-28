/* Copyright (c) 2006, NIF File Format Library and Tools
All rights reserved.  Please see niflib.h for licence. */

#include "bhkRefObject.h"

//Definition of TYPE constant
const Type bhkRefObject::TYPE("bhkRefObject", &BHK_REF_OBJECT_PARENT::TYPE );

bhkRefObject::bhkRefObject() BHK_REF_OBJECT_CONSTRUCT {}

bhkRefObject::~bhkRefObject() {}

void bhkRefObject::Read( istream& in, list<uint> link_stack, unsigned int version ) {
	BHK_REF_OBJECT_READ
}

void bhkRefObject::Write( ostream& out, map<NiObjectRef,uint> link_map, unsigned int version ) const {
	BHK_REF_OBJECT_WRITE
}

string bhkRefObject::asString( bool verbose ) const {
	BHK_REF_OBJECT_STRING
}

void bhkRefObject::FixLinks( const vector<NiObjectRef> & objects, list<uint> link_stack, unsigned int version ) {
	BHK_REF_OBJECT_FIXLINKS
}
