#include "db_sqlite3.h"
#include "db_mysql.h"
#include <sqlite3.h>
#include "store.h"
#include "shape.h" 

#include <iostream>

///////////////////////////////////////////////////////////////////////////////
// shape.h has 
//	- structure definition that will contain the data read from a database
//	- interface definitions for assigned the logic
//			
// shape_logic.cpp 
//	- contains the different implementations of how to calculate the area
//      of the shape_type
//	- mapping of logic to key_type via the Functions class constructor 
//
// shape_metadata.cpp 
//	- has the c structure to database table/column names
//
// db_sqlite3.cpp 
//	- the implementation of the database interface for sqlite
///////////////////////////////////////////////////////////////////////////////
using namespace dk;

int main(int argc, char *argv[]) {
	try {
		// define object store using a sqlite database
		Store store(
			sqlite::make_connection("/home/gerard/shape.db", SQLITE_OPEN_READONLY)
		);
		{
			std::shared_ptr<IConnection>  dst =
					mysql::make_connection(
							"gerard",
							"localhost",
							"gerard",
							"a46384");

//				= sqlite::make_connection(
//						"/home/gerard/shape2.db",
//						SQLITE_OPEN_CREATE|SQLITE_OPEN_READWRITE);

		Sink<layout::Shape> sink(dst);

			for (layout::Shape const *shape : store.projection<layout::Shape>()) {
				sink.push_back(*shape);
				std::cout
					<< shape->shapeName
					<< " : "
	//				<< shape->type->logic->area(*shape)
					<< shape->area()
					<< std::endl;
			}
//			store.save(dst);
		}
	}
	catch (std::exception &e) {
		std::string str = e.what();
		std::cout << e.what() << std::endl;

		std::cout << "Exiting with return 1" << std::endl;

		return 1;
	}
	return 0;
}

