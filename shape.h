#pragma once
#include <list>
#include <array>
#include <set>
#include "ref.h"
#include "function.h"
#include "decimal.h"

namespace dk {
	// define keys for tables
	// these should match the primary key definitions for the tables
	namespace key {
		struct ShapeType {
			uint32_t typeId;
			ShapeType() :typeId(0) {}
			ShapeType(uint32_t a) :typeId(a) {}
			ShapeType(const ShapeType &a) :typeId(a.typeId) {}
			friend bool operator<(const ShapeType &a, const ShapeType &b) {
				return a.typeId < b.typeId;
			}
		};

		struct Shape {
			uint32_t shapeId;
			Shape() :shapeId(0) {}
			Shape(uint32_t a) :shapeId(a) {}
			Shape(const Shape &a) :shapeId(a.shapeId) {}
			friend bool operator<(const Shape &a, const Shape &b) {
				return a.shapeId < b.shapeId;
			}
		};
	}

	namespace layout {
		struct ShapeType;
		struct Shape;
	}

	namespace logic {
		// interface definition for logic that will be assigned to rows
		// read into the ShapeType structure
		template<>
		class Function<layout::ShapeType> {
		public:
			virtual ~Function<layout::ShapeType>() {}
			virtual double area(const layout::Shape &shape) const = 0;
		};
	}

	namespace layout {
		struct ShapeType : key::ShapeType {
			using key_type = key::ShapeType; // layout structures must tyepdef key_type
			std::string typeDesc;
			// pointer to implementation of interface defined above
			const logic::Function<layout::ShapeType> *logic;
		};

		struct Shape : key::Shape {
			using key_type = key::Shape;
			char shapeName[64];
			// the following fields resolves to the appropriate ShapeType 
			// row based on value in field type.type_id
			Ref<layout::ShapeType> type; 
			
			std::array<dec::decimal<7>, 5> atttribute;
			double area() const {
				return type->logic->area(*this);
			}
		};
	}
}
