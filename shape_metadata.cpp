#include "shape.h"
#include "store.h"
#include "metadata.h"

namespace dk {
/*
CREATE TABLE shape_type (
shape_type_id INTEGER PRIMARY KEY,
shape_type_desc TEXT UNIQUE NOT NULL
) WITHOUT ROWID;

CREATE TABLE shape (
shape_id INTEGER PRIMARY KEY,
shape_name TEXT UNIQUE NOT NULL,
shape_type_id INTEGER REFERENCES shape_type (shape_type_id) NOT NULL,
attribute_1 DOUBLE,
attribute_2 DOUBLE,
attribute_3 DOUBLE,
attribute_4 DOUBLE,
attribute_5 DOUBLE
) WITHOUT ROWID;

insert into shape_type values (1,'Triangle');
insert into shape_type values (2,'Rectangle');
insert into shape_type values (3,'Square');
insert into shape_type values (4,'Circle');

delete from shape;
insert into shape values (1,'Circle 1',4,1,null,null,null,null);
insert into shape values (2,'Circle 5',4,5,null,null,null,null);
insert into shape values (3,'Rectangle 1x2',2,1,2,null,null,null);
insert into shape values (4,'Rectangle 4x8',2,4,8,null,null,null);
insert into shape values (5,'Square 4x4',3,4,null,null,null,null);
insert into shape values (6,'Rectangle 4x4',3,4,4,null,null,null);
insert into shape values (7,'Triangle 1x2',1,1,2,null,null,null);

*/
	/*mapping of structure to a database table*/
	template<>
	metadata<layout::ShapeType> ::metadata() : 
		Record("shape_type") { // shape_type is the name for the database table
		// map fields of structure to column names in database
		add(typeId, "shape_type_id");
		add(typeDesc, "shape_type_desc");
		// add the logic pointer that will be resolved based on ShapeType::key_type
		add(logic);
	}

	template<>
	metadata<layout::Shape> ::metadata() : 
		Record("shape") { // shape is the name of the database table
		// map fields of structure to column names in database
		add(shapeId, "shape_id");
		add(shapeName, "shape_name");
		add(type.typeId, "shape_type_id");
		add(atttribute.at(0), "attribute_1");
		add(atttribute.at(1), "attribute_2");
		add(atttribute.at(2), "attribute_3");
		add(atttribute.at(3), "attribute_4");
		add(atttribute.at(4), "attribute_5");
		//resolve to type to the ShareType entry
		add(/*ref*/ type,"shape_type");
	}
}
