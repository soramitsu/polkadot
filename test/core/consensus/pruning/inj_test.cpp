#include <gtest/gtest.h>
#include <boost/di.hpp>
#include <boost/di/extension/policies/types_dumper.hpp>

#include <iostream>

class A {
 public:
  virtual ~A() = default;
  virtual double val() = 0;
};

class B : public A {
  static double sval_;
  double val_;

  B() : val_{sval_++} {}

 public:
  static std::shared_ptr<B> create() {
    return std::shared_ptr<B>(new B);
  }
  ~B() override = default;
  double val() override {
    return val_;
  }
};

double B::sval_ = 0.0;


TEST(InjTest, SimpleInj) {
  namespace di = boost::di;

  auto d = B::create();

  auto injector = di::make_injector<di::extension::types_dumper>(
      di::bind<A>.to([d](const auto &) { return d; }));

  auto a = injector.template create<std::shared_ptr<A>>();
  auto b = injector.template create<std::shared_ptr<A>>();

  std::cout << a->val() << std::endl;
  std::cout << b->val() << std::endl;
}
