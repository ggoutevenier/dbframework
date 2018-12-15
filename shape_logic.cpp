//#include "store.h"
#include "shape.h"

namespace dk {
	namespace logic {
		// create implementations of the Function<ShapeType> interface
		// must create an instance of area function
		// needs to have a static function getId that defines the key_type 
		// for the implementation
		class Triangle : public Function<layout::ShapeType> {
		protected:
			// private helper functions
			double getBase(const layout::Shape &shape) const {
				return shape.atttribute.at(0);
			}
			double getHeight(const layout::Shape &shape) const {
				return shape.atttribute.at(1);
			}
		public:
			// overriden interface area function
			double area(const layout::Shape &shape) const override{
				return getBase(shape)*getHeight(shape)/2;
			}
			// pull key_type def from layout
			static auto getId() { return layout::ShapeType::key_type(1); }
		};

		class Rectangle : public Function<layout::ShapeType> {
		protected:
			virtual double getLength(const layout::Shape &shape) const {
				return shape.atttribute.at(0);
			}
			virtual double getWidth(const layout::Shape &shape) const {
				return shape.atttribute.at(1);
			}
		public:
			double area(const layout::Shape &shape) const {
				return getLength(shape)*getWidth(shape);
			}
			using key_type = dk::layout::ShapeType::key_type;
			static auto getId() { return layout::ShapeType::key_type(2); }
		};

		class Square : public Rectangle {
		protected:
			double getWidth(const layout::Shape &shape) const override {
				return shape.atttribute.at(0);
			}
		public:
			using key_type = Rectangle::key_type;
			static auto getId() { return layout::ShapeType::key_type(3); }
		};

		class Circle : public Function<layout::ShapeType> {
		protected:
			static const double pi;
			double getRadius(const layout::Shape &shape) const {
				return shape.atttribute.at(0);
			}
		public:

			double area(const layout::Shape &shape) const {
				return getRadius(shape)*pi;
			}
			using key_type = dk::layout::ShapeType::key_type;
			static auto getId() { return layout::ShapeType::key_type(4); }
		};

		const double Circle::pi = 3.14159;
		
		// create the construction of the singleton Functions<ShapeType>
		// This Functions class will be used to resolve the logic function 
		// after the data has been read in from the database
		template<>
		Functions<layout::ShapeType>::Functions() {
			add<Circle >();  // 4
			add<Square >();  // 3
			add<Rectangle>(); // 2
			add<Triangle >(); // 1
		}
	}
}
