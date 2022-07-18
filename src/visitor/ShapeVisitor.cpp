#include <iostream>
#include <memory>
#include <vector>

struct Hexagon;
struct Circle;
class Trapezium;

struct ShapeVisitor {
  virtual void visitHexagon(const std::shared_ptr<const Hexagon> shape) const = 0;
  virtual void visitCircle(const std::shared_ptr<const Circle> shape) const = 0;
  virtual void visitTrapezium(const std::shared_ptr<const Trapezium> shape) const = 0;
};

struct Shape {
  virtual constexpr double getPerimeter() const = 0;
  virtual void accept(const ShapeVisitor &shape_visitor) const = 0;
  virtual ~Shape() = default;
};

struct Hexagon : public Shape, public std::enable_shared_from_this<Hexagon> {
  constexpr Hexagon(double length) : length_(length) {
  };
  constexpr double getPerimeter() const override {
	return 6 * length_;
  }
  void accept(const ShapeVisitor &shape_visitor) const override {
	shape_visitor.visitHexagon(shared_from_this());
  }

  double length_{0};
};

struct Circle : public Shape, public std::enable_shared_from_this<Circle> {
  constexpr Circle(double radius) : radius_(radius) {
  };

  constexpr double getPerimeter() const override {
	return 2 * pi * radius_;
  }

  void accept(const ShapeVisitor &shape_visitor) const override {
	shape_visitor.visitCircle(shared_from_this());
  }

  double radius_{0};
  constexpr static double pi{3.141592654};
};

struct Trapezium : public Shape, public std::enable_shared_from_this<Trapezium> {

  constexpr Trapezium(double base, double top, double side1, double side2)
	  : base_(base), top_(top), side1_(side1), side2_(side2) {
  };

  constexpr double getPerimeter() const override {
	return base_ + top_ + side1_ + side2_;
  }

  void accept(const ShapeVisitor &shape_visitor) const override {
	shape_visitor.visitTrapezium(shared_from_this());
  }

  double base_{0}, top_{0}, side1_{0}, side2_{0};
};

struct PerimeterVisitor : ShapeVisitor {
  void visitHexagon(const std::shared_ptr<const Hexagon> shape) const override {
	std::cout << "Hexagon length " << shape->length_ << ", perimeter: " << shape->getPerimeter() << std::endl;
  }
  void visitCircle(const std::shared_ptr<const Circle> shape) const override {
	std::cout << "Circle radius " << shape->radius_ << " perimeter: " << shape->getPerimeter() << std::endl;
  }
  void visitTrapezium(const std::shared_ptr<const Trapezium> shape) const override {
	std::cout << "Trapezium base " << shape->base_
			  << ", top " << shape->top_
			  << ", side1 " << shape->side1_
			  << ", side2 " << shape->side2_
			  << ", perimeter: " << shape->getPerimeter()
			  << std::endl;
  }
};

int main() {
  auto circle = std::make_shared<Circle>(10);
  auto hexagon = std::make_shared<Hexagon>(11);
  auto trapezium = std::make_shared<Trapezium>(1, 2, 3, 4);
  std::vector<std::shared_ptr<const Shape>> v{circle, hexagon, trapezium};

  auto perimeter_client_code =
	  [](const std::vector<std::shared_ptr<const Shape>> &data, const PerimeterVisitor &visitor) {
		for (const auto &shape : data) {
		  shape->accept(visitor);
		}
	  };
  PerimeterVisitor visitor;

  perimeter_client_code(v, visitor);
}